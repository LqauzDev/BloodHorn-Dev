/*
 * BootManagerProtocol.h
 *
 * Enhanced Boot Manager Protocol for BloodHorn Bootloader
 * 
 * This protocol provides comprehensive boot entry management with support for
 * multiple boot protocols, secure boot verification, and advanced configuration.
 * 
 * Copyright (c) 2025 BloodyHell Industries INC
 * Licensed under BSD-2-Clause-Patent License
 */

#ifndef _BOOT_MANAGER_PROTOCOL_H_
#define _BOOT_MANAGER_PROTOCOL_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>

// =============================================================================
// CONSTANTS AND DEFINITIONS
// =============================================================================

#define BOOT_MANAGER_PROTOCOL_VERSION    0x00010002
#define BOOT_MANAGER_MAX_ENTRIES        64
#define BOOT_MANAGER_MAX_CMDLINE_LENGTH 1024
#define BOOT_MANAGER_MAX_PATH_LENGTH    512
#define BOOT_MANAGER_MAX_NAME_LENGTH    128
#define BOOT_MANAGER_MAX_DESC_LENGTH    256

// Boot entry types
#define BOOT_ENTRY_TYPE_UNKNOWN              0x00
#define BOOT_ENTRY_TYPE_MULTIBOOT1           0x01
#define BOOT_ENTRY_TYPE_MULTIBOOT2           0x02
#define BOOT_ENTRY_TYPE_LIMINE               0x03
#define BOOT_ENTRY_TYPE_LINUX                0x04
#define BOOT_ENTRY_TYPE_CHAINLOAD             0x05
#define BOOT_ENTRY_TYPE_UEFI_APPLICATION      0x06
#define BOOT_ENTRY_TYPE_RECOVERY             0x07
#define BOOT_ENTRY_TYPE_FIRMWARE_SETTINGS    0x08
#define BOOT_ENTRY_TYPE_BLOODCHAIN           0x09
#define BOOT_ENTRY_TYPE_WINDOWS_BOOT_MGR     0x0A
#define BOOT_ENTRY_TYPE_COREBOOT_PAYLOAD     0x0B
#define BOOT_ENTRY_TYPE_CUSTOM               0xFF

// Boot environments
#define BOOT_ENVIRONMENT_AUTO               0x00
#define BOOT_ENVIRONMENT_UEFI               0x01
#define BOOT_ENVIRONMENT_COREBOOT           0x02
#define BOOT_ENVIRONMENT_LEGACY_BIOS        0x03
#define BOOT_ENVIRONMENT_WINDOWS_BOOT_MGR   0x04

// Boot entry flags
#define BOOT_ENTRY_FLAG_ACTIVE              0x01
#define BOOT_ENTRY_FLAG_DEFAULT             0x02
#define BOOT_ENTRY_FLAG_HIDDEN              0x04
#define BOOT_ENTRY_FLAG_SECURE_BOOT         0x08
#define BOOT_ENTRY_FLAG_VERIFIED            0x10
#define BOOT_ENTRY_FLAG_ONCE               0x20
#define BOOT_ENTRY_FLAG_RECOVERY            0x40
#define BOOT_ENTRY_FLAG_SYSTEM              0x80

// Boot manager flags
#define BOOT_MANAGER_FLAG_SECURE_BOOT       0x01
#define BOOT_MANAGER_FLAG_TPM_AVAILABLE    0x02
#define BOOT_MANAGER_FLAG_MEASURED_BOOT     0x04
#define BOOT_MANAGER_FLAG_NETWORK_BOOT      0x08
#define BOOT_MANAGER_FLAG_RECOVERY_MODE     0x10

// =============================================================================
// DATA STRUCTURES
// =============================================================================

/**
 * Boot Manager Entry Structure
 */
