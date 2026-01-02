/*
 * main.c 
 * 
 * Written by alexander roman
 *
 * Hey there! Welcome to the main entry point of BloodHorn, our advanced UEFI bootloader.
 * This file is where all the magic happens - we're talking about booting operating systems,
 * managing hardware, handling security, and making sure everything works smoothly whether
 * you're running on plain UEFI or the fancy Coreboot + UEFI hybrid setup.
 * 
 * What makes BloodHorn special?
 * ==============================
 * 
 * 1. **Hybrid Architecture**: We can run on both pure UEFI systems and Coreboot+UEFI hybrids.
 *    When Coreboot is available, we let it handle the low-level hardware initialization
 *    (graphics, storage, network, TPM) while UEFI provides the high-level services.
 *    This gives us the best of both worlds - Coreboot's hardware control + UEFI's compatibility.
 * 
 * 2. **Multi-Boot Support**: We're not picky about what you want to boot! Linux kernels,
 *    Multiboot1/2 kernels, Limine kernels, even chainloading other bootloaders.
 *    We support x86 (32/64-bit), ARM64, RISC-V 64, and LoongArch 64 architectures.
 *    Basically, if it's a bootable kernel, we can probably handle it.
 * 
 * 3. **Security First**: We take security seriously. TPM 2.0 integration, secure boot support,
 *    kernel hash verification, and encrypted configuration options. Your system stays safe
 *    under our watch.
 * 
 * 4. **Modern UI**: Who says bootloaders have to be ugly? We've got a graphical boot menu
 *    with themes, mouse support, multiple languages, and customizable fonts. Booting can be
 *    both functional AND beautiful.
 * 
 * 5. **Network Booting**: Need to boot from the network? We've got PXE support built right in.
 *    Perfect for diskless workstations or enterprise deployments.
 * 
 * 6. **Configuration Flexibility**: INI files, JSON files, UEFI variables - take your pick!
 *    We support multiple configuration methods and let you override settings however you want.
 * 
 * The Boot Process - Step by Step
 * ================================
 * 
 * 1. **Entry Point** (`UefiMain`): This is where everything begins. We set up the basic
 *    UEFI environment, detect Coreboot if available, and initialize our internal systems.
 * 
 * 2. **Hardware Detection**: We check what hardware and firmware we're working with.
 *    Coreboot? UEFI-only? What graphics are available? TPM? Network? We figure it all out.
 * 
 * 3. **Configuration Loading**: We load settings from bloodhorn.ini, bloodhorn.json,
 *    and UEFI variables (in that order - later ones override earlier ones).
 * 
 * 4. **UI Initialization**: Set up the graphical interface, load themes and fonts,
 *    initialize mouse support, and get everything looking pretty.
 * 
 * 5. **Boot Menu**: Show the user their options (unless autoboot kicks in first).
 *    We've got entries for all supported boot methods and architectures.
 * 
 * 6. **Kernel Loading**: Once the user picks something (or autoboot happens), we load
 *    the kernel, verify it if security is enabled, and get ready to jump to it.
 * 
 * 7. **Boot Services Exit**: This is the point of no return. We carefully exit UEFI
 *    boot services, handle all the memory map shenanigans, and prepare the system
 *    for the kernel to take over.
 * 
 * 8. **Kernel Jump**: We set up boot parameters, configure the framebuffer if needed,
 *    and finally jump to the kernel entry point. From here, the OS takes over.
 * 
 * Memory Management - The Tricky Part
 * ====================================
 * 
 * One of the biggest challenges in bootloader development is memory management.
 * We need to:
 * - Allocate memory for ourselves while UEFI boot services are running
 * - Find a good spot to load the kernel (usually above 1MB to avoid conflicts)
 * - Handle the memory map changes that happen when we exit boot services
 * - Make sure we don't step on any toes or corrupt anything important
 * 
 * The `ExitBootServicesAndExecuteKernel` function handles this delicate dance.
 * We retry the memory map/exit sequence up to 8 times because UEFI can be
 * finicky about memory maps changing between calls.
 * 
 * Error Handling - When Things Go Wrong
 * ======================================
 * 
 * Let's be real - things can and do go wrong. Hardware fails, files are missing,
 * kernels are corrupted. We handle all of this gracefully:
 * - Clear error messages that tell you what actually happened
 * - Fallback options (if autoboot fails, we show the menu)
 * - Recovery shell for when things really go south
 * - Safe reboots that don't leave the system in a weird state
 * 
 * Architecture Support - We Speak Multiple Languages
 * ================================================
 * 
 * BloodHorn isn't just for x86 anymore! We support:
 * - IA-32 (32-bit x86) - For older systems or specific use cases
 * - x86-64 (64-bit x86) - The modern standard for most desktops/servers
 * - ARM64 (aarch64) - For ARM-based systems like Raspberry Pi 4+ or ARM servers
 * - RISC-V 64 - The open-source architecture that's gaining traction
 * - LoongArch 64 - For Chinese LoongArch processors
 * 
 * Each architecture has its own quirks and requirements, but we handle them all
 * through our architecture-specific wrapper functions.
 * 
 * Coreboot Integration - The Hybrid Magic
 * =====================================
 * 
 * When running on Coreboot+UEFI, we get the best of both worlds:
 * - Coreboot handles low-level hardware initialization (graphics, storage, etc.)
 * - UEFI provides the standard boot services and compatibility
 * - We can access both Coreboot tables and UEFI protocols
 * - Better hardware support and faster boot times
 * 
 * The `InitializeBloodHorn` function detects this setup and configures everything
 * accordingly. If Coreboot is available, we use its services; otherwise, we fall back
 * to pure UEFI mode.
 * 
 * Security Features - Keeping You Safe
 * ====================================
 * 
 * Security isn't just a feature - it's a core principle:
 * - TPM 2.0 integration for hardware-rooted trust
 * - Secure Boot support for chain of trust verification
 * - Kernel hash verification to ensure you're booting what you think you're booting
 * - Encrypted configuration options
 * - Constant-time comparisons to prevent timing attacks
 * - Memory zeroization to prevent leaks
 * 
 * The Future - Where We're Headed
 * ================================
 * 
 * BloodHorn is constantly evolving. We're working on:
 * - Better plugin system for extensibility
 * - More architecture support (maybe PowerPC? Maybe MIPS? Who knows!)
 * - Enhanced security features (measured boot, attestation)
 * - Better network booting protocols
 * - Even more UI improvements
 * 
 * Contributing
 * ============
 * 
 * Think you can make BloodHorn better? We'd love your help! Check out the project
 * documentation, join our community, and let's build the best bootloader together.
 * 
 * Anyway, that's the overview. Now let's dive into the code and see how all this
 * magic actually happens...
 * 
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

// =============================================================================
// INCLUDES - All the Goodies We Need
// =============================================================================
// 
// Alright, let's bring in all the libraries and headers we need to make this
// bootloader sing. We've got UEFI headers, BloodHorn modules, and everything
// in between. Each include has a purpose - no random imports here!

#include <Uefi.h>
#include "compat.h"                    // Compatibility layer for different platforms
#include <Library/UefiLib.h>           // Basic UEFI library functions
#include <Library/UefiBootServicesTableLib.h>  // Boot services - essential for bootloader work
#include <Library/UefiRuntimeServicesTableLib.h>  // Runtime services - for after boot services exit
#include <Library/BaseMemoryLib.h>     // Memory operations (copy, zero, compare)
#include <Library/MemoryAllocationLib.h> // Memory allocation - we need this a lot!
#include <Protocol/LoadedImage.h>      // To get info about our loaded image
#include <Protocol/GraphicsOutput.h>   // Graphics Output Protocol - for pretty UI
#include <Protocol/SimpleFileSystem.h> // File system access - gotta read those kernels
#include <Protocol/DevicePath.h>       // Device path handling - for finding boot devices
#include <Protocol/Tcg2Protocol.h>     // TPM 2.0 protocol - security stuff
#include <Library/DevicePathLib.h>    // Device path utilities
#include <Library/BaseLib.h>           // Basic library functions (CPU pause, etc.)
#include <Library/PrintLib.h>         // Formatted printing - for debug messages
#include <Guid/FileInfo.h>             // File information GUID - for getting file stats

// =============================================================================
// BLOODHORN MODULES - Our Own Goodies
// =============================================================================
// 
// These are all the internal BloodHorn modules that make us special.
// Each one handles a specific part of the bootloader experience.

#include "boot/menu.h"                // The graphical boot menu - user's gateway to choices
#include "boot/theme.h"               // Theme system - because bootloaders should look good
#include "boot/localization.h"        // Multi-language support - we speak your language!
#include "boot/font.h"                // Font rendering system - for pretty text
#include "boot/mouse.h"               // Mouse support - point and click booting!
#include "boot/secure.h"              // Security features - keeping you safe
#include "fs/fat32.h"                 // FAT32 filesystem support - most common format
#include "security/crypto.h"          // Cryptographic functions - encryption, hashing
#include "security/tpm2.h"            // TPM 2.0 integration - hardware security
#include "scripting/lua.h"            // Lua scripting - for advanced customization
#include "recovery/shell.h"            // Recovery shell - when things go wrong
#include "plugins/plugin.h"            // Plugin system - extensibility
#include "net/pxe.h"                  // PXE network boot - boot from the network

// =============================================================================
// ARCHITECTURE SUPPORT - We Speak Multiple CPU Languages
// =============================================================================
// 
// Here's where we include support for all the different CPU architectures
// that BloodHorn can handle. Each architecture has its own way of doing things,
// so we need specialized code for each one.

#include "boot/Arch32/linux.h"        // Linux kernel boot (32-bit)
#include "boot/Arch32/limine.h"       // Limine protocol support
#include "boot/Arch32/multiboot1.h"   // Multiboot v1 protocol (GRUB-style)
#include "boot/Arch32/multiboot2.h"   // Multiboot v2 protocol (modern version)
#include "boot/Arch32/chainload.h"    // Chainloading other bootloaders
#include "boot/Arch32/ia32.h"         // IA-32 (32-bit x86) support
#include "boot/Arch32/x86_64.h"       // x86-64 (64-bit x86) support
#include "boot/Arch32/aarch64.h"      // ARM64 support - for ARM systems
#include "boot/Arch32/riscv64.h"      // RISC-V 64 support - the open ISA
#include "boot/Arch32/loongarch64.h"  // LoongArch 64 support - Chinese processors
#include "boot/Arch32/BloodChain/bloodchain.h"  // Our own BloodChain boot protocol

// =============================================================================
// CONFIGURATION SYSTEMS - How We Learn What You Want
// =============================================================================
// 
// We support multiple configuration formats because different people have
// different preferences. INI for simplicity, JSON for structure, and UEFI
// variables for enterprise environments.

#include "config/config_ini.h"        // INI file configuration - simple and readable
#include "config/config_json.h"       // JSON configuration - structured and modern
#include "config/config_env.h"        // Environment variable configuration
#include "boot/libb/include/bloodhorn/bloodhorn.h"  // BloodHorn library integration
#include "security/sha512.h"          // SHA-512 hashing - for kernel verification

// =============================================================================
// COREBOOT INTEGRATION - The Hybrid Magic
// =============================================================================
// 
// Coreboot is an open-source firmware that can work alongside UEFI.
// When both are available, we get the best of both worlds!

#include "coreboot/coreboot_platform.h"  // Coreboot platform integration

// =============================================================================
// GLOBAL STATE - Things We Need to Remember
// =============================================================================
// 
// These are the global variables that keep track of important state
// throughout the bootloader's lifecycle. Each one has a specific purpose.

// File hash verification structure - for security checking
// We store the expected SHA-512 hash of files we want to verify
typedef struct {
    char path[256];                    // File path (relative to boot device)
    uint8_t expected_hash[64];         // SHA-512 hash we expect (64 bytes = 512 bits)
} FILE_HASH;

// Global TPM 2.0 protocol handle - for hardware security operations
// If TPM is available, this will point to the TPM protocol interface
static EFI_TCG2_PROTOCOL* gTcg2Protocol = NULL;

// Global BloodHorn context - our internal state management
// This keeps track of all the BloodHorn library state and configuration
static bh_context_t* gBhContext = NULL;

// Global image handle - super important for UEFI operations
// We need this for ExitBootServices and loading child images
static EFI_HANDLE gImageHandle = NULL;

// Global flag - is Coreboot available on this system?
// This determines whether we run in hybrid mode or pure UEFI mode
STATIC BOOLEAN gCorebootAvailable = FALSE;

// Global font handles - for rendering text in our GUI
// We have separate fonts for regular text and headers
static bh_font_t gDefaultFont = {0};    // Regular text font
static bh_font_t gHeaderFont = {0};     // Header/title font

// Global known hashes for kernel verification (if security is enabled)
STATIC FILE_HASH g_known_hashes[1] = {0};

// Forward declaration for Coreboot main entry point
// This is the entry point when running as a Coreboot payload
extern VOID EFIAPI CorebootMain(VOID* coreboot_table, VOID* payload);

// Global flag - are we running as a Coreboot payload?
// This affects how we initialize and what services we use
STATIC BOOLEAN gRunningAsCorebootPayload = FALSE;

// =============================================================================
// BOOT CONFIGURATION STRUCTURE - What You Want to Boot and How
// =============================================================================
// 
// This structure holds all the configuration options that control how BloodHorn
// behaves. We load this from INI files, JSON files, or UEFI variables.
// Each field represents something you might want to customize about your boot experience.

typedef struct {
    char default_entry[64];           // Which boot option to select by default
    int menu_timeout;                  // How long to wait before auto-booting (seconds)
    char kernel[128];                  // Path to the default kernel to boot
    char initrd[128];                  // Path to the initrd (initial RAM disk)
    char cmdline[256];                  // Kernel command line parameters
    bool tpm_enabled;                  // Should we use TPM 2.0 for security?
    bool secure_boot;                  // Should we enable secure boot features?
    bool use_gui;                      // Should we show the graphical boot menu?
    char font_path[256];               // Path to custom font file for the GUI
    uint32_t font_size;                // Size of regular font in pixels
    uint32_t header_font_size;         // Size of header font in pixels
    char language[8];                  // Language code (e.g., "en", "fr", "de")
    bool enable_networking;            // Should we initialize network interfaces?
} BOOT_CONFIG;

// =============================================================================
// COREBOOT BOOT PARAMETERS - Passing Info to the Kernel
// =============================================================================
// 
// When we boot a kernel, especially in Coreboot mode, we need to pass it a bunch
// of information about the system. This structure contains all the details the
// kernel needs to know to take over properly.

// Coreboot boot signature and version - for kernel identification
#define COREBOOT_BOOT_SIGNATURE 0x12345678  // Magic number to identify our boot params
#define COREBOOT_BOOT_FLAG_KERNEL 0x01       // We're booting a kernel
#define COREBOOT_BOOT_FLAG_FRAMEBUFFER 0x02  // We have framebuffer info
#define COREBOOT_BOOT_FLAG_INITRD 0x04       // We have an initrd loaded

typedef struct {
    UINT32 signature;                   // Boot signature - must be COREBOOT_BOOT_SIGNATURE
    UINT32 version;                     // Boot parameters structure version
    UINT64 kernel_base;                 // Physical address where kernel is loaded
    UINT64 kernel_size;                 // Size of kernel in bytes
    UINT32 boot_flags;                  // Combination of COREBOOT_BOOT_FLAG_* flags

    // Framebuffer information - for graphics setup in the kernel
    UINT64 framebuffer_addr;            // Physical address of framebuffer memory
    UINT32 framebuffer_width;           // Width in pixels
    UINT32 framebuffer_height;          // Height in pixels
    UINT32 framebuffer_bpp;             // Bits per pixel (usually 32)
    UINT32 framebuffer_pitch;            // Bytes per line (width * bytes_per_pixel)

    // Memory information - helps kernel know what RAM is available
    UINT64 memory_size;                 // Total system memory size in bytes

    // Initrd information - initial RAM disk for early boot setup
    UINT64 initrd_addr;                 // Physical address where initrd is loaded
    UINT64 initrd_size;                 // Size of initrd in bytes

    // Command line - parameters to pass to the kernel
    // This is how you tell the kernel what root filesystem to use, 
    // what drivers to load, debug options, etc.
    CHAR8 cmdline[256];                 // Kernel command line (null-terminated ASCII)
} COREBOOT_BOOT_PARAMS;

// =============================================================================
// BOOT WRAPPER DECLARATIONS - Our Menu Options in Code Form
// =============================================================================
// 
// These are all the different boot methods we support. Each one is a wrapper
// function that gets called when the user selects that option from the boot menu.
// Think of these as the "actions" behind each menu item.

EFI_STATUS EFIAPI BootBloodchainWrapper(VOID);      // Our own BloodChain protocol
EFI_STATUS EFIAPI BootLinuxKernelWrapper(VOID);      // Standard Linux kernel boot
EFI_STATUS EFIAPI BootMultiboot2KernelWrapper(VOID); // Multiboot v2 kernels (modern)
EFI_STATUS EFIAPI BootLimineKernelWrapper(VOID);     // Limine protocol kernels
EFI_STATUS EFIAPI BootChainloadWrapper(VOID);        // Chainload other bootloaders
EFI_STATUS EFIAPI BootPxeNetworkWrapper(VOID);       // Network boot via PXE
EFI_STATUS EFIAPI BootRecoveryShellWrapper(VOID);    // Our recovery shell
EFI_STATUS EFIAPI BootUefiShellWrapper(VOID);        // UEFI shell (if available)
EFI_STATUS EFIAPI ExitToFirmwareWrapper(VOID);        // Reboot into firmware settings
EFI_STATUS EFIAPI BootIa32Wrapper(VOID);              // IA-32 (32-bit x86) kernels
EFI_STATUS EFIAPI BootX86_64Wrapper(VOID);            // x86-64 (64-bit x86) kernels
EFI_STATUS EFIAPI BootAarch64Wrapper(VOID);           // ARM64 kernels
EFI_STATUS EFIAPI BootRiscv64Wrapper(VOID);           // RISC-V 64 kernels
EFI_STATUS EFIAPI BootLoongarch64Wrapper(VOID);       // LoongArch 64 kernels

// =============================================================================
// CONFIGURATION PARSING HELPERS - Making Sense of Your Settings
// =============================================================================
// 
// These functions help us parse configuration from different sources.
// We keep them simple and focused - each one does one thing well.

/**
 * Case-insensitive string comparison
 * 
 * Sometimes we need to compare strings without caring about case.
 * "Linux", "linux", and "LINUX" should all be the same to us.
 * This helper makes that happen.
 * 
 * @param a First string to compare
 * @param b Second string to compare
 * @return TRUE if strings match (case-insensitive), FALSE otherwise
 */
