/*
 * BloodHorn Bootloader
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */
#include <Uefi.h>
#include "compat.h"
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleTextInEx.h>
#include "../uefi/graphics.h"
#include "theme.h"
#include "localization.h"
#include "mouse.h"
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../config/config_ini.h"
#include "../config/config_json.h"
#include "../config/config_env.h"
#include "font.h"
#include "../security/crypto.h"

#define MAX_BOOT_ENTRIES 128
#define MAX_ENTRY_LENGTH 64
#define VISIBLE_MENU_ENTRIES 10

/*
 * Boot menu architecture overview (high-level yapping):
 *
 * - This file is the orchestration layer for the interactive boot menu.
 *   It does NOT know how to actually boot an OS; instead it owns a list of
 *   `BOOT_MENU_ENTRY` items, each of which points at a boot function that
 *   lives elsewhere.
 *
 * - Rendering is delegated to the `uefi/graphics.c` side via
 *   `InitializeGraphics`, `DrawRect`, `ClearScreen` (and any platform
 *   provided primitives like `GraphicsOutput`). The menu only decides
 *   *what* to draw and *where*; it does not care how pixels hit the screen.
 *
 * - The color palette and optional background image are provided by
 *   `boot/theme.c` through `GetBootMenuTheme()`. Configuration code or
 *   main.c call `LoadThemeAndLanguageFromConfig()` and `SetBootMenuTheme()`
 *   before we ever show the menu, so this file can assume the theme is
 *   ready to use.
 *
 * - Localized strings (title, footer instructions, etc.) are provided by
 *   `boot/localization.c` via `GetLocalizedString()`. This file additionally
 *   supports an optional flat text file based localization override via
 *   `LoadLocalizationFile()` below.
 *
 * - Input comes from two sources:
 *     * Keyboard via the standard UEFI `SimpleTextIn` protocol and scan
 *       codes (arrow keys, Enter, Esc, alphanumeric hotkeys).
 *     * Mouse via `boot/mouse.c`, which exposes a very small `MouseState`
 *       abstraction we can query every frame.
 *
 * - Global state in this file (the `BootEntries` array, `BootEntryCount`,
 *   `SelectedEntry`, `MenuScrollOffset` and `BootEntryHotkeys`) is the
 *   canonical in-memory representation of the current menu. The rest of the
 *   code only reads or mutates this state; it does not duplicate it.
 */

// Boot menu entry structure: a human readable name + a callback to execute. (hopefully)
typedef struct {
    CHAR16 Name[MAX_ENTRY_LENGTH];
    EFI_STATUS (*BootFunction)(VOID);
} BOOT_MENU_ENTRY;

// Global boot menu state (lives for the lifetime of the process).
static BOOT_MENU_ENTRY BootEntries[MAX_BOOT_ENTRIES];
static UINTN BootEntryCount = 0;      // How many entries are currently valid.
static INTN SelectedEntry = 0;        // Index of the currently highlighted entry.
static INTN MenuScrollOffset = 0;     // First visible entry when list is long.

// Hotkey mapping: one (lowercased ASCII) character per entry.
// Using CHAR16 here keeps it aligned with UEFI's CHAR16 / UnicodeChar.
static CHAR16 BootEntryHotkeys[MAX_BOOT_ENTRIES];