typedef struct {
    UINT32      EntryId;                                    // Unique entry identifier
    UINT8       EntryType;                                   // Boot entry type
    UINT8       BootEnvironment;                             // Target boot environment
    UINT8       Flags;                                       // Entry flags
    UINT8       Reserved;                                    // Reserved for future use
    CHAR16      Name[BOOT_MANAGER_MAX_NAME_LENGTH];          // Display name
    CHAR16      Description[BOOT_MANAGER_MAX_DESC_LENGTH];    // Description
    CHAR16      DevicePath[BOOT_MANAGER_MAX_PATH_LENGTH];    // Device path
    CHAR16      KernelPath[BOOT_MANAGER_MAX_PATH_LENGTH];    // Kernel image path
    CHAR16      InitrdPath[BOOT_MANAGER_MAX_PATH_LENGTH];    // Initrd path
    CHAR16      CommandLine[BOOT_MANAGER_MAX_CMDLINE_LENGTH]; // Command line
    CHAR16      IconPath[BOOT_MANAGER_MAX_PATH_LENGTH];      // Icon path
    UINT8*      KernelData;                                  // Kernel data buffer
    UINTN       KernelSize;                                   // Kernel data size
    UINT8*      InitrdData;                                  // Initrd data buffer
    UINTN       InitrdSize;                                   // Initrd data size
    EFI_GUID    VendorGuid;                                   // Vendor-specific GUID
    UINT32      Timeout;                                      // Boot timeout (seconds)
    UINT64      LoadAddress;                                   // Load address
    UINT64      EntryAddress;                                  // Entry address
    UINT32      Attributes;                                   // Memory attributes
    UINT32      Reserved2[3];                                 // Reserved for future use
} BOOT_MANAGER_ENTRY;

/**
 * Boot Manager Statistics
 */
typedef struct {
    UINT32      TotalEntries;                                 // Total boot entries
    UINT32      ActiveEntries;                                // Active boot entries
    UINT32      SecureBootEntries;                             // Secure boot entries
    UINT32      VerifiedEntries;                               // Cryptographically verified entries
    UINT32      FailedBoots;                                   // Failed boot attempts
    UINT32      SuccessfulBoots;                               // Successful boot attempts
    UINT64      TotalBootTime;                                 // Total boot time (microseconds)
    UINT64      AverageBootTime;                               // Average boot time (microseconds)
    UINT32      LastBootEntryId;                              // Last booted entry ID
    EFI_STATUS  LastBootStatus;                               // Last boot status
    UINT32      Reserved[8];                                   // Reserved for future use
} BOOT_MANAGER_STATISTICS;

/**
 * Boot Manager Configuration
 */
typedef struct {
    UINT32      Version;                                      // Configuration version
    UINT32      DefaultTimeout;                                // Default timeout (seconds)
    UINT8       DefaultBootEnvironment;                        // Default boot environment
    UINT8       Flags;                                        // Boot manager flags
    UINT8       SecureBootPolicy;                              // Secure boot policy
    UINT8       Reserved;                                      // Reserved for future use
    CHAR16      DefaultIconPath[BOOT_MANAGER_MAX_PATH_LENGTH]; // Default icon path
    CHAR16      ThemePath[BOOT_MANAGER_MAX_PATH_LENGTH];      // Theme path
    CHAR16      ConfigPath[BOOT_MANAGER_MAX_PATH_LENGTH];      // Configuration path
    EFI_GUID    SystemGuid;                                   // System identifier
    UINT32      Reserved2[8];                                 // Reserved for future use
} BOOT_MANAGER_CONFIGURATION;

/**
 * Boot Manager Protocol Structure
 */