STATIC BOOLEAN str_ieq(const CHAR8* a, const CHAR8* b) {
    while (*a && *b) {
        CHAR8 ca = (*a >= 'A' && *a <= 'Z') ? (*a - 'A' + 'a') : *a;
        CHAR8 cb = (*b >= 'A' && *b <= 'Z') ? (*b - 'A' + 'a') : *b;
        if (ca != cb) return FALSE;
        a++; b++;
    }
    return *a == 0 && *b == 0;
}

// =============================================================================
// FILE OPERATIONS - Reading Files from the Filesystem
// =============================================================================
// 
// Bootloaders need to read files - kernels, initrds, configuration files, etc.
// These functions handle the low-level file operations using UEFI protocols.

/**
 * Load a file from the filesystem into memory (no hash verification)
 * 
 * This is our basic file loading function. It opens a file, reads its entire
 * contents into memory, and returns a buffer to the caller. No security
 * checks here - just raw file loading.
 * 
 * @param Path Path to the file (UCS-2 string)
 * @param Buffer Output pointer for the loaded file data
 * @param Size Output pointer for the file size in bytes
 * @return EFI_SUCCESS if successful, error code otherwise
 */
STATIC
EFI_STATUS
LoadFileRaw (
  IN CHAR16* Path,
  OUT VOID** Buffer,
  OUT UINTN* Size
  )
{
    EFI_STATUS Status;
    EFI_FILE_HANDLE root_dir;
    EFI_FILE_HANDLE file;
    VOID* buffer = NULL;
    UINTN size = 0;

    // First, we need to get the root directory of the filesystem
    Status = get_root_dir(&root_dir);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Open the file for reading
    Status = root_dir->Open(root_dir, &file, Path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Get file information to determine the file size
    EFI_FILE_INFO* info = NULL;
    UINTN info_size = 0;
    Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        info = AllocateZeroPool(info_size);
        if (!info) { file->Close(file); return EFI_OUT_OF_RESOURCES; }
        Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, info);
    }
    if (EFI_ERROR(Status)) { if (info) FreePool(info); file->Close(file); return Status; }

    // Allocate memory for the file contents
    size = (UINTN)info->FileSize;
    buffer = AllocateZeroPool(size);
    if (!buffer) { FreePool(info); file->Close(file); return EFI_OUT_OF_RESOURCES; }

    // Read the entire file into our buffer
    Status = file->Read(file, &size, buffer);
    file->Close(file);
    FreePool(info);
    if (EFI_ERROR(Status)) { FreePool(buffer); return Status; }

    *Buffer = buffer;
    *Size = size;
    return EFI_SUCCESS;
}