// Small helper to keep SelectedEntry within the visible window whenever
// it changes (via arrows, hotkeys, mouse hover, or programmatic jumps).
//
// Intuition / invariants:
// - 0 <= SelectedEntry < BootEntryCount  (when BootEntryCount > 0)
// - 0 <= MenuScrollOffset < BootEntryCount  (or 0 when the list is short)
// - At any point where drawing relies on (MenuScrollOffset, SelectedEntry),
//   the selected row index falls into the inclusive window
//       [MenuScrollOffset, MenuScrollOffset + VISIBLE_MENU_ENTRIES - 1]
//   unless the total number of entries is smaller than VISIBLE_MENU_ENTRIES,
//   in which case the window is effectively truncated by BootEntryCount.
//
// In other words, this function enforces a coupling between two pieces of
// state that would otherwise be easy to get out of sync if each caller tried
// to manually manipulate both.
static VOID
EnsureSelectedEntryVisible(VOID)
{
    // Degenerate case: the caller somehow asked for visibility when there
    // are no entries at all. We normalize both indices to 0. This ensures
    // that downstream code that assumes non-negative indices does not need
    // extra guards here.
    if (BootEntryCount == 0) {
        SelectedEntry = 0;
        MenuScrollOffset = 0;
        return;
    }

    // Clamp SelectedEntry to the mathematically valid closed interval
    // [0, BootEntryCount - 1]. If SelectedEntry is outside this interval
    // for any reason (e.g. arithmetic wrap, caller bug), we snap it back
    // to the nearest boundary point.
    if (SelectedEntry < 0) {
        SelectedEntry = 0;
    } else if ((UINTN)SelectedEntry >= BootEntryCount) {
        SelectedEntry = (INTN)(BootEntryCount - 1);
    }

    // Now we enforce the visibility condition described in the header
    // comment. There are two ways the selected item can violate the
    // current scroll window:
    //   1. It is located above MenuScrollOffset (off-screen at the top).
    //   2. It is located at or beyond MenuScrollOffset + VISIBLE_MENU_ENTRIES
    //      (off-screen at the bottom).
    if (SelectedEntry < MenuScrollOffset) {
        MenuScrollOffset = SelectedEntry;
    } else if (SelectedEntry >= MenuScrollOffset + VISIBLE_MENU_ENTRIES) {
        MenuScrollOffset = SelectedEntry - VISIBLE_MENU_ENTRIES + 1;
    }

    // Finally, guard against negative scroll offsets. In the arithmetic
    // above we only subtract up to (VISIBLE_MENU_ENTRIES - 1) and we know
    // SelectedEntry >= 0, but this last clamp documents the invariant that
    // callers should rely on: MenuScrollOffset is never negative.
    if (MenuScrollOffset < 0) {
        MenuScrollOffset = 0;
    }
}

// Helper: assign hotkeys (first unique ASCII letter per entry).
//
// Conceptually, we want a partial, injective mapping from entries to
// printable ASCII keys:
//     EntryIndex -> Key (char in [32, 127])
// such that:
//   - Each chosen key is unique across all entries (no two entries share
//     the same hotkey).
//   - The key is taken from the entry's display name, scanning from left
//     to right, so the user can often guess the shortcut.
//   - The mapping is stable under re-entry: given the same ordered list of
//     names, we produce the same hotkeys.
//
// We intentionally restrict ourselves to 7-bit ASCII so we don't have to
// reason about OEM layouts or high Unicode ranges in firmware, which rarely
// behave consistently. It also simplifies the equality relation we use
// when matching key strokes.
//
// Formally, the algorithm is:
//   - used[c] tracks whether an ASCII codepoint has already been claimed.
//   - For each entry i in [0, BootEntryCount):
//       * Iterate over the entry name as a CHAR16 string.
//       * Fold A–Z to a–z so case doesn't matter.
//       * If the character lives in [32, 127) and !used[c], claim it and
//         stop scanning this name.
//   - If no character qualifies, the entry gets no hotkey (BootEntryHotkeys[i] == 0).
static VOID
AssignHotkeys(VOID)
{
    bool used[256] = {0};

    // Clear any stale hotkeys from previous menu invocations so entries
    // that disappear do not keep phantom bindings that "do not stick".
    // Here we effectively enforce that hotkey state is a pure function
    // of the current BootEntries[0..BootEntryCount) contents.
    for (UINTN i = 0; i < MAX_BOOT_ENTRIES; ++i) {
        BootEntryHotkeys[i] = 0;
    }

    for (UINTN i = 0; i < BootEntryCount; ++i) {
        const CHAR16 *name = BootEntries[i].Name;  // invariant: NULL-terminated

        for (UINTN j = 0; name[j] != 0; ++j) {
            CHAR16 c = name[j];

            // Basic ASCII‑only lowercase conversion so that 'A' and 'a'
            // map to the same hotkey bucket.
            if (c >= L'A' && c <= L'Z') {
                c = (CHAR16)(c - L'A' + L'a');
            }

            // The restriction 32 <= c < 128 implicitly discards control
            // characters and high Unicode. We treat the CHAR16 code unit
            // as a single-byte index into `used`; this is safe as long as
            // we stay in the 7-bit range.
            if (c >= 32 && c < 128 && !used[c]) {
                BootEntryHotkeys[i] = c;
                used[c] = true;
                break;
            }
        }
    }
}