typedef struct _BOOT_MANAGER_PROTOCOL {
    UINT64      Version;                                      // Protocol version
    UINT32      Flags;                                        // Boot manager flags
    UINT32      Reserved;                                      // Reserved for future use
    
    // Protocol interface functions
    EFI_STATUS (EFIAPI *AddEntry) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN CONST BOOT_MANAGER_ENTRY *Entry
        );
    
    EFI_STATUS (EFIAPI *RemoveEntry) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN UINT32 EntryId
        );
    
    EFI_STATUS (EFIAPI *UpdateEntry) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN UINT32 EntryId,
        IN CONST BOOT_MANAGER_ENTRY *Entry
        );
    
    EFI_STATUS (EFIAPI *GetEntry) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN UINT32 EntryId,
        OUT BOOT_MANAGER_ENTRY *Entry
        );
    
    EFI_STATUS (EFIAPI *GetEntries) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        OUT BOOT_MANAGER_ENTRY *Entries,
        OUT UINTN *EntryCount
        );
    
    EFI_STATUS (EFIAPI *SetBootOrder) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN UINT32 *EntryOrder,
        IN UINTN EntryCount
        );
    
    EFI_STATUS (EFIAPI *GetBootOrder) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        OUT UINT32 *EntryOrder,
        OUT UINTN *EntryCount
        );
    
    EFI_STATUS (EFIAPI *SetDefaultEntry) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN UINT32 EntryId
        );
    
    EFI_STATUS (EFIAPI *GetDefaultEntry) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        OUT UINT32 *EntryId
        );
    
    EFI_STATUS (EFIAPI *BootEntry) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN UINT32 EntryId
        );
    
    EFI_STATUS (EFIAPI *BootEntryWithTimeout) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN UINT32 EntryId,
        IN UINTN TimeoutSeconds
        );
    
    EFI_STATUS (EFIAPI *BootEntryWithEnvironment) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN UINT32 EntryId,
        IN UINT8 BootEnvironment
        );
    
    EFI_STATUS (EFIAPI *VerifyEntry) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN UINT32 EntryId,
        OUT BOOLEAN *Verified
        );
    
    EFI_STATUS (EFIAPI *MeasureEntry) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN UINT32 EntryId,
        OUT UINT8 *Measurement,
        OUT UINTN *MeasurementSize
        );
    
    EFI_STATUS (EFIAPI *GetStatistics) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        OUT BOOT_MANAGER_STATISTICS *Statistics
        );
    
    EFI_STATUS (EFIAPI *GetConfiguration) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        OUT BOOT_MANAGER_CONFIGURATION *Configuration
        );
    
    EFI_STATUS (EFIAPI *SetConfiguration) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN CONST BOOT_MANAGER_CONFIGURATION *Configuration
        );
    
    EFI_STATUS (EFIAPI *SaveConfiguration) (
        IN struct _BOOT_MANAGER_PROTOCOL *This
        );
    
    EFI_STATUS (EFIAPI *LoadConfiguration) (
        IN struct _BOOT_MANAGER_PROTOCOL *This
        );
    
    EFI_STATUS (EFIAPI *ValidateConfiguration) (
        IN struct _BOOT_MANAGER_PROTOCOL *This
        );
    
    EFI_STATUS (EFIAPI *ResetStatistics) (
        IN struct _BOOT_MANAGER_PROTOCOL *This
        );
    
    EFI_STATUS (EFIAPI *GetSystemInformation) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        OUT CHAR16 *SystemInfo,
        OUT UINTN *InfoSize
        );
    
    EFI_STATUS (EFIAPI *ExportConfiguration) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN CHAR16 *ExportPath,
        IN UINT8 Format
        );
    
    EFI_STATUS (EFIAPI *ImportConfiguration) (
        IN struct _BOOT_MANAGER_PROTOCOL *This,
        IN CHAR16 *ImportPath,
        IN UINT8 Format
        );
    
    // Private data
    VOID*       PrivateData;
} BOOT_MANAGER_PROTOCOL;

// =============================================================================
// EXTERNAL VARIABLES
// =============================================================================

extern EFI_GUID gBootManagerProtocolGuid;
extern BOOLEAN gCorebootAvailable;

// =============================================================================
// FUNCTION PROTOTYPES
// =============================================================================