/**
 * Parse a boolean value from an ASCII string
 * 
 * Configuration files can represent booleans in different ways.
 * This helper understands "true/1/yes" and "false/0/no" (case-insensitive).
 * If the value is unclear, we return the default.
 * 
 * @param v String value to parse
 * @param defv Default value if parsing fails
 * @return TRUE or FALSE based on the string content
 */
STATIC BOOLEAN parse_bool_ascii(const CHAR8* v, BOOLEAN defv) {
    if (!v) return defv;
    if (str_ieq(v, "true") || str_ieq(v, "1") || str_ieq(v, "yes")) return TRUE;
    if (str_ieq(v, "false") || str_ieq(v, "0") || str_ieq(v, "no")) return FALSE;
    return defv;
}

/**
 * Trim whitespace from an ASCII string
 * 
 * Configuration files often have extra whitespace that we need to clean up.
 * This function removes leading and trailing spaces, tabs, and line breaks.
 * We modify the string in-place to save memory.
 * 
 * @param s String to trim (modified in-place)
 */
STATIC VOID trim_ascii(CHAR8* s) {
    if (!s) return;
    // trim leading whitespace
    CHAR8* p = s; while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (p != s) CopyMem(s, p, AsciiStrLen(p)+1);
    // trim trailing whitespace
    UINTN n = AsciiStrLen(s);
    while (n > 0 && (s[n-1] == ' ' || s[n-1] == '\t' || s[n-1] == '\r' || s[n-1] == '\n')) { s[n-1] = 0; n--; }
}

/**
 * Read an entire file as ASCII text
 * 
 * This helper function reads a whole file into memory and ensures it's
 * null-terminated so we can treat it as a string. Perfect for reading
 * configuration files.
 * 
 * @param root Root directory handle
 * @param path File path (UCS-2 string)
 * @param outBuf Output pointer for file contents
 * @param outLen Output pointer for file length
 * @return EFI_SUCCESS if successful, error code otherwise
 */
STATIC EFI_STATUS ReadWholeFileAscii(EFI_FILE_HANDLE root, CONST CHAR16* path, CHAR8** outBuf, UINTN* outLen) {
    EFI_STATUS Status;
    EFI_FILE_HANDLE file = NULL;
    *outBuf = NULL; *outLen = 0;
    Status = root->Open(root, &file, (CHAR16*)path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) return Status;
    EFI_FILE_INFO* info = NULL; UINTN info_size = 0;
    Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        info = AllocateZeroPool(info_size);
        if (!info) { file->Close(file); return EFI_OUT_OF_RESOURCES; }
        Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, info);
    }
    if (EFI_ERROR(Status)) { if (info) FreePool(info); file->Close(file); return Status; }
    UINTN size = (UINTN)info->FileSize;
    CHAR8* buf = AllocateZeroPool(size + 1);  // +1 for null terminator
    if (!buf) { FreePool(info); file->Close(file); return EFI_OUT_OF_RESOURCES; }
    Status = file->Read(file, &size, buf);
    file->Close(file);
    FreePool(info);
    if (EFI_ERROR(Status)) { FreePool(buf); return Status; }
    buf[size] = 0;  // Ensure null termination
    *outBuf = buf; *outLen = size;
    return EFI_SUCCESS;
}

/**
 * Apply INI configuration to our boot config structure
 * 
 * INI files are simple and human-readable. We parse sections, keys,
 * and values to populate our configuration structure. This is a minimal
 * but functional INI parser that handles the basics we need.
 * 
 * @param ini INI file content as a string
 * @param config Configuration structure to populate
 */
STATIC VOID ApplyIniToConfig(const CHAR8* ini, BOOT_CONFIG* config) {
    CHAR8 section[64] = {0};
    const CHAR8* cur = ini;
    
    // Process the INI file line by line
    while (*cur) {
        // read one line
        const CHAR8* lineStart = cur;
        while (*cur && *cur != '\n') cur++;
        UINTN len = (UINTN)(cur - lineStart);
        CHAR8* line = AllocateZeroPool(len + 1);
        if (!line) return;
        CopyMem(line, lineStart, len); line[len] = 0;
        if (*cur == '\n') cur++;

        trim_ascii(line);
        
        // Skip empty lines and comments
        if (line[0] == '#' || line[0] == ';' || line[0] == 0) { FreePool(line); continue; }
        
        // Handle section headers like [boot] or [linux]
        if (line[0] == '[') {
            CHAR8* rb = AsciiStrStr(line, "]");
            if (rb) {
                *rb = 0;
                AsciiStrCpyS(section, sizeof(section), line + 1);
            }
            FreePool(line); continue;
        }
        
        // Handle key=value pairs
        CHAR8* eq = AsciiStrStr(line, "=");
        if (!eq) { FreePool(line); continue; }
        *eq = 0; CHAR8* k = line; CHAR8* v = eq + 1; trim_ascii(k); trim_ascii(v);

        // Apply the setting based on section and key
        if (str_ieq(section, "boot")) {
            if (str_ieq(k, "default")) {
                AsciiStrCpyS(config->default_entry, sizeof(config->default_entry), v);
            } else if (str_ieq(k, "menu_timeout")) {
                config->menu_timeout = (int)AsciiStrDecimalToUintn(v);
            } else if (str_ieq(k, "language")) {
                AsciiStrCpyS(config->language, sizeof(config->language), v);
            } else if (str_ieq(k, "secure_boot")) {
                config->secure_boot = parse_bool_ascii(v, config->secure_boot);
            } else if (str_ieq(k, "tpm_enabled")) {
                config->tpm_enabled = parse_bool_ascii(v, config->tpm_enabled);
            } else if (str_ieq(k, "use_gui")) {
                config->use_gui = parse_bool_ascii(v, config->use_gui);
            }
        } else if (str_ieq(section, "linux")) {
            if (str_ieq(k, "kernel")) {
                AsciiStrCpyS(config->kernel, sizeof(config->kernel), v);
            } else if (str_ieq(k, "initrd")) {
                AsciiStrCpyS(config->initrd, sizeof(config->initrd), v);
            } else if (str_ieq(k, "cmdline")) {
                AsciiStrCpyS(config->cmdline, sizeof(config->cmdline), v);
            }
        }
        FreePool(line);
    }
}

STATIC VOID ApplyJsonToConfig(const CHAR8* js, BOOT_CONFIG* config) {
    // Minimal substring extraction; this is not a full JSON parser but adequate for expected fields.
    #define FIND_STR(key, dst) do { const CHAR8* p = AsciiStrStr((CHAR8*)js, key); if (p) { p = AsciiStrStr((CHAR8*)p, ":"); if (p) { p++; while (*p==' '||*p=='\t' || *p=='\n') p++; if (*p=='\"') { p++; const CHAR8* q=p; while (*q && *q!='\"') q++; UINTN L=(UINTN)(q-p); CHAR8 tmp[512]={0}; if (L>=sizeof(tmp)) L=sizeof(tmp)-1; CopyMem(tmp,p,L); tmp[L]=0; AsciiStrCpyS(dst, sizeof(dst), tmp); } } } } while(0)
    #define FIND_INT(key, dst) do { const CHAR8* p = AsciiStrStr((CHAR8*)js, key); if (p) { p = AsciiStrStr((CHAR8*)p, ":"); if (p) { p++; while (*p==' '||*p=='\t'||*p=='\n') p++; INTN val=0; while (*p>='0'&&*p<='9'){ val = val*10 + (*p-'0'); p++; } dst = (int)val; } } } while(0)
    #define FIND_BOOL(key, dst) do { const CHAR8* p = AsciiStrStr((CHAR8*)js, key); if (p) { p = AsciiStrStr((CHAR8*)p, ":"); if (p) { p++; while (*p==' '||*p=='\t'||*p=='\n') p++; if (AsciiStrnCmp(p, "true", 4)==0) dst=TRUE; else if (AsciiStrnCmp(p,"false",5)==0) dst=FALSE; } } } while(0)

    FIND_STR("\"default\"", config->default_entry);
    FIND_INT("\"menu_timeout\"", config->menu_timeout);
    FIND_STR("\"language\"", config->language);

    // entries.linux
    FIND_STR("\"kernel\"", config->kernel);
    FIND_STR("\"initrd\"", config->initrd);
    FIND_STR("\"cmdline\"", config->cmdline);
}