// Helper: load external localization file
static void LoadLocalizationFile(const char* lang_code) {
    char filename[64];
    snprintf(filename, sizeof(filename), "lang_%s.txt", lang_code);
    FILE* f = fopen(filename, "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char key[64]; wchar_t value[256];
        if (sscanf(line, "%63[^=]=%255ls", key, value) == 2) {
            AddLocalizedString(key, value);
        }
    }
    fclose(f);
}

/**
  Adds a new boot menu entry.
  
  @param[in] Name           The display name of the boot entry.
  @param[in] BootFunction   The function to call when this entry is selected.
  
  @retval EFI_SUCCESS       The entry was added successfully.
  @retval EFI_OUT_OF_RESOURCES  No more space is available for boot entries.
**/
EFI_STATUS
AddBootEntry(
    IN CONST CHAR16 *Name,
    IN EFI_STATUS (*BootFunction)(VOID)
) {
    if (BootEntryCount >= MAX_BOOT_ENTRIES) {
        return EFI_OUT_OF_RESOURCES;
    }

    // Copy the name and boot function.
    //
    // We deliberately cap the copy at MAX_ENTRY_LENGTH - 1 characters and
    // let StrnCpyS handle termination rules. The UI side of this file uses
    // MAX_ENTRY_LENGTH as the size for its on-stack buffers as well, so the
    // invariant we want is:
    //     length(BootEntries[i].Name) <= MAX_ENTRY_LENGTH - 1
    // which ensures we never overflow local buffers when formatting.
    StrnCpyS(
        BootEntries[BootEntryCount].Name,
        MAX_ENTRY_LENGTH,
        Name,
        MAX_ENTRY_LENGTH - 1
    );
    
    BootEntries[BootEntryCount].BootFunction = BootFunction;

    // Post-condition after this increment:
    //   - Entries live in indices [0, BootEntryCount) and are valid.
    //   - BootEntryCount never exceeds MAX_BOOT_ENTRIES.
    BootEntryCount++;
    
    return EFI_SUCCESS;
}