// Protocol installation and management
EFI_STATUS EFIAPI InstallBootManagerProtocol (
    IN EFI_HANDLE ImageHandle,
    IN BOOT_MANAGER_PROTOCOL **BootManagerProtocol
    );

EFI_STATUS EFIAPI UninstallBootManagerProtocol (
    IN EFI_HANDLE ImageHandle
    );

// Boot manager library functions
EFI_STATUS EFIAPI BootManagerLibInitialize (VOID);
EFI_STATUS EFIAPI BootManagerLibGetProtocol (OUT BOOT_MANAGER_PROTOCOL** Protocol);
EFI_STATUS EFIAPI BootManagerLibSetProtocol (IN BOOT_MANAGER_PROTOCOL* Protocol);

// Boot entry management
EFI_STATUS EFIAPI AddBootEntry (
    IN BOOT_MANAGER_PROTOCOL *This,
    IN CONST BOOT_MANAGER_ENTRY *Entry
    );

EFI_STATUS EFIAPI RemoveBootEntry (
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId
    );

EFI_STATUS EFIAPI UpdateBootEntry (
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId,
    IN CONST BOOT_MANAGER_ENTRY *Entry
    );

EFI_STATUS EFIAPI GetBootEntries (
    IN BOOT_MANAGER_PROTOCOL *This,
    OUT BOOT_MANAGER_ENTRY *Entries,
    OUT UINTN *EntryCount
    );

EFI_STATUS EFIAPI SetBootOrder (
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 *EntryOrder,
    IN UINTN EntryCount
    );

EFI_STATUS EFIAPI SetDefaultEntry (
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId
    );

// Boot execution
EFI_STATUS EFIAPI BootEntry (
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId
    );

EFI_STATUS EFIAPI BootEntryWithTimeout (
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId,
    IN UINTN TimeoutSeconds
    );

EFI_STATUS EFIAPI BootEntryWithEnvironment (
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId,
    IN UINT8 BootEnvironment
    );

// Configuration management
EFI_STATUS EFIAPI SaveConfiguration (
    IN BOOT_MANAGER_PROTOCOL *This
    );

EFI_STATUS EFIAPI LoadConfiguration (
    IN BOOT_MANAGER_PROTOCOL *This
    );

EFI_STATUS EFIAPI ValidateBootConfiguration (
    IN BOOT_MANAGER_PROTOCOL *This
    );

// Security and verification
EFI_STATUS EFIAPI VerifyBootEntry (
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId,
    OUT BOOLEAN *Verified
    );

EFI_STATUS EFIAPI MeasureBootEntry (
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId,
    OUT UINT8 *Measurement,
    OUT UINTN *MeasurementSize
    );

// System information
EFI_STATUS EFIAPI GetSystemInformation (
    IN BOOT_MANAGER_PROTOCOL *This,
    OUT CHAR16 *SystemInfo,
    OUT UINTN *InfoSize
    );

// Utility functions
EFI_STATUS EFIAPI LoadAndStartImageFromPath (
    IN EFI_HANDLE ParentImageHandle,
    IN CHAR16 *ImagePath,
    IN UINT8 *ImageData OPTIONAL,
    IN UINTN ImageSize OPTIONAL
    );

EFI_STATUS EFIAPI BootWindowsBootManager (
    IN BOOT_MANAGER_ENTRY *Entry
    );

EFI_STATUS EFIAPI BootBloodchainWrapper (VOID);

// =============================================================================
// GUID DEFINITION
// =============================================================================

#define BOOT_MANAGER_PROTOCOL_GUID \
    { 0x8B8E7E1F, 0x5C4A, 0x4A2B, { 0x9A, 0x1F, 0x8B, 0x3C, 0x7D, 0x2E, 0x4F, 0x6A } }

#endif // _BOOT_MANAGER_PROTOCOL_H_