STATIC VOID ApplyUefiEnvOverrides(BOOT_CONFIG* config) {
    struct { CONST CHAR16* name; enum { T_STR, T_INT, T_BOOL } typ; VOID* target; UINTN tsize; } vars[] = {
        { L"BLOODHORN_DEFAULT", T_STR,  config->default_entry, sizeof(config->default_entry) },
        { L"BLOODHORN_MENU_TIMEOUT", T_INT, &config->menu_timeout, sizeof(config->menu_timeout) },
        { L"BLOODHORN_LANGUAGE", T_STR,  config->language, sizeof(config->language) },
        { L"BLOODHORN_LINUX_KERNEL", T_STR,  config->kernel, sizeof(config->kernel) },
        { L"BLOODHORN_LINUX_INITRD", T_STR,  config->initrd, sizeof(config->initrd) },
        { L"BLOODHORN_LINUX_CMDLINE", T_STR,  config->cmdline, sizeof(config->cmdline) },
        { L"BLOODHORN_SECURE_BOOT", T_BOOL, &config->secure_boot, sizeof(config->secure_boot) },
        { L"BLOODHORN_TPM_ENABLED", T_BOOL, &config->tpm_enabled, sizeof(config->tpm_enabled) },
    };

    for (UINTN i = 0; i < ARRAY_SIZE(vars); ++i) {
        UINTN sz = 0; EFI_STATUS st;
        st = gRT->GetVariable((CHAR16*)vars[i].name, &gEfiGlobalVariableGuid, NULL, &sz, NULL);
        if (st != EFI_BUFFER_TOO_SMALL) continue;
        VOID* buf = AllocateZeroPool(sz);
        if (!buf) continue;
        st = gRT->GetVariable((CHAR16*)vars[i].name, &gEfiGlobalVariableGuid, NULL, &sz, buf);
        if (EFI_ERROR(st)) { FreePool(buf); continue; }
        if (vars[i].typ == T_STR) {
            // treat as UCS-2 string -> convert to ASCII
            CHAR16* w = (CHAR16*)buf; CHAR8 a[256] = {0};
            UnicodeStrToAsciiStrS(w, a, sizeof(a));
            AsciiStrCpyS((CHAR8*)vars[i].target, vars[i].tsize, a);
        } else if (vars[i].typ == T_INT) {
            if (sz >= sizeof(UINT32)) *(int*)vars[i].target = (int)(*(UINT32*)buf);
        } else if (vars[i].typ == T_BOOL) {
            if (sz >= sizeof(UINT8)) *(BOOLEAN*)vars[i].target = (*(UINT8*)buf) ? TRUE : FALSE;
        }
        FreePool(buf);
    }
}

/**
 * Load boot configuration from files
 */
STATIC
EFI_STATUS
LoadBootConfig (
  OUT BOOT_CONFIG* config
  )
{
    // Defaults
    AsciiStrCpyS(config->default_entry, sizeof(config->default_entry), "linux");
    config->menu_timeout = 10;
    config->tpm_enabled = TRUE;
    config->secure_boot = FALSE;
    config->use_gui = TRUE;
    AsciiStrCpyS(config->font_path, sizeof(config->font_path), "DejaVuSans.ttf");
    config->font_size = 12;
    config->header_font_size = 16;
    AsciiStrCpyS(config->language, sizeof(config->language), "en");
    config->enable_networking = FALSE;
    config->kernel[0] = 0; config->initrd[0] = 0; config->cmdline[0] = 0;

    EFI_STATUS Status;
    EFI_FILE_HANDLE root_dir;
    Status = get_root_dir(&root_dir);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open filesystem for config: %r\n", Status);
        return EFI_SUCCESS; // keep defaults, do not fail boot
    }

    // 1) INI: bloodhorn.ini
    CHAR8* buf = NULL; UINTN blen = 0;
    Status = ReadWholeFileAscii(root_dir, L"bloodhorn.ini", &buf, &blen);
    if (!EFI_ERROR(Status) && buf && blen > 0) {
        ApplyIniToConfig(buf, config);
        FreePool(buf); buf = NULL; blen = 0;
    }

    // 2) JSON: bloodhorn.json
    Status = ReadWholeFileAscii(root_dir, L"bloodhorn.json", &buf, &blen);
    if (!EFI_ERROR(Status) && buf && blen > 0) {
        ApplyJsonToConfig(buf, config);
        FreePool(buf); buf = NULL; blen = 0;
    }

    // 3) Environment variables (UEFI vars)
    ApplyUefiEnvOverrides(config);

    // Clamp values according to docs
    if (config->menu_timeout < 0) config->menu_timeout = 0;
    if (config->menu_timeout > 300) config->menu_timeout = 300;

    return EFI_SUCCESS;
}

/**
 * Initialize BloodHorn bootloader under Coreboot + UEFI hybrid mode
 */
STATIC
EFI_STATUS
InitializeBloodHorn (
  VOID
  )
{
    EFI_STATUS Status;

    // Check for Coreboot firmware and initialize if present
    gCorebootAvailable = CorebootPlatformInit();

    if (gCorebootAvailable) {
        Print(L"BloodHorn Bootloader (Coreboot + UEFI Hybrid Mode)\n");
        Print(L"Coreboot firmware detected - using hybrid initialization\n");

        // Use Coreboot for hardware initialization when available
        if (CorebootInitGraphics()) {
            Print(L"Graphics initialized using Coreboot framebuffer\n");
        }

        if (CorebootInitStorage()) {
            Print(L"Storage initialized by Coreboot\n");
        }

        if (CorebootInitNetwork()) {
            Print(L"Network initialized by Coreboot\n");
        }

        if (CorebootInitTpm()) {
            Print(L"TPM initialized by Coreboot\n");
        }

        // Use UEFI for higher-level services
        Print(L"Using UEFI services for boot menu and file operations\n");
    } else {
        Print(L"BloodHorn Bootloader (UEFI Mode)\n");
        Print(L"Coreboot firmware not detected - using UEFI initialization\n");
    }

    return EFI_SUCCESS;
}

// =============================================================================
// MAIN ENTRY POINT - Where the Boot Journey Begins!
// =============================================================================
// 
// This is it! The main function that gets called when BloodHorn starts.
// We're running in UEFI environment, and our job is to:
// 1. Set up the basic environment
// 2. Detect what hardware/firmware we're working with
// 3. Load configuration
// 4. Show the boot menu (or autoboot)
// 5. Load and execute the chosen kernel
// 
// This function is the heart of BloodHorn - everything starts here!