/**
  Draws the boot menu on the screen.
**/
VOID
DrawBootMenu() {
    const struct BootMenuTheme* theme = GetBootMenuTheme();
    // Background layer: either a full-screen image specified by the
    // theme, or a solid-color fill acting as a canvas. This is drawn
    // once per frame before we start painting higher-level UI elements.
    if (theme->background_image) {
        DrawImage(theme->background_image, 0, 0);
    } else {
        ClearScreen(theme->background_color);
    }
    INT32 ScreenWidth = GraphicsOutput->Mode->Info->HorizontalResolution;
    INT32 ScreenHeight = GraphicsOutput->Mode->Info->VerticalResolution;

    // Header band across the top of the screen. We conceptually treat the
    // screen as a 2D coordinate system [0, ScreenWidth) x [0, ScreenHeight)
    // in pixel space, with (0,0) at the top-left corner.
    DrawRect(0, 0, ScreenWidth, 60, theme->header_color);
    // We place the menu box in the center horizontally by allocating a
    // quarter-screen margin on each side and let it float vertically at a
    // fixed Y offset. The chosen dimensions (row height 50, padding 40)
    // define the geometry that both the drawing code and the mouse hit
    // testing code must agree on.
    INT32 MenuX = ScreenWidth / 4;
    INT32 MenuY = 100;
    INT32 MenuWidth = ScreenWidth / 2;
    INT32 MenuHeight = (VISIBLE_MENU_ENTRIES * 50) + 40;
    DrawRect(MenuX, MenuY, MenuWidth, MenuHeight, theme->header_color);
    INT32 TextX = MenuX + 20;
    INT32 TextY = MenuY + 20;
    const wchar_t* menu_title = GetLocalizedString("menu_title");
    // Center the title text in the header by assuming an approximate
    // glyph width of 10 pixels. This is intentionally approximate rather
    // than font-metric perfect; it trades precision for simplicity and
    // keeps the coordinates deterministic.
    PrintXY((ScreenWidth - StrLen(menu_title) * 10) / 2, 30, theme->selected_text_color, theme->header_color, L"%s", menu_title);
    // Draw visible entries
    // For each logically visible entry index i in the current window, we
    // compute the corresponding row slot. Any change to MenuScrollOffset
    // or VISIBLE_MENU_ENTRIES must preserve the invariant that the loop
    // bounds below stay within [0, BootEntryCount).
    for (UINTN i = MenuScrollOffset; i < BootEntryCount && i < MenuScrollOffset + VISIBLE_MENU_ENTRIES; i++) {
        if (i == (UINTN)SelectedEntry) {
            // Highlight band under the selected entry. Note that we offset
            // the rectangle slightly relative to TextY so the text appears
            // visually centered inside it.
            DrawRect(MenuX + 10, TextY - 5, MenuWidth - 20, 40, theme->highlight_color);
        }
        // Compose the label for this row. We allocate a small constant
        // margin (+8) over MAX_ENTRY_LENGTH to account for the optional
        // "(x) " hotkey prefix. The earlier AddBootEntry cap ensures we
        // never overflow this buffer.
        wchar_t entry_label[MAX_ENTRY_LENGTH+8];
        if (BootEntryHotkeys[i]) {
            swprintf(entry_label, sizeof(entry_label)/sizeof(wchar_t), L"(%c) %s", BootEntryHotkeys[i], BootEntries[i].Name);
        } else {
            swprintf(entry_label, sizeof(entry_label)/sizeof(wchar_t), L"%s", BootEntries[i].Name);
        }
        PrintXY(TextX, TextY, (i == (UINTN)SelectedEntry) ? theme->selected_text_color : theme->text_color, 0x00000000, L"%s", entry_label);
        TextY += 50;
    }
    // Up/down arrows if needed
    // Visual affordances for scrollability: if MenuScrollOffset is
    // strictly greater than zero, it means there exist entries above the
    // current window, so we draw an upward arrow in the upper-right.
    if (MenuScrollOffset > 0) {
        PrintXY(MenuX + MenuWidth - 40, MenuY + 10, theme->footer_color, 0x00000000, L"↑");
    }
    // Symmetrically, if the window end is strictly less than BootEntryCount
    // there exist entries below the visible area, so we draw a down arrow.
    if (MenuScrollOffset + VISIBLE_MENU_ENTRIES < BootEntryCount) {
        PrintXY(MenuX + MenuWidth - 40, MenuY + MenuHeight - 30, theme->footer_color, 0x00000000, L"↓");
    }
    const wchar_t* instructions = GetLocalizedString("instructions");
    // Same approximate-centering trick as the title, but anchored near the
    // bottom edge of the menu box.
    UINTN InstructionsWidth = StrLen(instructions) * 10;
    PrintXY((ScreenWidth - InstructionsWidth) / 2, MenuY + MenuHeight + 20, theme->footer_color, 0x00000000, L"%s", instructions);
}

