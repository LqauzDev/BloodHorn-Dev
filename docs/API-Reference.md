# BloodHorn API Reference

This document provides API reference documentation for BloodHorn bootloader developers.

## Table of Contents

1. [Core Bootloader APIs](#core-bootloader-apis)
2. [Security APIs](#security-apis)
3. [Filesystem APIs](#filesystem-apis)
4. [Graphics and UI APIs](#graphics-and-ui-apis)
5. [Network APIs](#network-apis)
6. [Configuration APIs](#configuration-apis)
7. [Architecture-Specific APIs](#architecture-specific-apis)
8. [Error Codes](#error-codes)
9. [Data Structures](#data-structures)

## Core Bootloader APIs

### Boot Manager Protocol

```c
/**
 * Add a boot entry to the boot menu
 * 
 * @param Name Display name for the boot entry (UCS-2 string)
 * @param BootFunction Function to call when this entry is selected
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI AddBootEntry(
    IN CONST CHAR16 *Name,
    IN EFI_STATUS (*BootFunction)(VOID)
);

/**
 * Display the boot menu and handle user selection
 * 
 * @return EFI_SUCCESS if boot succeeded, error code otherwise
 */
EFI_STATUS EFIAPI ShowBootMenu(VOID);

/**
 * Print text at specific coordinates with colors
 * 
 * @param X X coordinate
 * @param Y Y coordinate
 * @param FgColor Foreground color (ARGB format)
 * @param BgColor Background color (ARGB format)
 * @param Format Format string (UCS-2)
 * @param ... Variable arguments
 */
VOID EFIAPI PrintXY(
    IN UINTN X,
    IN UINTN Y,
    IN UINT32 FgColor,
    IN UINT32 BgColor,
    IN CONST CHAR16 *Format,
    ...
);
```

### Kernel Loading APIs

```c
/**
 * Load and verify a kernel image with optional secure boot verification
 * 
 * @param FileName Path to the kernel file (UCS-2 string)
 * @param ImageBuffer Output buffer containing loaded kernel
 * @param ImageSize Output size of loaded kernel
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI LoadAndVerifyKernel(
    IN  CHAR16  *FileName,
    OUT VOID    **ImageBuffer,
    OUT UINTN   *ImageSize
);

/**
 * Execute a loaded kernel image using UEFI boot services
 * 
 * @param ImageBuffer Pointer to loaded kernel image
 * @param ImageSize Size of kernel image
 * @param CmdLine Optional kernel command line (UCS-2 string)
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI ExecuteKernel(
    IN VOID     *ImageBuffer,
    IN UINTN    ImageSize,
    IN CHAR16   *CmdLine OPTIONAL
);

/**
 * Verify image signature using RSA PKCS#1 v1.5
 * 
 * @param ImageBuffer Buffer containing image and signature
 * @param ImageSize Size of image including signature
 * @param PublicKey Public key for verification
 * @param PublicKeySize Size of public key
 * @return EFI_SUCCESS if signature valid, error otherwise
 */
EFI_STATUS EFIAPI VerifyImageSignature(
    IN CONST VOID    *ImageBuffer,
    IN UINTN         ImageSize,
    IN CONST UINT8   *PublicKey,
    IN UINTN         PublicKeySize
);

/**
 * Check if UEFI Secure Boot is enabled
 * 
 * @return TRUE if Secure Boot is enabled, FALSE otherwise
 */
BOOLEAN EFIAPI IsSecureBootEnabled(VOID);
```

### Boot Wrapper Functions

```c
/**
 * Boot Linux kernel using hardcoded paths
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI BootLinuxKernelWrapper(VOID);

/**
 * Boot Multiboot2 kernel using hardcoded paths
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI BootMultiboot2KernelWrapper(VOID);

/**
 * Boot Limine kernel using hardcoded paths
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI BootLimineKernelWrapper(VOID);

/**
 * Chainload another bootloader
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI BootChainloadWrapper(VOID);

/**
 * Boot via PXE network
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI BootPxeNetworkWrapper(VOID);

/**
 * Start recovery shell
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI BootRecoveryShellWrapper(VOID);

/**
 * Boot UEFI shell if available
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI BootUefiShellWrapper(VOID);

/**
 * Exit to firmware settings
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI ExitToFirmwareWrapper(VOID);

// Architecture-specific wrappers
EFI_STATUS EFIAPI BootIa32Wrapper(VOID);
EFI_STATUS EFIAPI BootX86_64Wrapper(VOID);
EFI_STATUS EFIAPI BootAarch64Wrapper(VOID);
EFI_STATUS EFIAPI BootRiscv64Wrapper(VOID);
EFI_STATUS EFIAPI BootLoongarch64Wrapper(VOID);

/**
 * Boot using BloodChain protocol
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI BootBloodchainWrapper(VOID);
```

### Security APIs

#### UEFI Secure Boot Integration

```c
/**
 * Check if UEFI Secure Boot is enabled
 * 
 * @return TRUE if Secure Boot is enabled, FALSE otherwise
 */
BOOLEAN EFIAPI IsSecureBootEnabled(VOID);

/**
 * Verify image signature using RSA PKCS#1 v1.5
 * 
 * Assumes signature is appended to image: [image][signature]
 * Uses SHA-256 for hashing and RSA 2048 for verification
 * 
 * @param ImageBuffer Buffer containing image and signature
 * @param ImageSize Size of image including signature
 * @param PublicKey Public key for verification
 * @param PublicKeySize Size of public key
 * @return EFI_SUCCESS if signature valid, EFI_SECURITY_VIOLATION otherwise
 */
EFI_STATUS EFIAPI VerifyImageSignature(
    IN CONST VOID    *ImageBuffer,
    IN UINTN         ImageSize,
    IN CONST UINT8   *PublicKey,
    IN UINTN         PublicKeySize
);

/**
 * Load and verify kernel with optional secure boot verification
 * 
 * If Secure Boot is enabled, verifies signature using PK variable
 * 
 * @param FileName Path to kernel file (UCS-2 string)
 * @param ImageBuffer Output buffer containing loaded kernel
 * @param ImageSize Output size of loaded kernel
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI LoadAndVerifyKernel(
    IN  CHAR16  *FileName,
    OUT VOID    **ImageBuffer,
    OUT UINTN   *ImageSize
);

/**
 * Execute loaded kernel using UEFI boot services
 * 
 * @param ImageBuffer Pointer to loaded kernel image
 * @param ImageSize Size of kernel image
 * @param CmdLine Optional kernel command line (UCS-2 string)
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI ExecuteKernel(
    IN VOID     *ImageBuffer,
    IN UINTN    ImageSize,
    IN CHAR16   *CmdLine OPTIONAL
);
```

#### Cryptographic Operations

BloodHorn uses EDK2's BaseCryptLib for cryptographic operations:

- **SHA-256**: `Sha256HashAll()` for image hashing
- **RSA PKCS#1 v1.5**: `RsaPkcs1Verify()` for signature verification
- **Random Numbers**: Available via UEFI RNG protocol

**Note**: BloodHorn does not implement custom cryptographic functions but relies on EDK2's verified implementations.

## Filesystem APIs

### File Operations

```c
/**
 * Read file from filesystem into allocated buffer
 * 
 * @param FileName Path to file (UCS-2 string)
 * @param Buffer Output buffer pointer (allocated by function)
 * @param Size Output file size
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS ReadFile(
    IN CHAR16* FileName,
    OUT VOID** Buffer,
    OUT UINTN* Size
);

/**
 * Get root directory of boot device
 * 
 * @param root_dir Output root directory handle
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS get_root_dir(
    OUT EFI_FILE_HANDLE* root_dir
);
```

**Note**: BloodHorn currently uses basic UEFI file operations. Advanced filesystem support (FAT32, EXT2) is planned but not fully implemented in the current codebase.

## Graphics and UI APIs

### Graphics and UI APIs

```c
/**
 * Print text at specific coordinates with colors
 * 
 * @param X X coordinate
 * @param Y Y coordinate
 * @param FgColor Foreground color (ARGB format)
 * @param BgColor Background color (ARGB format)
 * @param Format Format string (UCS-2)
 * @param ... Variable arguments
 */
VOID EFIAPI PrintXY(
    IN UINTN X,
    IN UINTN Y,
    IN UINT32 FgColor,
    IN UINT32 BgColor,
    IN CONST CHAR16 *Format,
    ...
);
```

**Current Implementation Status:**
- Basic text output via `PrintXY()` is implemented
- Font rendering system exists but is basic
- Mouse support is initialized but functionality is limited
- Advanced graphics (framebuffer manipulation) is planned

**Note**: The graphics system is currently minimal. Most advanced graphics features mentioned in the original documentation are not yet implemented.

## Network APIs

### Network APIs

**Current Implementation Status:**
- Network boot wrapper functions are defined but not implemented
- PXE and HTTP boot support is planned but not functional
- Basic network initialization exists in coreboot integration

```c
/**
 * Boot via PXE network (not implemented)
 * 
 * @return EFI_UNSUPPORTED - not yet implemented
 */
EFI_STATUS EFIAPI BootPxeNetworkWrapper(VOID);
```

**Note**: Network boot functionality is currently a placeholder. The infrastructure exists but the actual network stack implementation is incomplete.

## Configuration APIs

### Configuration APIs

```c
/**
 * Load boot configuration from INI/JSON files
 * 
 * @param config Output configuration structure
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS LoadBootConfig(BOOT_CONFIG* config);

/**
 * Apply INI configuration to config structure
 * 
 * @param ini INI file content as string
 * @param config Configuration structure to populate
 */
VOID ApplyIniToConfig(const CHAR8* ini, BOOT_CONFIG* config);

/**
 * Apply JSON configuration to config structure
 * 
 * @param js JSON file content as string
 * @param config Configuration structure to populate
 */
VOID ApplyJsonToConfig(const CHAR8* js, BOOT_CONFIG* config);

/**
 * Apply UEFI environment variable overrides
 * 
 * @param config Configuration structure to update
 */
VOID ApplyUefiEnvOverrides(BOOT_CONFIG* config);
```

**Configuration Structure:**
```c
typedef struct {
    CHAR8 default_entry[64];        // Default boot entry name
    INT32 menu_timeout;             // Menu timeout in seconds
    CHAR8 kernel[128];              // Default kernel path
    CHAR8 initrd[128];              // Default initrd path
    CHAR8 cmdline[256];             // Kernel command line
    BOOLEAN tpm_enabled;            // TPM 2.0 enabled (not implemented)
    BOOLEAN secure_boot;            // Secure boot enabled
    BOOLEAN use_gui;                // Use graphical interface
    CHAR8 font_path[256];           // Custom font file path
    UINT32 font_size;               // Font size in pixels
    UINT32 header_font_size;        // Header font size
    CHAR8 language[8];              // Language code (e.g., "en")
    BOOLEAN enable_networking;      // Enable network boot
} BOOT_CONFIG;
```

## Architecture-Specific APIs

### Architecture-Specific APIs

**Current Implementation:**
BloodHorn provides architecture-specific boot wrappers, but most use hardcoded paths and return placeholder status codes.

```c
/**
 * Boot IA-32 (32-bit x86) kernel
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI BootIa32Wrapper(VOID);

/**
 * Boot x86-64 (64-bit x86) kernel
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI BootX86_64Wrapper(VOID);

/**
 * Boot ARM64 kernel
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI BootAarch64Wrapper(VOID);

/**
 * Boot RISC-V 64 kernel
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI BootRiscv64Wrapper(VOID);

/**
 * Boot LoongArch 64 kernel
 * 
 * @return EFI_SUCCESS on success, error code on failure
 */
EFI_STATUS EFIAPI BootLoongarch64Wrapper(VOID);
```

**Note**: These wrappers currently use hardcoded paths like `/boot/vmlinuz-x86_64` and may not be fully functional. The architecture support is structural but implementation is incomplete.

## Error Codes

### Common EFI Status Codes

| Status Code | Value | Description |
|-------------|-------|-------------|
| EFI_SUCCESS | 0x00000000 | Operation completed successfully |
| EFI_LOAD_ERROR | 0x80000001 | Image failed to load |
| EFI_INVALID_PARAMETER | 0x80000002 | Invalid parameter |
| EFI_UNSUPPORTED | 0x80000003 | Operation not supported |
| EFI_BAD_BUFFER_SIZE | 0x80000004 | Buffer too small |
| EFI_BUFFER_TOO_SMALL | 0x80000005 | Buffer too small |
| EFI_NOT_READY | 0x80000006 | Not ready |
| EFI_DEVICE_ERROR | 0x80000007 | Device error |
| EFI_WRITE_PROTECTED | 0x80000008 | Write protected |
| EFI_OUT_OF_RESOURCES | 0x80000009 | Out of resources |
| EFI_VOLUME_CORRUPTED | 0x8000000A | Volume corrupted |
| EFI_VOLUME_FULL | 0x8000000B | Volume full |
| EFI_NO_MEDIA | 0x8000000C | No media |
| EFI_MEDIA_CHANGED | 0x8000000D | Media changed |
| EFI_NOT_FOUND | 0x8000000E | Not found |
| EFI_ACCESS_DENIED | 0x8000000F | Access denied |
| EFI_NO_RESPONSE | 0x80000010 | No response |
| EFI_NO_MAPPING | 0x80000011 | No mapping |
| EFI_TIMEOUT | 0x80000012 | Timeout |
| EFI_NOT_STARTED | 0x80000013 | Not started |
| EFI_ALREADY_STARTED | 0x80000014 | Already started |
| EFI_ABORTED | 0x80000015 | Aborted |

### BloodHorn-Specific Error Codes

| Status Code | Value | Description |
|-------------|-------|-------------|
| BH_SUCCESS | 0x00000000 | BloodHorn operation successful |
| BH_INVALID_PARAMETER | 0x80000001 | Invalid BloodHorn parameter |
| BH_DEVICE_ERROR | 0x80000002 | BloodHorn device error |
| BH_NOT_FOUND | 0x80000003 | BloodHorn resource not found |
| BH_SECURITY_ERROR | 0x80000004 | Security verification failed |
| BH_CRYPTO_ERROR | 0x80000005 | Cryptographic operation failed |
| BH_TPM_ERROR | 0x80000006 | TPM operation failed |

## Data Structures

### Boot Configuration Structure

```c
typedef struct {
    CHAR8 default_entry[64];        // Default boot entry name
    INT32 menu_timeout;             // Menu timeout in seconds
    CHAR8 kernel[128];              // Default kernel path
    CHAR8 initrd[128];              // Default initrd path
    CHAR8 cmdline[256];             // Kernel command line
    BOOLEAN tpm_enabled;            // TPM 2.0 enabled
    BOOLEAN secure_boot;            // Secure boot enabled
    BOOLEAN use_gui;                // Use graphical interface
    CHAR8 font_path[256];           // Custom font file path
    UINT32 font_size;               // Font size in pixels
    UINT32 header_font_size;        // Header font size
    CHAR8 language[8];              // Language code (e.g., "en")
    BOOLEAN enable_networking;      // Enable network boot
} BOOT_CONFIG;
```

### Coreboot Boot Parameters

```c
typedef struct {
    UINT32 signature;               // Boot signature (0x12345678)
    UINT32 version;                 // Structure version
    UINT64 kernel_base;             // Kernel load address
    UINT64 kernel_size;             // Kernel size
    UINT32 boot_flags;              // Boot flags
    UINT64 framebuffer_addr;        // Framebuffer address
    UINT32 framebuffer_width;       // Framebuffer width
    UINT32 framebuffer_height;      // Framebuffer height
    UINT32 framebuffer_bpp;         // Bits per pixel
    UINT32 framebuffer_pitch;       // Bytes per scanline
    UINT64 memory_size;             // Total memory size
    UINT64 initrd_addr;             // Initrd address
    UINT64 initrd_size;             // Initrd size
    CHAR8 cmdline[256];             // Kernel command line
} COREBOOT_BOOT_PARAMS;
```

### File Hash Structure

```c
typedef struct {
    CHAR8 path[256];                // File path
    UINT8 expected_hash[64];        // SHA-512 hash
} FILE_HASH;
```

## Usage Examples

### Basic Kernel Loading

```c
EFI_STATUS BootLinuxKernel(VOID) {
    EFI_STATUS Status;
    VOID* ImageBuffer = NULL;
    UINTN ImageSize = 0;
    
    // Load and verify kernel
    Status = LoadAndVerifyKernel(L"vmlinuz", &ImageBuffer, &ImageSize);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to load kernel: %r\n", Status);
        return Status;
    }
    
    // Execute kernel with command line
    Status = ExecuteKernel(ImageBuffer, ImageSize, L"root=/dev/sda1 ro");
    if (EFI_ERROR(Status)) {
        Print(L"Failed to execute kernel: %r\n", Status);
        FreePool(ImageBuffer);
        return Status;
    }
    
    return EFI_SUCCESS;
}
```

### Adding Boot Menu Entries

```c
EFI_STATUS SetupBootMenu(VOID) {
    EFI_STATUS Status;
    
    // Add boot entries
    Status = AddBootEntry(L"Linux Kernel", BootLinuxKernelWrapper);
    if (EFI_ERROR(Status)) return Status;
    
    Status = AddBootEntry(L"Multiboot2 Kernel", BootMultiboot2KernelWrapper);
    if (EFI_ERROR(Status)) return Status;
    
    Status = AddBootEntry(L"Recovery Shell", BootRecoveryShellWrapper);
    if (EFI_ERROR(Status)) return Status;
    
    // Show boot menu
    return ShowBootMenu();
}
```

### Secure Boot with Verification

```c
EFI_STATUS SecureBootExample(VOID) {
    EFI_STATUS Status;
    VOID* ImageBuffer = NULL;
    UINTN ImageSize = 0;
    
    // Check if Secure Boot is enabled
    if (IsSecureBootEnabled()) {
        Print(L"Secure Boot is enabled - verifying kernel signature\n");
    } else {
        Print(L"Secure Boot is disabled\n");
    }
    
    // Load and verify kernel (automatic if Secure Boot enabled)
    Status = LoadAndVerifyKernel(L"kernel.efi", &ImageBuffer, &ImageSize);
    if (EFI_ERROR(Status)) {
        Print(L"Kernel verification failed: %r\n", Status);
        return Status;
    }
    
    // Execute kernel
    Status = ExecuteKernel(ImageBuffer, ImageSize, NULL);
    if (EFI_ERROR(Status)) {
        FreePool(ImageBuffer);
        return Status;
    }
    
    return EFI_SUCCESS;
}
```

## Thread Safety

BloodHorn runs in a single-threaded UEFI environment. Most APIs are **not thread-safe** as UEFI boot services are not designed for concurrent access.

**Guidelines:**
- Always check return values from UEFI function calls
- Use proper memory allocation/deallocation patterns
- Avoid re-entrancy issues in callback functions
- Handle protocol installation and location carefully
- Do not assume thread safety for any global state

## Memory Management

- Use `AllocatePool()` and `FreePool()` for dynamic memory
- Always check for allocation failures
- Free memory before calling `ExitBootServices()`
- Use `SecureZeroMemory()` for sensitive data (not implemented in current codebase)
- Avoid memory leaks in error paths

**Current Implementation:**
The codebase uses standard EDK2 memory management. No custom memory pools or advanced memory management is currently implemented.

## Security Considerations

- Always verify kernel signatures when Secure Boot is enabled
- Use constant-time comparisons for security-critical data (not implemented)
- Zero out sensitive memory after use (not implemented)
- Validate all input parameters
- Implement proper error handling without information leakage

**Current Security Features:**
- UEFI Secure Boot integration using PK variable
- RSA signature verification for kernel images
- SHA-256 hashing for integrity verification

**Missing Security Features:**
- TPM 2.0 integration (placeholders exist)
- Constant-time comparisons
- Secure memory zeroization
- Advanced threat protection

This API reference provides comprehensive documentation for BloodHorn bootloader development. For more specific implementation details, see the individual module documentation and source code comments.