/**
 * Main entry point for BloodHorn bootloader under Coreboot + UEFI hybrid
 * 
 * Welcome to the show! This is where BloodHorn comes to life. We're called
 * by the UEFI firmware, and from here we orchestrate the entire boot process.
 * 
 * What happens in here:
 * - Set up UEFI system table pointers
 * - Initialize our hybrid Coreboot+UEFI environment
 * - Load configuration from multiple sources
 * - Set up the graphical interface
 * - Handle autoboot or show the boot menu
 * - Load and execute the chosen kernel
 * 
 * @param ImageHandle Our EFI image handle (needed for various operations)
 * @param SystemTable Pointer to the UEFI system table (our gateway to UEFI services)
 * @return EFI_STATUS Success if we successfully boot a kernel, error code otherwise
 */
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput = NULL;

    // First things first - set up our global pointers to UEFI services
    // These are our lifelines to the UEFI firmware services
    gST = SystemTable;                    // System table - console, runtime services
    gBS = SystemTable->BootServices;       // Boot services - memory, protocols, etc.
    gRT = SystemTable->RuntimeServices;    // Runtime services - after boot services exit
    gImageHandle = ImageHandle;            // Save our image handle for later use

    // Get information about our loaded image - we need this to find our device
    Status = gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage);
    if (EFI_ERROR(Status)) return Status;

    // Try to get the graphics output protocol - for our fancy GUI
    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&GraphicsOutput);

    // Set up the console - clear the screen and reset text output
    gST->ConOut->Reset(gST->ConOut, FALSE);
    gST->ConOut->SetMode(gST->ConOut, 0);
    gST->ConOut->ClearScreen(gST->ConOut);

    // Initialize BloodHorn in hybrid mode (detect Coreboot, set up hardware)
    // This is where we figure out if we're running with Coreboot or pure UEFI
    Status = InitializeBloodHorn();
    if (EFI_ERROR(Status)) {
        Print(L"Failed to initialize BloodHorn: %r\n", Status);
        return Status;
    }

    // Initialize BloodHorn library with UEFI system table integration.
    // Memory-map access goes through a Rust-based shim to avoid unsafe
    // casting between incompatible function pointer types.
    extern bh_status_t bhshim_get_memory_map(
        bh_memory_descriptor_t **Map,
        bh_size_t *MapSize,
        bh_size_t *DescriptorSize
    );

    bh_system_table_t bloodhorn_system_table = {
        // Memory management (use UEFI services)
        .alloc = (void* (*)(bh_size_t))AllocatePool,
        .free = (void (*)(void*))FreePool,

        // Console output (redirect to UEFI console)
        .putc = bh_uefi_putc,
        .puts = bh_uefi_puts,
        .printf = (void (*)(const char*, ...))Print,

        // Memory map: use Rust-backed shim wrapper via C glue
        .get_memory_map = bhshim_get_memory_map,

        // Graphics (use UEFI Graphics Output Protocol or Coreboot framebuffer)
        .get_graphics_info = NULL, // Graphics are handled outside libb

        // ACPI and firmware tables (use UEFI or Coreboot tables)
        .get_rsdp = (void* (*)(void))GetRsdp,
        .get_boot_device = NULL, // Platform specific

        // Power management (use UEFI runtime services)
        .reboot = bh_uefi_reboot,
        .shutdown = bh_uefi_shutdown,

        // Debugging
        .debug_break = bh_uefi_debug_break
    };

    // Initialize the BloodHorn library
    bh_status_t bh_status = bh_initialize(&bloodhorn_system_table);
    if (bh_status != BH_SUCCESS) {
        Print(L"Warning: BloodHorn library initialization failed: %a\n", bh_status_to_string(bh_status));
    } else {
        Print(L"BloodHorn library initialized successfully\n");

        uint32_t major, minor, patch;
        bh_get_version(&major, &minor, &patch);
        Print(L"BloodHorn Library v%d.%d.%d\n", major, minor, patch);
    }

    // Load configuration (INI -> JSON -> UEFI vars)
    BOOT_CONFIG config;
    LoadBootConfig(&config);

    // Apply language and font from configuration before any UI
    SetLanguage(config.language);
    if (config.font_path[0] != '\0') {
        Font* user_font = LoadFontFile(config.font_path);
        if (user_font) {
            SetDefaultFont(user_font);
        }
    }

    // Theme loading (language already applied from config)
    LoadThemeAndLanguageFromConfig();
    InitMouse();

    // Pre-menu autoboot based on configuration
    BOOLEAN showMenu = TRUE;
    if (config.menu_timeout > 0) {
        // Countdown; any key press cancels and shows menu
        INTN secs = config.menu_timeout;
        while (secs > 0) {
            Print(L"Auto-boot '%a' in %d seconds... Press any key to open menu.\r\n", config.default_entry, secs);
            // Wait up to 1 second, polling key every 100ms
            BOOLEAN keyPressed = FALSE;
            for (int i = 0; i < 10; i++) {
                EFI_INPUT_KEY Key;
                if (!EFI_ERROR(gST->ConIn->ReadKeyStroke(gST->ConIn, &Key))) { keyPressed = TRUE; break; }
                gBS->Stall(100000); // 100ms
            }
            if (keyPressed) { showMenu = TRUE; break; }
            secs--;
            if (secs == 0) {
                showMenu = FALSE;
            }
        }
    }

    // If no key pressed within timeout, autoboot supported default
    if (!showMenu) {
        if (AsciiStrCmp(config.default_entry, "linux") == 0 && config.kernel[0] != 0) {
            const char* k = config.kernel;
            const char* i = (config.initrd[0] != 0) ? config.initrd : NULL;
            const char* c = (config.cmdline[0] != 0) ? config.cmdline : "";
            EFI_STATUS ls = linux_load_kernel(k, i, c);
            if (!EFI_ERROR(ls)) {
                return EFI_SUCCESS;
            } else {
                Print(L"Autoboot linux failed: %r\n", ls);
                showMenu = TRUE;
            }
        } else {
            // Unsupported default entry fallback to menu
            showMenu = TRUE;
        }
    }

    AddBootEntry(L"BloodChain Boot Protocol", BootBloodchainWrapper);
    AddBootEntry(L"Linux Kernel", BootLinuxKernelWrapper);
    AddBootEntry(L"Multiboot2 Kernel", BootMultiboot2KernelWrapper);
    AddBootEntry(L"Limine Kernel", BootLimineKernelWrapper);
    AddBootEntry(L"Chainload Bootloader", BootChainloadWrapper);
    AddBootEntry(L"PXE Network Boot", BootPxeNetworkWrapper);
    AddBootEntry(L"IA-32 (32-bit x86)", BootIa32Wrapper);
    AddBootEntry(L"x86-64 (64-bit x86)", BootX86_64Wrapper);
    AddBootEntry(L"ARM64 (aarch64)", BootAarch64Wrapper);
    AddBootEntry(L"RISC-V 64", BootRiscv64Wrapper);
    AddBootEntry(L"LoongArch 64", BootLoongarch64Wrapper);
    AddBootEntry(L"Recovery Shell", BootRecoveryShellWrapper);
    AddBootEntry(L"UEFI Shell", (EFI_STATUS (*)(void))BootUefiShellWrapper);
    AddBootEntry(L"Exit to UEFI Firmware", ExitToFirmwareWrapper);

    Status = showMenu ? ShowBootMenu() : EFI_NOT_READY;
    if (Status == EFI_SUCCESS) {
        VOID* KernelBuffer = NULL;
        UINTN KernelSize = 0;
        Status = LoadAndVerifyKernel(L"kernel.efi", &KernelBuffer, &KernelSize);
        if (!EFI_ERROR(Status)) {
            Status = ExecuteKernel(KernelBuffer, KernelSize, NULL);
            if (!EFI_ERROR(Status)) return EFI_SUCCESS;
        }
    }

    Print(L"\r\n  No bootable device found or kernel failed.\r\n");
    Print(L"  Press any key to reboot...");
    EFI_INPUT_KEY Key;
    UINTN Index;
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Index);
    gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

    // Use Coreboot for reboot if available, otherwise use UEFI
    if (gCorebootAvailable) {
        CorebootReboot();
    } else {
        gST->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    }

    return EFI_DEVICE_ERROR;
}

// Helper function to convert UEFI putc to bh_putc
static void bh_uefi_putc(char c) {
    CHAR16 C[2] = {c, 0};
    gST->ConOut->OutputString(gST->ConOut, C);
}

// Helper function to convert UEFI puts to bh_puts
static void bh_uefi_puts(const char* s) {
    while (*s) {
        bh_uefi_putc(*s++);
    }
}

// Helper function for UEFI reboot
static void bh_uefi_reboot(void) {
    gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
}

// Helper function for UEFI shutdown
static void bh_uefi_shutdown(void) {
    gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}

// Helper function for debug break
static void bh_uefi_debug_break(void) {
#if defined(__i386__) || defined(__x86_64__)
    __asm__ volatile("int $3");
#else
    /* On non-x86 platforms, there's no INT3; use a portable hang to aid debugging */
    for (;;) { __asm__ volatile ("nop"); }
#endif
}