/**
  Displays the boot menu and handles user input.
  
  @retval EFI_SUCCESS   A boot option was selected successfully.
  @retval Other         An error occurred or the user exited the menu.
**/
EFI_STATUS
ShowBootMenu() {
    EFI_STATUS Status;
    EFI_EVENT WaitList[1];
    UINTN WaitIndex;
    EFI_INPUT_KEY Key;
    struct MouseState mouse = {0};
    // Reset menu selection/scroll every time we enter the menu so that
    // callers do not have to remember to do this. This also avoids stale
    // state across re-entries which can feel like the menu "does not stick".
    // From a state-machine perspective, ShowBootMenu() always starts in a
    // well-defined initial configuration regardless of historical uses.
    SelectedEntry = 0;
    MenuScrollOffset = 0;

    // LoadThemeAndLanguageFromConfig(); // removed, now in main.c
    // Load localization based on configured language. The localization
    // module may already know the current language from config; we simply
    // ask for it and optionally overlay flat-file overrides.
    const char* selected_lang = GetCurrentLanguage();
    if (selected_lang && selected_lang[0]) {
        LoadLocalizationFile(selected_lang);
    } else {
        LoadLocalizationFile("en"); // Fallback to English
    }
    // Initialize graphics. If this fails we gracefully fall back to
    // UEFI text mode rendering in the loop below. Logically, this is a
    // branch in our rendering back-end: one branch uses pixel primitives,
    // the other relies on firmware text output.
    Status = InitializeGraphics();
    if (EFI_ERROR(Status)) {
        gST->ConOut->ClearScreen(gST->ConOut);
    }
    // Ensure that there is always at least one entry so the user can
    // escape back into the firmware UI even if configuration did not
    // provide any boot targets. This establishes a liveness property:
    // the menu always offers at least one action that returns control
    // to the environment instead of trapping the user.
    if (BootEntryCount == 0) {
        AddBootEntry(L"Exit to UEFI Firmware", NULL);
    }

    // (Re)build hotkeys after the entry list is fully known. Doing this
    // *after* any implicit fallback entry is added ensures that hotkeys
    // are always in sync with what the user sees.
    AssignHotkeys();
    const struct BootMenuTheme* theme = GetBootMenuTheme();
    // Mouse input is optional sugar on top of the keyboard navigation.
    // If graphics are not available we still call InitMouse(), but all
    // coordinate‑based logic is guarded behind a successful graphics init.
    InitMouse();
    while (TRUE) {
        if (!EFI_ERROR(Status)) {
            // Graphical back-end: we own the full frame and can redraw it
            // every loop iteration. The drawing routine consumes only the
            // abstract menu state; it does not perform any input.
            DrawBootMenu();
        } else {
            // Text-mode back-end: here we approximate the same information
            // using firmware text output. This is intentionally minimalistic
            // but must preserve the same interaction semantics.
            gST->ConOut->ClearScreen(gST->ConOut);
            Print(L"\r\n  BloodHorn Boot Menu\r\n\r\n");
            for (UINTN i = 0; i < BootEntryCount; i++) {
                Print(L"  %c %s\r\n", (i == (UINTN)SelectedEntry) ? '>' : ' ', BootEntries[i].Name);
            }
            Print(L"\r\n  Use arrow keys to select, Enter to boot, ESC to exit");
        }

        // Mouse support (only meaningful when graphics is active so that
        // `GraphicsOutput` is valid and the visual layout matches what we
        // use for hit‑testing).
        if (!EFI_ERROR(Status)) {
            GetMouseState(&mouse);
            INT32 MenuX = GraphicsOutput->Mode->Info->HorizontalResolution / 4;
            INT32 MenuY = 100;
            INT32 MenuWidth = GraphicsOutput->Mode->Info->HorizontalResolution / 2;
            INT32 TextX = MenuX + 20;
            INT32 TextY = MenuY + 20;
            for (UINTN i = MenuScrollOffset; i < BootEntryCount && i < MenuScrollOffset + VISIBLE_MENU_ENTRIES; i++) {
                INT32 entryY = TextY + (i - MenuScrollOffset) * 50;
                // The hit-test region for each row is defined to match the
                // drawing coordinates in DrawBootMenu: same X range, and a
                // Y slice of height 40px starting slightly above TextY.
                if (mouse.x >= TextX && mouse.x < TextX + MenuWidth - 40 && mouse.y >= entryY && mouse.y < entryY + 40) {
                    SelectedEntry = (INTN)i;
                    EnsureSelectedEntryVisible();
                    if (mouse.left_button) {
                        if (BootEntries[SelectedEntry].BootFunction != NULL) {
                            return BootEntries[SelectedEntry].BootFunction();
                        }
                        return EFI_SUCCESS;
                    }
                }
            }
        }
        // Wait for key or mouse event. In practice we only wait on the
        // keyboard event, but mouse state is sampled every frame and does
        // not block.
        WaitList[0] = gST->ConIn->WaitForKey;
        Status = gBS->WaitForEvent(1, WaitList, &WaitIndex);
        if (EFI_ERROR(Status)) {
            continue;
        }
        Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
        if (EFI_ERROR(Status)) {
            continue;
        }
        // Hotkey support: map a UnicodeChar coming from the firmware to our
        // ASCII-based hotkey table. We intentionally fold to lower-case in
        // the same way AssignHotkeys() did, so the relation is symmetric.
        if (Key.UnicodeChar) {
            CHAR16 c = Key.UnicodeChar;
            // Apply the same simple ASCII folding that AssignHotkeys uses
            // so that users can press either upper‑ or lower‑case letters.
            if (c >= L'A' && c <= L'Z') {
                c = (CHAR16)(c - L'A' + L'a');
            }
            for (UINTN i = 0; i < BootEntryCount; ++i) {
                if (BootEntryHotkeys[i] == c) {
                    SelectedEntry = (INTN)i;
                    EnsureSelectedEntryVisible();
                    break;
                }
            }
        }
        // At this point we have consumed any hotkey-like characters. We now
        // branch on two orthogonal dimensions:
        //   - Key.UnicodeChar for printable/semantic keys like Enter.
        //   - Key.ScanCode for non-printable navigation keys.
        switch (Key.UnicodeChar) {
            case CHAR_CARRIAGE_RETURN:
                if (BootEntries[SelectedEntry].BootFunction != NULL) {
                    return BootEntries[SelectedEntry].BootFunction();
                }
                return EFI_SUCCESS;
            case 0:
                switch (Key.ScanCode) {
                    case SCAN_UP:
                        // Wrap around when the user navigates past the
                        // first entry, then normalize the scroll window.
                        if (SelectedEntry > 0) {
                            SelectedEntry--;
                        } else if (BootEntryCount > 0) {
                            SelectedEntry = (INTN)(BootEntryCount - 1);
                        }
                        EnsureSelectedEntryVisible();
                        break;
                    case SCAN_DOWN:
                        // Same idea as above but moving forward; wrap to
                        // the first entry when we reach the bottom.
                        if ((UINTN)SelectedEntry < BootEntryCount - 1) {
                            SelectedEntry++;
                        } else if (BootEntryCount > 0) {
                            SelectedEntry = 0;
                        }
                        EnsureSelectedEntryVisible();
                        break;
                    case SCAN_ESC:
                        return EFI_ABORTED;
                }
                break;
        }
    }
    return EFI_SUCCESS;
}