// Boot wrapper implementations
EFI_STATUS EFIAPI BootLinuxKernelWrapper(VOID) {
    return linux_load_kernel("/boot/vmlinuz", "/boot/initrd.img", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootMultiboot2KernelWrapper(VOID) {
    return multiboot2_load_kernel("/boot/vmlinuz-mb2", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootLimineKernelWrapper(VOID) {
    return limine_load_kernel("/boot/vmlinuz-limine", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootChainloadWrapper(VOID) {
    return chainload_file("/boot/grub2.bin");
}

EFI_STATUS EFIAPI BootPxeNetworkWrapper(VOID) {
    EFI_STATUS Status;

    Status = InitializeNetwork();
    if (EFI_ERROR(Status)) {
        Print(L"Failed to initialize network\n");
        return Status;
    }

    Status = BootFromNetwork(
        L"/boot/kernel.efi",
        L"/boot/initrd.img",
        "console=ttyS0"
    );

    if (EFI_ERROR(Status)) {
        Print(L"Network boot failed: %r\n", Status);
    }

    return Status;
}

EFI_STATUS EFIAPI BootRecoveryShellWrapper(VOID) {
    return shell_start();
}

EFI_STATUS EFIAPI BootUefiShellWrapper(VOID) {
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage = NULL;
    Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&LoadedImage);
    if (EFI_ERROR(Status)) return Status;

    CONST CHAR16* Candidates[] = {
        L"\\EFI\\BOOT\\Shell.efi",
        L"\\EFI\\tools\\Shell.efi",
        L"\\Shell.efi"
    };

    for (UINTN i = 0; i < ARRAY_SIZE(Candidates); ++i) {
        Status = LoadAndStartImageFromPath(gImageHandle, LoadedImage->DeviceHandle, Candidates[i]);
        if (!EFI_ERROR(Status)) return EFI_SUCCESS;
    }

    Print(L"UEFI Shell not found on this system.\n");
    return EFI_NOT_FOUND;
}

EFI_STATUS EFIAPI ExitToFirmwareWrapper(VOID) {
    if (gCorebootAvailable) {
        CorebootReboot();
    } else {
        gST->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    }
    return EFI_SUCCESS;
}

// BloodChain Boot Protocol implementation
EFI_STATUS EFIAPI BootBloodchainWrapper(VOID) {
    EFI_STATUS Status;
    EFI_PHYSICAL_ADDRESS KernelBase = 0x100000; // 1MB mark
    EFI_PHYSICAL_ADDRESS BcbpBase;

    // Allocate memory for BCBP header (4KB aligned)
    Status = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData,
                              EFI_SIZE_TO_PAGES(64 * 1024), &BcbpBase);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to allocate memory for BCBP header\n");
        return Status;
    }

    // Initialize BCBP header
    struct bcbp_header* hdr = (struct bcbp_header*)(UINTN)BcbpBase;
    bcbp_init(hdr, KernelBase, 0);  // 0 for boot device (set by bootloader)

    // Load kernel
    const char* kernel_path = "kernel.elf";
    const char* initrd_path = "initrd.img";
    const char* cmdline = "root=/dev/sda1 ro";

    // Load kernel into memory
    EFI_PHYSICAL_ADDRESS KernelLoadAddr = KernelBase;
    UINTN KernelSize = 0;

    Status = LoadFileToMemory(kernel_path, &KernelLoadAddr, &KernelSize);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to load kernel: %r\n", Status);
        return Status;
    }

    // Add kernel module
    bcbp_add_module(hdr, KernelLoadAddr, KernelSize, "kernel",
                   BCBP_MODTYPE_KERNEL, cmdline);

    // Load initrd if it exists
    EFI_PHYSICAL_ADDRESS InitrdLoadAddr = KernelLoadAddr + ALIGN_UP(KernelSize, 0x1000);
    UINTN InitrdSize = 0;

    if (FileExists(initrd_path)) {
        Status = LoadFileToMemory(initrd_path, &InitrdLoadAddr, &InitrdSize);
        if (!EFI_ERROR(Status) && InitrdSize > 0) {
            bcbp_add_module(hdr, InitrdLoadAddr, InitrdSize, "initrd",
                          BCBP_MODTYPE_INITRD, NULL);
        }
    }

    // Set up ACPI and SMBIOS if available
    EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER* Rsdp = NULL;
    EFI_CONFIGURATION_TABLE* ConfigTable = gST->ConfigurationTable;

    for (UINTN i = 0; i < gST->NumberOfTableEntries; i++) {
        if (CompareGuid(&ConfigTable[i].VendorGuid, &gEfiAcpi20TableGuid)) {
            Rsdp = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)ConfigTable[i].VendorTable;
            break;
        }
    }

    if (Rsdp) {
        bcbp_set_acpi_rsdp(hdr, (UINT64)(UINTN)Rsdp);
    }

    // Find SMBIOS table
    for (UINTN i = 0; i < gST->NumberOfTableEntries; i++) {
        if (CompareGuid(&ConfigTable[i].VendorGuid, &gEfiSmbiosTableGuid)) {
            bcbp_set_smbios(hdr, (UINT64)(UINTN)ConfigTable[i].VendorTable);
            break;
        }
    }

    // Set framebuffer information if available
    EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop = NULL;
    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (void**)&Gop);
    if (!EFI_ERROR(Status) && Gop) {
        bcbp_set_framebuffer(hdr, Gop->Mode->FrameBufferBase);
    }

    // Set secure boot status
    UINT8 SecureBoot = 0;
    UINTN DataSize = sizeof(SecureBoot);
    if (EFI_ERROR(gRT->GetVariable(L"SecureBoot", &gEfiGlobalVariableGuid,
                                 NULL, &DataSize, &SecureBoot)) == EFI_SUCCESS) {
        hdr->secure_boot = SecureBoot;
    }

    // Set UEFI 64-bit flag
    hdr->uefi_64bit = (sizeof(UINTN) == 8) ? 1 : 0;

    // Validate BCBP structure
    if (bcbp_validate(hdr) != 0) {
        Print(L"Invalid BCBP structure\n");
        return EFI_LOAD_ERROR;
    }

    // Print boot information
    Print(L"Booting with BloodChain Boot Protocol\n");
    Print(L"  Kernel: 0x%llx (%u bytes)\n", KernelLoadAddr, KernelSize);
    if (InitrdSize > 0) {
        Print(L"  Initrd: 0x%llx (%u bytes)\n", InitrdLoadAddr, InitrdSize);
    }
    Print(L"  Command line: %a\n", cmdline);

    // Jump to kernel
    typedef void (*KernelEntry)(struct bcbp_header*);
    KernelEntry EntryPoint = (KernelEntry)(UINTN)KernelLoadAddr;

    // Properly exit boot services (robustly handle map changes/races)
    UINTN MapSize = 0, MapKey = 0, DescSize = 0;
    UINT32 DescVer = 0;
    EFI_MEMORY_DESCRIPTOR* MemMap = NULL;
    EFI_STATUS EStatus = EFI_SUCCESS;

    const int max_retries = 8;
    int attempt = 0;
    while (attempt++ < max_retries) {
        MapSize = 0;
        EStatus = gBS->GetMemoryMap(&MapSize, MemMap, &MapKey, &DescSize, &DescVer);
        if (EStatus == EFI_BUFFER_TOO_SMALL) {
            if (MemMap) { FreePool(MemMap); MemMap = NULL; }
            MemMap = AllocatePool(MapSize);
            if (!MemMap) { EStatus = EFI_OUT_OF_RESOURCES; break; }
            EStatus = gBS->GetMemoryMap(&MapSize, MemMap, &MapKey, &DescSize, &DescVer);
        }

        if (EFI_ERROR(EStatus)) {
            /* Unexpected error; retry a couple times */
            CpuPause();
            continue;
        }

        /* Try to exit boot services; if it fails with EFI_INVALID_PARAMETER the map changed -> retry */
        EStatus = gBS->ExitBootServices(gImageHandle, MapKey);
        if (EStatus == EFI_INVALID_PARAMETER) {
            /* Map changed between GetMemoryMap and ExitBootServices; retry */
            CpuPause();
            continue;
        }

        /* On success or other error, break and handle below */
        break;
    }

    if (MemMap) { FreePool(MemMap); MemMap = NULL; }
    if (EFI_ERROR(EStatus)) {
        Print(L"Failed to exit boot services (status=%r)\n", EStatus);
        return EFI_LOAD_ERROR;
    }

    EntryPoint(hdr);

    return EFI_LOAD_ERROR;
}

// Architecture-specific boot wrappers
EFI_STATUS EFIAPI BootIa32Wrapper(VOID) {
    return ia32_load_kernel("/boot/vmlinuz-ia32", "/boot/initrd-ia32.img", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootX86_64Wrapper(VOID) {
    return x86_64_load_kernel("/boot/vmlinuz-x86_64", "/boot/initrd-x86_64.img", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootAarch64Wrapper(VOID) {
    return aarch64_load_kernel("/boot/Image-aarch64", "/boot/initrd-aarch64.img", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootRiscv64Wrapper(VOID) {
    return riscv64_load_kernel("/boot/Image-riscv64", "/boot/initrd-riscv64.img", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootLoongarch64Wrapper(VOID) {
    return loongarch64_load_kernel("/boot/Image-loongarch64", "/boot/initrd-loongarch64.img", "root=/dev/sda1 ro");
}

// Network and utility functions
EFI_STATUS InitializeNetwork(VOID) {
    int rc = pxe_network_init();
    return (rc == 0) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

VOID ShutdownNetwork(VOID) {
    pxe_cleanup_network();
}

EFI_STATUS BootFromNetwork(CONST CHAR16* kernel_path, CONST CHAR16* initrd_path, CONST CHAR8* cmdline) {
    CHAR8 kernel_ascii[256] = {0};
    CHAR8 initrd_ascii[256] = {0};
    if (kernel_path) UnicodeStrToAsciiStrS(kernel_path, kernel_ascii, sizeof(kernel_ascii));
    if (initrd_path) UnicodeStrToAsciiStrS(initrd_path, initrd_ascii, sizeof(initrd_ascii));

    const char* initrd = initrd_path ? (const char*)initrd_ascii : NULL;
    const char* cmd = cmdline ? (const char*)cmdline : "";

    int rc = pxe_boot_kernel((const char*)kernel_ascii, initrd, cmd);
    return (rc == 0) ? EFI_SUCCESS : EFI_LOAD_ERROR;
}

STATIC EFI_STATUS LoadAndStartImageFromPath(EFI_HANDLE ParentImage, EFI_HANDLE DeviceHandle, CONST CHAR16* Path) {
    EFI_STATUS Status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* Fs = NULL;
    EFI_FILE_PROTOCOL* Root = NULL;
    EFI_FILE_PROTOCOL* File = NULL;
    EFI_DEVICE_PATH_PROTOCOL* FilePath = NULL;
    VOID* Buffer = NULL;
    UINTN Size = 0;

    Status = gBS->HandleProtocol(DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&Fs);
    if (EFI_ERROR(Status)) return Status;
    Status = Fs->OpenVolume(Fs, &Root);
    if (EFI_ERROR(Status)) return Status;

    Status = Root->Open(Root, &File, (CHAR16*)Path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) { Root->Close(Root); return Status; }

    EFI_FILE_INFO* Info = NULL;
    UINTN InfoSize = 0;
    Status = File->GetInfo(File, &gEfiFileInfoGuid, &InfoSize, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        Info = AllocateZeroPool(InfoSize);
        if (!Info) { File->Close(File); Root->Close(Root); return EFI_OUT_OF_RESOURCES; }
        Status = File->GetInfo(File, &gEfiFileInfoGuid, &InfoSize, Info);
    }
    if (EFI_ERROR(Status)) { if (Info) FreePool(Info); File->Close(File); Root->Close(Root); return Status; }
    Size = (UINTN)Info->FileSize;
    Buffer = AllocateZeroPool(Size);
    if (!Buffer) { FreePool(Info); File->Close(File); Root->Close(Root); return EFI_OUT_OF_RESOURCES; }
    Status = File->Read(File, &Size, Buffer);
    File->Close(File);
    Root->Close(Root);
    FreePool(Info);
    if (EFI_ERROR(Status)) { FreePool(Buffer); return Status; }

    FilePath = FileDevicePath(DeviceHandle, Path);
    if (!FilePath) { FreePool(Buffer); return EFI_OUT_OF_RESOURCES; }
    EFI_HANDLE Child = NULL;
    Status = gBS->LoadImage(FALSE, ParentImage, FilePath, Buffer, Size, &Child);
    FreePool(FilePath);
    FreePool(Buffer);
    if (EFI_ERROR(Status)) return Status;
    return gBS->StartImage(Child, NULL, NULL);
}

/**
 * Load and verify kernel from filesystem
 */
STATIC
EFI_STATUS
LoadAndVerifyKernel (
  IN CHAR16* KernelPath,
  OUT VOID** KernelBuffer,
  OUT UINTN* KernelSize
  )
{
    EFI_STATUS Status;
    EFI_FILE_HANDLE root_dir;
    EFI_FILE_HANDLE file;
    VOID* buffer = NULL;
    UINTN size = 0;

    // Get root directory
    Status = get_root_dir(&root_dir);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get root directory\n");
        return Status;
    }

    // Open kernel file
    Status = root_dir->Open(root_dir, &file, KernelPath, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open kernel file: %s\n", KernelPath);
        return Status;
    }

    // Get file size
    EFI_FILE_INFO* info = NULL;
    UINTN info_size = 0;
    Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        info = AllocateZeroPool(info_size);
        if (!info) {
            file->Close(file);
            return EFI_OUT_OF_RESOURCES;
        }
        Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, info);
    }

    if (EFI_ERROR(Status)) {
        if (info) FreePool(info);
        file->Close(file);
        return Status;
    }

    size = (UINTN)info->FileSize;
    buffer = AllocateZeroPool(size);
    if (!buffer) {
        FreePool(info);
        file->Close(file);
        return EFI_OUT_OF_RESOURCES;
    }

    // Read kernel into memory
    Status = file->Read(file, &size, buffer);
    file->Close(file);
    FreePool(info);

    if (EFI_ERROR(Status)) {
        FreePool(buffer);
        return Status;
    }

    *KernelBuffer = buffer;
    *KernelSize = size;

    // Verify kernel hash if security is enabled
    if (g_known_hashes[0].expected_hash[0] != 0) {
        crypto_sha512_ctx_t ctx;
        uint8_t actual_hash[64];

        crypto_sha512_init(&ctx);
        crypto_sha512_update(&ctx, buffer, (uint32_t)size);
        crypto_sha512_final(&ctx, actual_hash);

        if (CompareMem(actual_hash, g_known_hashes[0].expected_hash, 64) != 0) {
            Print(L"Kernel hash verification failed!\n");
            FreePool(buffer);
            return EFI_SECURITY_VIOLATION;
        }
    }

    return EFI_SUCCESS;
}

/**
 * Execute loaded kernel in hybrid EDK2 + Coreboot environment
 */
STATIC
EFI_STATUS
ExecuteKernel (
  IN VOID* KernelBuffer,
  IN UINTN KernelSize,
  IN VOID* InitrdBuffer OPTIONAL
  )
{
    EFI_STATUS Status;

    Print(L"Executing kernel at 0x%llx (%u bytes)\n",
          (UINT64)(UINTN)KernelBuffer, KernelSize);

    // Set up execution environment based on available firmware
    if (gCorebootAvailable) {
        // Use Coreboot memory management for kernel execution
        Status = ExecuteKernelWithCoreboot(KernelBuffer, KernelSize, InitrdBuffer);
    } else {
        // Use UEFI services for kernel execution
        Status = ExecuteKernelWithUefi(KernelBuffer, KernelSize, InitrdBuffer);
    }

    return Status;
}

/**
 * Execute kernel using Coreboot services
 */
STATIC
EFI_STATUS
ExecuteKernelWithCoreboot (
  IN VOID* KernelBuffer,
  IN UINTN KernelSize,
  IN VOID* InitrdBuffer OPTIONAL
  )
{
    EFI_STATUS Status;

    // Get Coreboot memory map for proper kernel setup
    UINT32 mem_map_count = 0;
    CONST COREBOOT_MEM_ENTRY* mem_map = CorebootGetMemoryMap(&mem_map_count);

    if (!mem_map || mem_map_count == 0) {
        Print(L"Failed to get Coreboot memory map for kernel execution\n");
        return EFI_DEVICE_ERROR;
    }

    // Find suitable memory region for kernel execution
    UINT64 kernel_base = 0;
    UINT64 largest_size = 0;

    for (UINT32 i = 0; i < mem_map_count; i++) {
        if (mem_map[i].type == CB_MEM_RAM && mem_map[i].size > largest_size) {
            kernel_base = mem_map[i].addr;
            largest_size = mem_map[i].size;
        }
    }

    if (kernel_base == 0) {
        Print(L"No suitable RAM region found for kernel execution\n");
        return EFI_DEVICE_ERROR;
    }

    // Set up kernel execution environment
    Print(L"Setting up kernel execution environment...\n");
    Print(L"Kernel base: 0x%llx, Size: %u bytes\n", kernel_base, KernelSize);

    // Configure framebuffer if graphics available
    CONST COREBOOT_FB* framebuffer = CorebootGetFramebuffer();
    if (framebuffer) {
        Print(L"Configuring Coreboot framebuffer for kernel\n");
        Print(L"Framebuffer: 0x%llx, %ux%u, %u bpp\n",
              framebuffer->physical_address,
              framebuffer->x_resolution,
              framebuffer->y_resolution,
              framebuffer->bits_per_pixel);
    }

    // Set up boot parameters in Coreboot format
    // Set up proper boot parameter structure that kernel can access
    UINT64 boot_params_addr = kernel_base + 0x1000; // 4KB after kernel

    // Set up Coreboot boot parameters structure
    COREBOOT_BOOT_PARAMS* boot_params = (COREBOOT_BOOT_PARAMS*)boot_params_addr;

    // Zero out the boot parameters structure
    SetMem(boot_params, sizeof(COREBOOT_BOOT_PARAMS), 0);

    // Set up basic boot parameters
    boot_params->signature = COREBOOT_BOOT_SIGNATURE;
    boot_params->version = 1;
    boot_params->kernel_base = kernel_base;
    boot_params->kernel_size = KernelSize;
    boot_params->boot_flags = COREBOOT_BOOT_FLAG_KERNEL;

    // Set up framebuffer information if available
    if (framebuffer) {
        boot_params->framebuffer_addr = framebuffer->physical_address;
        boot_params->framebuffer_width = framebuffer->x_resolution;
        boot_params->framebuffer_height = framebuffer->y_resolution;
        boot_params->framebuffer_bpp = framebuffer->bits_per_pixel;
        boot_params->framebuffer_pitch = framebuffer->bytes_per_line;
        boot_params->boot_flags |= COREBOOT_BOOT_FLAG_FRAMEBUFFER;
    }

    // Set up memory information
    UINT64 total_memory = CorebootGetTotalMemory();
    boot_params->memory_size = total_memory;

    // Set up initrd if provided
    if (InitrdBuffer) {
        boot_params->initrd_addr = (UINT64)(UINTN)InitrdBuffer;
        boot_params->boot_flags |= COREBOOT_BOOT_FLAG_INITRD;
    }

    /* Ensure kernel is placed at chosen Coreboot region. The Coreboot memory map
       provides physical addresses; copy the kernel buffer into the selected
       kernel_base region so boot params and entry point are consistent. */
    if (KernelSize > largest_size) {
        Print(L"Kernel size %u exceeds selected Coreboot region of %u bytes\n", KernelSize, largest_size);
        return EFI_BAD_BUFFER_SIZE;
    }

    VOID* kernel_target = (VOID*)(UINTN)kernel_base;
    CopyMem(kernel_target, KernelBuffer, (UINTN)KernelSize);
    /* Use the kernel target as the canonical kernel buffer/entry point from now on */
    KernelBuffer = kernel_target;

    // Validate boot parameters structure
    if (!ValidateBootParameters(boot_params)) {
        Print(L"Boot parameters validation failed\n");
        return EFI_INVALID_PARAMETER;
    }

    Print(L"Boot parameters set up at 0x%llx\n", boot_params_addr);
    Print(L"Framebuffer: %ux%u @ 0x%llx\n",
          boot_params->framebuffer_width,
          boot_params->framebuffer_height,
          boot_params->framebuffer_addr);

    // Exit boot services and jump to kernel
    Status = ExitBootServicesAndExecuteKernel(KernelBuffer, KernelSize, boot_params_addr);

    return Status;
}

/**
 * Execute kernel using UEFI services
 */
STATIC
EFI_STATUS
ExecuteKernelWithUefi (
  IN VOID* KernelBuffer,
  IN UINTN KernelSize,
  IN VOID* InitrdBuffer OPTIONAL
  )
{
    EFI_STATUS Status;

    // Get UEFI memory map for kernel setup
    UINTN MapSize = 0, MapKey = 0, DescSize = 0;
    UINT32 DescVer = 0;
    EFI_MEMORY_DESCRIPTOR* MemMap = NULL;

    Status = gBS->GetMemoryMap(&MapSize, MemMap, &MapKey, &DescSize, &DescVer);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        MemMap = AllocatePool(MapSize);
        if (!MemMap) {
            return EFI_OUT_OF_RESOURCES;
        }
        Status = gBS->GetMemoryMap(&MapSize, MemMap, &MapKey, &DescSize, &DescVer);
    }

    if (EFI_ERROR(Status)) {
        Print(L"Failed to get UEFI memory map\n");
        return Status;
    }

    // Find suitable memory region for kernel
    EFI_PHYSICAL_ADDRESS KernelBase = 0x100000; // Default 1MB

    // Look for conventional memory above 1MB
    for (UINTN i = 0; i < MapSize / DescSize; i++) {
        EFI_MEMORY_DESCRIPTOR* Desc = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MemMap + i * DescSize);
        if (Desc->Type == EfiConventionalMemory &&
            Desc->PhysicalStart >= 0x100000 &&
            Desc->NumberOfPages * EFI_PAGE_SIZE >= KernelSize) {
            KernelBase = Desc->PhysicalStart;
            break;
        }
    }

    // Configure GOP framebuffer if available
    EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop = NULL;
    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&Gop);
    if (!EFI_ERROR(Status) && Gop && Gop->Mode) {
        Print(L"Configuring UEFI GOP framebuffer for kernel\n");
        Print(L"GOP Framebuffer: 0x%llx, %ux%u, %u bpp\n",
              Gop->Mode->FrameBufferBase,
              Gop->Mode->Info->HorizontalResolution,
              Gop->Mode->Info->VerticalResolution,
              Gop->Mode->Info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor ? 32 : 24);
    }

    // Set up boot parameters
    EFI_PHYSICAL_ADDRESS BootParamsAddr = KernelBase + 0x1000;

    // Exit boot services and execute kernel
    Status = ExitBootServicesAndExecuteKernel(KernelBuffer, KernelSize, BootParamsAddr);

    if (MemMap) {
        FreePool(MemMap);
    }

    return Status;
}

/**
 * Exit boot services and execute kernel
 */
STATIC
EFI_STATUS
ExitBootServicesAndExecuteKernel (
  IN VOID* KernelBuffer,
  IN UINTN KernelSize,
  IN EFI_PHYSICAL_ADDRESS BootParamsAddr
  )
{
    EFI_STATUS Status;
    UINTN MapSize = 0, MapKey = 0, DescSize = 0;
    UINT32 DescVer = 0;
    EFI_MEMORY_DESCRIPTOR* MemMap = NULL;

    // Robustly get the memory map and exit boot services (handle concurrent map updates)
    Status = EFI_SUCCESS;
    const int max_retries = 8;
    int attempt = 0;
    while (attempt++ < max_retries) {
        MapSize = 0;
        Status = gBS->GetMemoryMap(&MapSize, MemMap, &MapKey, &DescSize, &DescVer);
        if (Status == EFI_BUFFER_TOO_SMALL) {
            if (MemMap) { FreePool(MemMap); MemMap = NULL; }
            MemMap = AllocatePool(MapSize);
            if (!MemMap) { Status = EFI_OUT_OF_RESOURCES; break; }
            Status = gBS->GetMemoryMap(&MapSize, MemMap, &MapKey, &DescSize, &DescVer);
        }

        if (EFI_ERROR(Status)) {
            CpuPause();
            continue;
        }

        Status = gBS->ExitBootServices(gImageHandle, MapKey);
        if (Status == EFI_INVALID_PARAMETER) {
            /* Map changed between GetMemoryMap and ExitBootServices; retry */
            CpuPause();
            continue;
        }

        /* Either succeeded or failed with a different error */
        break;
    }

    if (EFI_ERROR(Status)) {
        Print(L"Failed to exit boot services: %r\n", Status);
        if (MemMap) FreePool(MemMap);
        return Status;
    }

    // Set up boot parameters at specified address
    // Set up proper boot parameter structure that kernel can access
    COREBOOT_BOOT_PARAMS* boot_params = (COREBOOT_BOOT_PARAMS*)BootParamsAddr;

    // Zero out the boot parameters structure
    SetMem(boot_params, sizeof(COREBOOT_BOOT_PARAMS), 0);

    // Set up basic boot parameters
    boot_params->signature = COREBOOT_BOOT_SIGNATURE;
    boot_params->version = 1;
    boot_params->kernel_base = (UINT64)(UINTN)KernelBuffer;
    boot_params->kernel_size = KernelSize;
    boot_params->boot_flags = COREBOOT_BOOT_FLAG_KERNEL;

    // Set up framebuffer information if available (Coreboot or UEFI)
    if (gCorebootAvailable) {
        CONST COREBOOT_FB* framebuffer = CorebootGetFramebuffer();
        if (framebuffer) {
            boot_params->framebuffer_addr = framebuffer->physical_address;
            boot_params->framebuffer_width = framebuffer->x_resolution;
            boot_params->framebuffer_height = framebuffer->y_resolution;
            boot_params->framebuffer_bpp = framebuffer->bits_per_pixel;
            boot_params->framebuffer_pitch = framebuffer->bytes_per_line;
            boot_params->boot_flags |= COREBOOT_BOOT_FLAG_FRAMEBUFFER;
        }
    } else {
        EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop = NULL;
        Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&Gop);
        if (!EFI_ERROR(Status) && Gop && Gop->Mode) {
            boot_params->framebuffer_addr = Gop->Mode->FrameBufferBase;
            boot_params->framebuffer_width = Gop->Mode->Info->HorizontalResolution;
            boot_params->framebuffer_height = Gop->Mode->Info->VerticalResolution;
            boot_params->framebuffer_bpp = Gop->Mode->Info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor ? 32 : 24;
            boot_params->framebuffer_pitch = Gop->Mode->Info->PixelsPerScanLine * (boot_params->framebuffer_bpp / 8);
            boot_params->boot_flags |= COREBOOT_BOOT_FLAG_FRAMEBUFFER;
        }
    }

    // Set up memory information
    if (gCorebootAvailable) {
        UINT64 total_memory = CorebootGetTotalMemory();
        boot_params->memory_size = total_memory;
    } else {
        // Calculate total memory from UEFI memory map
        UINT64 total_memory = 0;
        for (UINTN i = 0; i < MapSize / DescSize; i++) {
            EFI_MEMORY_DESCRIPTOR* Desc = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MemMap + i * DescSize);
            if (Desc->Type == EfiConventionalMemory || Desc->Type == EfiLoaderCode ||
                Desc->Type == EfiLoaderData || Desc->Type == EfiBootServicesCode ||
                Desc->Type == EfiBootServicesData) {
                total_memory += Desc->NumberOfPages * EFI_PAGE_SIZE;
            }
        }
        boot_params->memory_size = total_memory;
    }

    // Set up initrd if provided
    // Note: InitrdBuffer parameter is not used in this implementation
    // but would be set up here if needed

    // Validate boot parameters
    if (!ValidateBootParameters(boot_params)) {
        Print(L"Boot parameters validation failed\n");
        if (MemMap) FreePool(MemMap);
        return EFI_INVALID_PARAMETER;
    }

    Print(L"Boot parameters validated successfully\n");
    Print(L"Framebuffer: %ux%u @ 0x%llx\n",
          boot_params->framebuffer_width,
          boot_params->framebuffer_height,
          boot_params->framebuffer_addr);

    // Jump to kernel entry point
    typedef VOID (*KernelEntry)(COREBOOT_BOOT_PARAMS*);
    KernelEntry EntryPoint = (KernelEntry)KernelBuffer;

    Print(L"Jumping to kernel entry point at 0x%llx\n", (UINT64)(UINTN)KernelBuffer);

    // Execute kernel (this should not return)
    EntryPoint(boot_params);

    // If we get here, kernel execution failed
    if (MemMap) FreePool(MemMap);
    return EFI_LOAD_ERROR;
}

/**
 * Validate boot parameters structure
 */
STATIC
BOOLEAN
ValidateBootParameters (
  IN COREBOOT_BOOT_PARAMS* boot_params
  )
{
    if (!boot_params) {
        return FALSE;
    }

    // Check signature
    if (boot_params->signature != COREBOOT_BOOT_SIGNATURE) {
        Print(L"Invalid boot parameters signature: 0x%x\n", boot_params->signature);
        return FALSE;
    }

    // Check version
    if (boot_params->version != 1) {
        Print(L"Unsupported boot parameters version: %d\n", boot_params->version);
        return FALSE;
    }

    // Check kernel information
    if (boot_params->kernel_base == 0 || boot_params->kernel_size == 0) {
        Print(L"Invalid kernel base or size\n");
        return FALSE;
    }

    // Check boot flags
    if ((boot_params->boot_flags & COREBOOT_BOOT_FLAG_KERNEL) == 0) {
        Print(L"Kernel flag not set in boot parameters\n");
        return FALSE;
    }

    // Validate framebuffer information if present
    if (boot_params->boot_flags & COREBOOT_BOOT_FLAG_FRAMEBUFFER) {
        if (boot_params->framebuffer_addr == 0 ||
            boot_params->framebuffer_width == 0 ||
            boot_params->framebuffer_height == 0) {
            Print(L"Invalid framebuffer information\n");
            return FALSE;
        }
    }

    // Validate initrd information if present
    if (boot_params->boot_flags & COREBOOT_BOOT_FLAG_INITRD) {
        if (boot_params->initrd_addr == 0) {
            Print(L"Invalid initrd address\n");
            return FALSE;
        }
    }

    return TRUE;
}

/**
 * Check if running as Coreboot payload and handle entry point
 */
STATIC
EFI_STATUS
HandleCorebootPayloadEntry (
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
    // Check if we have Coreboot table available (indicates Coreboot payload)
    if (CorebootPlatformInit()) {
        Print(L"Running as Coreboot payload - using dedicated entry point\n");

        // Set flag for hybrid mode detection
        gRunningAsCorebootPayload = TRUE;
        gCorebootAvailable = TRUE;

        // Call Coreboot main entry point
        CorebootMain(NULL, NULL);

        // Should not return, but handle gracefully if it does
        return EFI_SUCCESS;
    }

    return EFI_NOT_FOUND; // Not running as Coreboot payload
}

/**
 * Get root directory handle for filesystem operations
 */
EFI_STATUS
EFIAPI
get_root_dir (
  OUT EFI_FILE_HANDLE* root_dir
  )
{
    EFI_STATUS Status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* Fs = NULL;
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage = NULL;

    // Get the loaded image protocol to find our device
    Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&LoadedImage);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get loaded image protocol: %r\n", Status);
        return Status;
    }

    // Get the file system protocol for the device
    Status = gBS->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&Fs);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get file system protocol: %r\n", Status);
        return Status;
    }

    // Open the root directory
    Status = Fs->OpenVolume(Fs, root_dir);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open root volume: %r\n", Status);
        return Status;
    }

    return EFI_SUCCESS;
}