/**
  Helper function to print text at specific coordinates with foreground and background colors.
  
  @param[in] X          The X coordinate.
  @param[in] Y          The Y coordinate.
  @param[in] FgColor    The foreground color in 0xRRGGBB format.
  @param[in] BgColor    The background color in 0xRRGGBB format.
  @param[in] Format     The format string.
  @param[in] ...        Variable arguments for the format string.
**/
VOID
EFIAPI
PrintXY(
    IN UINTN X,
    IN UINTN Y,
    IN UINT32 FgColor,
    IN UINT32 BgColor,
    IN CONST CHAR16 *Format,
    ...
) {
    VA_LIST Args;
    CHAR16 Buffer[256];
    
    // Format the string.
    //
    // We deliberately cap the internal buffer to 256 WCHARs to keep stack
    // usage bounded in firmware. Callers are expected to format short UI
    // labels, not unbounded logs.
    VA_START(Args, Format);
    UnicodeVSPrint(Buffer, sizeof(Buffer), Format, Args);
    VA_END(Args);
    
    // Set cursor position in the firmware text console coordinate system.
    gST->ConOut->SetCursorPosition(gST->ConOut, (UINTN)X, (UINTN)Y);
    
    // Map 0xRRGGBB to EFI colors (basic 16 colors, best effort).
    // Using only intensity and basic colors (not true RGB). The exact
    // mapping is intentionally coarse but deterministic.
    UINT8 fg = EFI_LIGHTGRAY;
    UINT8 bg = EFI_BLACK;
    // You can improve color mapping here if needed 
    
    gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(fg, bg));
    
    // Print the string.
    gST->ConOut->OutputString(gST->ConOut, Buffer);
    
    // Reset to default colors (white on black) so that subsequent
    // firmware output is not accidentally tinted by our attributes.
    gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK));
}
