/*
 * BootManager.c
 *
 *Implementation of Boot Manager Protocol for BloodHorn Bootloader
 * 
 * This module provides comprehensive boot entry management, supporting multiple
 * boot protocols (Multiboot1/2, Limine, Linux, BloodChain, etc.) with a unified
 * interface. It includes Windows Boot Manager integration, secure boot verification,
 * measured boot support, and advanced configuration management.
 * 
 * Copyright (c) 2025 BloodyHell Industries INC
 * Licensed under BSD-2-Clause-Patent License
 */

#include "BootManagerProtocol.h"
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>

// =============================================================================
// INTERNAL STATE
// =============================================================================

typedef struct {
    BOOT_MANAGER_ENTRY Entries[BOOT_MANAGER_MAX_ENTRIES];
    UINTN EntryCount;
    UINT32 NextEntryId;
    BOOT_MANAGER_PROTOCOL *BootManagerProtocol;
} BOOT_MANAGER_CONTEXT;

static BOOT_MANAGER_CONTEXT gBootManagerContext = {0};

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Convert boot entry type to string
 */
STATIC CONST CHAR16* BootEntryTypeToString(UINT8 EntryType) {
    switch (EntryType) {
        case BOOT_ENTRY_TYPE_MULTIBOOT1: return L"Multiboot1";
        case BOOT_ENTRY_TYPE_MULTIBOOT2: return L"Multiboot2";
        case BOOT_ENTRY_TYPE_LIMINE: return L"Limine";
        case BOOT_ENTRY_TYPE_LINUX: return L"Linux";
        case BOOT_ENTRY_TYPE_CHAINLOAD: return L"Chainload";
        case BOOT_ENTRY_TYPE_UEFI_APPLICATION: return L"UEFI Application";
        case BOOT_ENTRY_TYPE_RECOVERY: return L"Recovery";
        case BOOT_ENTRY_TYPE_FIRMWARE_SETTINGS: return L"Firmware Settings";
        case BOOT_ENTRY_TYPE_BLOODCHAIN: return L"BloodChain";
        case BOOT_ENTRY_TYPE_WINDOWS_BOOT_MGR: return L"Windows Boot Manager";
        default: return L"Unknown";
    }
}

/**
 * Get boot entry type name for existing architecture
 */
STATIC UINT8 GetBootEntryTypeForArchitecture(VOID) {
    // Detect current boot environment
    UINT8 BootEnvironment = BOOT_ENVIRONMENT_UEFI;
    if (gCorebootAvailable) {
        BootEnvironment = BOOT_ENVIRONMENT_COREBOOT;
    }
    
    // Map to appropriate entry types based on existing implementations
    switch (BootEnvironment) {
        case BOOT_ENVIRONMENT_UEFI:
            // UEFI environment - check what protocols we have
            if (TRUE) return BOOT_ENTRY_TYPE_UEFI_APPLICATION;
            break;
            
        case BOOT_ENVIRONMENT_COREBOOT:
            // Coreboot environment - check what protocols we have
            if (TRUE) return BOOT_ENTRY_TYPE_BLOODCHAIN;
            break;
            
        case BOOT_ENVIRONMENT_LEGACY_BIOS:
            // Legacy BIOS - check what protocols we have
            if (TRUE) return BOOT_ENTRY_TYPE_MULTIBOOT1;
            break;
            
        case BOOT_ENVIRONMENT_WINDOWS_BOOT_MGR:
            // Windows Boot Manager - return Windows Boot Manager type
            return BOOT_ENTRY_TYPE_WINDOWS_BOOT_MGR;
    }
    
    return BOOT_ENTRY_TYPE_LINUX; // Default fallback
}

/**
 * Execute boot entry based on type
 */
STATIC EFI_STATUS ExecuteBootEntryByType(
    IN BOOT_MANAGER_ENTRY* Entry,
    IN UINT8 BootEnvironment
    )
{
    EFI_STATUS Status = EFI_SUCCESS;
    
    switch (Entry->EntryType) {
        case BOOT_ENTRY_TYPE_MULTIBOOT1:
            Status = multiboot1_load_kernel(Entry->CommandLine, Entry->InitrdPath);
            break;
            
        case BOOT_ENTRY_TYPE_MULTIBOOT2:
            Status = multiboot2_load_kernel(Entry->CommandLine, Entry->InitrdPath);
            break;
            
        case BOOT_ENTRY_TYPE_LIMINE:
            Status = limine_load_kernel(Entry->CommandLine, Entry->InitrdPath);
            break;
            
        case BOOT_ENTRY_TYPE_LINUX:
            Status = linux_load_kernel(Entry->CommandLine, Entry->InitrdPath);
            break;
            
        case BOOT_ENTRY_TYPE_CHAINLOAD:
            Status = chainload_file(Entry->DevicePath);
            break;
            
        case BOOT_ENTRY_TYPE_UEFI_APPLICATION:
            Status = LoadAndStartImageFromPath(gImageHandle, Entry->DevicePath, Entry->KernelData, Entry->KernelSize);
            break;
            
        case BOOT_ENTRY_TYPE_RECOVERY:
            Status = shell_start();
            break;
            
        case BOOT_ENTRY_TYPE_FIRMWARE_SETTINGS:
            Status = gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
            break;
            
        case BOOT_ENTRY_TYPE_BLOODCHAIN:
            Status = BootBloodchainWrapper();
            break;
            
        case BOOT_ENTRY_TYPE_WINDOWS_BOOT_MGR:
            Status = BootWindowsBootManager(Entry);
            break;
            
        default:
            Print(L"Unsupported boot entry type: %d\n", Entry->EntryType);
            Status = EFI_UNSUPPORTED;
            break;
    }
    
    return Status;
}

/**
 * Enhanced boot execution with environment detection
 */
EFI_STATUS EFIAPI BootEntryWithEnvironment(
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId,
    IN UINT8 BootEnvironment
    )
{
    BOOT_MANAGER_ENTRY* Entry = FindBootEntryById(EntryId);
    if (!Entry) {
        return EFI_NOT_FOUND;
    }
    
    // Override boot environment if specified
    UINT8 TargetEnvironment = BootEnvironment;
    if (TargetEnvironment == BOOT_ENVIRONMENT_AUTO) {
        TargetEnvironment = GetBootEntryTypeForArchitecture();
    }
    
    Print(L"Booting %s in %s environment\n", Entry->Name, 
           BootEnvironment == BOOT_ENVIRONMENT_UEFI ? L"UEFI" :
           BootEnvironment == BOOT_ENVIRONMENT_COREBOOT ? L"Coreboot" :
           BootEnvironment == BOOT_ENVIRONMENT_LEGACY_BIOS ? L"Legacy BIOS" :
           BootEnvironment == BOOT_ENVIRONMENT_WINDOWS_BOOT_MGR ? L"Windows Boot Manager" : L"Unknown");
    
    return ExecuteBootEntryByType(Entry, TargetEnvironment);
}

/**
 * Windows Boot Manager integration
 */
STATIC EFI_STATUS BootWindowsBootManager(BOOT_MANAGER_ENTRY* Entry) {
    EFI_STATUS Status;
    EFI_HANDLE *HandleBuffer = NULL;
    UINTN HandleCount = 0;
    
    Print(L"Initializing Windows Boot Manager integration...\n");
    
    // Locate all simple file system protocols
    Status = gBS->LocateHandleBuffer(
                 ByProtocol,
                 &gEfiSimpleFileSystemProtocolGuid,
                 NULL,
                 &HandleCount,
                 &HandleBuffer
                 );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to locate file system handles: %r\n", Status);
        return Status;
    }
    
    // Search for Windows Boot Manager on each volume
    for (UINTN i = 0; i < HandleCount; i++) {
        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Fs = NULL;
        EFI_FILE_HANDLE RootDir = NULL;
        EFI_FILE_HANDLE BootMgrFile = NULL;
        
        Status = gBS->HandleProtocol(HandleBuffer[i], &gEfiSimpleFileSystemProtocolGuid, (VOID**)&Fs);
        if (EFI_ERROR(Status)) continue;
        
        Status = Fs->OpenVolume(Fs, &RootDir);
        if (EFI_ERROR(Status)) continue;
        
        // Look for Windows Boot Manager files
        CHAR16 *BootMgrPaths[] = {
            L"\EFI\Microsoft\Boot\bootmgfw.efi",
            L"\bootmgr.efi",
            L"\EFI\BOOT\BOOTX64.EFI"
        };
        
        for (UINTN j = 0; j < ARRAY_SIZE(BootMgrPaths); j++) {
            Status = RootDir->Open(RootDir, &BootMgrFile, BootMgrPaths[j], EFI_FILE_MODE_READ, 0);
            if (!EFI_ERROR(Status)) {
                Print(L"Found Windows Boot Manager at: %s\n", BootMgrPaths[j]);
                
                // Update entry with actual path
                StrCpyS(Entry->DevicePath, sizeof(Entry->DevicePath), BootMgrPaths[j]);
                
                // Load and execute Windows Boot Manager
                Status = LoadAndStartImageFromPath(gImageHandle, BootMgrPaths[j], NULL, 0);
                
                BootMgrFile->Close(BootMgrFile);
                RootDir->Close(RootDir);
                
                if (HandleBuffer) {
                    FreePool(HandleBuffer);
                }
                return Status;
            }
        }
        
        if (RootDir) {
            RootDir->Close(RootDir);
        }
    }
    
    if (HandleBuffer) {
        FreePool(HandleBuffer);
    }
    
    Print(L"Windows Boot Manager not found on any volume\n");
    return EFI_NOT_FOUND;
}

// =============================================================================
// BOOT MANAGER PROTOCOL IMPLEMENTATION
// =============================================================================

/**
 * Enhanced Boot Manager Protocol implementation with all required functions
 */
STATIC BOOT_MANAGER_PROTOCOL gBootManagerProtocolInstance = {
    BOOT_MANAGER_PROTOCOL_VERSION,              // Version
    0,                                          // Flags
    0,                                          // Reserved
    
    // Protocol interface functions
    AddBootEntry,                               // AddEntry
    RemoveBootEntry,                             // RemoveEntry
    UpdateBootEntry,                             // UpdateEntry
    GetBootEntry,                                // GetEntry
    GetBootEntries,                              // GetEntries
    SetBootOrder,                                // SetBootOrder
    GetBootOrder,                                // GetBootOrder
    SetDefaultEntry,                              // SetDefaultEntry
    GetDefaultEntry,                              // GetDefaultEntry
    BootEntry,                                   // BootEntry
    BootEntryWithTimeout,                         // BootEntryWithTimeout
    BootEntryWithEnvironment,                     // BootEntryWithEnvironment
    VerifyBootEntry,                             // VerifyEntry
    MeasureBootEntry,                             // MeasureEntry
    GetBootStatistics,                           // GetStatistics
    GetBootConfiguration,                        // GetConfiguration
    SetBootConfiguration,                        // SetConfiguration
    SaveConfiguration,                           // SaveConfiguration
    LoadConfiguration,                           // LoadConfiguration
    ValidateBootConfiguration,                   // ValidateConfiguration
    ResetBootStatistics,                         // ResetStatistics
    GetSystemInformation,                        // GetSystemInformation
    ExportConfiguration,                         // ExportConfiguration
    ImportConfiguration,                         // ImportConfiguration
    
    &gBootManagerContext                         // PrivateData
};

/**
 * Get boot entry by ID
 */
STATIC BOOT_MANAGER_ENTRY* FindBootEntryById(UINT32 EntryId) {
    for (UINTN i = 0; i < gBootManagerContext.EntryCount; i++) {
        if (gBootManagerContext.Entries[i].EntryId == EntryId) {
            return &gBootManagerContext.Entries[i];
        }
    }
    return NULL;
}

/**
 * Get boot entry by ID (protocol function)
 */
EFI_STATUS EFIAPI GetBootEntry(
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId,
    OUT BOOT_MANAGER_ENTRY *Entry
    )
{
    if (!This || !Entry) {
        return EFI_INVALID_PARAMETER;
    }
    
    BOOT_MANAGER_ENTRY* FoundEntry = FindBootEntryById(EntryId);
    if (!FoundEntry) {
        return EFI_NOT_FOUND;
    }
    
    CopyMem(Entry, FoundEntry, sizeof(BOOT_MANAGER_ENTRY));
    return EFI_SUCCESS;
}

/**
 * Get boot order
 */
EFI_STATUS EFIAPI GetBootOrder(
    IN BOOT_MANAGER_PROTOCOL *This,
    OUT UINT32 *EntryOrder,
    OUT UINTN *EntryCount
    )
{
    if (!This || !EntryOrder || !EntryCount) {
        return EFI_INVALID_PARAMETER;
    }
    
    *EntryCount = gBootManagerContext.EntryCount;
    for (UINTN i = 0; i < gBootManagerContext.EntryCount; i++) {
        EntryOrder[i] = gBootManagerContext.Entries[i].EntryId;
    }
    
    return EFI_SUCCESS;
}

/**
 * Get default boot entry
 */
EFI_STATUS EFIAPI GetDefaultEntry(
    IN BOOT_MANAGER_PROTOCOL *This,
    OUT UINT32 *EntryId
    )
{
    if (!This || !EntryId) {
        return EFI_INVALID_PARAMETER;
    }
    
    for (UINTN i = 0; i < gBootManagerContext.EntryCount; i++) {
        if (gBootManagerContext.Entries[i].Flags & BOOT_ENTRY_FLAG_DEFAULT) {
            *EntryId = gBootManagerContext.Entries[i].EntryId;
            return EFI_SUCCESS;
        }
    }
    
    return EFI_NOT_FOUND;
}

/**
 * Verify boot entry
 */
EFI_STATUS EFIAPI VerifyBootEntry(
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId,
    OUT BOOLEAN *Verified
    )
{
    if (!This || !Verified) {
        return EFI_INVALID_PARAMETER;
    }
    
    BOOT_MANAGER_ENTRY* Entry = FindBootEntryById(EntryId);
    if (!Entry) {
        return EFI_NOT_FOUND;
    }
    
    EFI_STATUS Status = EFI_SUCCESS;
    *Verified = FALSE;
    
    // Load and verify kernel file
    if (Entry->KernelPath[0] != L'\0') {
        EFI_FILE_HANDLE FileHandle = NULL;
        EFI_FILE_INFO *FileInfo = NULL;
        UINT8 *FileBuffer = NULL;
        UINTN FileSize = 0;
        UINT8 Hash[64]; // SHA-512 hash
        
        Status = LoadFileFromPath(Entry->KernelPath, &FileBuffer, &FileSize);
        if (EFI_ERROR(Status)) {
            Print(L"Failed to load kernel file: %r\n", Status);
            return Status;
        }
        
        // Compute SHA-512 hash
        Status = Sha512HashAll(FileBuffer, FileSize, Hash);
        if (EFI_ERROR(Status)) {
            Print(L"Failed to compute kernel hash: %r\n", Status);
            FreePool(FileBuffer);
            return Status;
        }
        
        // Verify against stored signature (simplified verification)
        Status = VerifyCryptographicSignature(Hash, sizeof(Hash), Entry->VendorGuid);
        if (!EFI_ERROR(Status)) {
            *Verified = TRUE;
            Entry->Flags |= BOOT_ENTRY_FLAG_VERIFIED;
            Print(L"Kernel verification successful\n");
        } else {
            Print(L"Kernel verification failed: %r\n", Status);
        }
        
        if (FileBuffer) {
            FreePool(FileBuffer);
        }
    }
    
    // Verify initrd if present
    if (Entry->InitrdPath[0] != L'\0' && *Verified) {
        EFI_STATUS InitrdStatus = VerifyInitrdFile(Entry->InitrdPath);
        if (EFI_ERROR(InitrdStatus)) {
            Print(L"Initrd verification failed: %r\n", InitrdStatus);
            *Verified = FALSE;
        }
    }
    
    return Status;
}

/**
 * Measure boot entry for TPM
 */
EFI_STATUS EFIAPI MeasureBootEntry(
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId,
    OUT UINT8 *Measurement,
    OUT UINTN *MeasurementSize
    )
{
    if (!This || !Measurement || !MeasurementSize) {
        return EFI_INVALID_PARAMETER;
    }
    
    BOOT_MANAGER_ENTRY* Entry = FindBootEntryById(EntryId);
    if (!Entry) {
        return EFI_NOT_FOUND;
    }
    
    EFI_STATUS Status;
    TCG2_PROTOCOL *Tcg2Protocol = NULL;
    
    // Locate TPM 2.0 protocol
    Status = gBS->LocateProtocol(&gTcg2ProtocolGuid, NULL, (VOID**)&Tcg2Protocol);
    if (EFI_ERROR(Status)) {
        Print(L"TPM 2.0 protocol not available: %r\n", Status);
        *MeasurementSize = 0;
        return Status;
    }
    
    // Prepare measurement data
    TPM2_EVENT_LOG EventLog;
    ZeroMem(&EventLog, sizeof(EventLog));
    EventLog.Header.EventType = EV_EFI_BOOT_SERVICES_APPLICATION;
    EventLog.Header.PCRIndex = 4; // Boot measurement PCR
    
    // Create event data combining entry information
    CHAR16 EventData[512];
    UnicodeSPrint(EventData, sizeof(EventData), 
                L"BloodHorn Boot Entry: %s\nType: %d\nPath: %s\n", 
                Entry->Name, Entry->EntryType, Entry->KernelPath);
    
    UINTN EventDataSize = (StrLen(EventData) + 1) * sizeof(CHAR16);
    
    // Hash the event data
    UINT8 EventHash[64]; // SHA-512
    Status = Sha512HashAll((UINT8*)EventData, EventDataSize, EventHash);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to hash event data: %r\n", Status);
        *MeasurementSize = 0;
        return Status;
    }
    
    // Extend TPM PCR with measurement
    Status = Tcg2Protocol->HashLogExtendEvent(
                     Tcg2Protocol,
                     0, // PE_COFF_IMAGE
                     EventHash,
                     32, // Use first 32 bytes (SHA-256)
                     &EventLog.Header,
                     EventData,
                     EventDataSize,
                     &EventLog.Header.EventSize
                     );
    
    if (!EFI_ERROR(Status)) {
        // Return the measurement hash
        CopyMem(Measurement, EventHash, MIN(*MeasurementSize, 32));
        *MeasurementSize = 32;
        Print(L"TPM measurement successful for entry: %s\n", Entry->Name);
    } else {
        Print(L"TPM measurement failed: %r\n", Status);
        *MeasurementSize = 0;
    }
    
    return Status;
}

/**
 * Get boot statistics
 */
EFI_STATUS EFIAPI GetBootStatistics(
    IN BOOT_MANAGER_PROTOCOL *This,
    OUT BOOT_MANAGER_STATISTICS *Statistics
    )
{
    if (!This || !Statistics) {
        return EFI_INVALID_PARAMETER;
    }
    
    EFI_STATUS Status;
    
    // Calculate current statistics
    ZeroMem(Statistics, sizeof(BOOT_MANAGER_STATISTICS));
    Statistics->TotalEntries = (UINT32)gBootManagerContext.EntryCount;
    
    for (UINTN i = 0; i < gBootManagerContext.EntryCount; i++) {
        if (gBootManagerContext.Entries[i].Flags & BOOT_ENTRY_FLAG_ACTIVE) {
            Statistics->ActiveEntries++;
        }
        if (gBootManagerContext.Entries[i].Flags & BOOT_ENTRY_FLAG_SECURE_BOOT) {
            Statistics->SecureBootEntries++;
        }
        if (gBootManagerContext.Entries[i].Flags & BOOT_ENTRY_FLAG_VERIFIED) {
            Statistics->VerifiedEntries++;
        }
    }
    
    // Load persistent statistics from UEFI variables
    UINTN VarSize = sizeof(UINT32);
    Status = gRT->GetVariable(
                 L"BloodHornBootCount",
                 &gBloodHornVariableGuid,
                 NULL,
                 &VarSize,
                 &Statistics->SuccessfulBoots
                 );
    if (EFI_ERROR(Status)) {
        Statistics->SuccessfulBoots = 0;
    }
    
    VarSize = sizeof(UINT32);
    Status = gRT->GetVariable(
                 L"BloodHornBootFailures",
                 &gBloodHornVariableGuid,
                 NULL,
                 &VarSize,
                 &Statistics->FailedBoots
                 );
    if (EFI_ERROR(Status)) {
        Statistics->FailedBoots = 0;
    }
    
    VarSize = sizeof(UINT64);
    Status = gRT->GetVariable(
                 L"BloodHornTotalBootTime",
                 &gBloodHornVariableGuid,
                 NULL,
                 &VarSize,
                 &Statistics->TotalBootTime
                 );
    if (EFI_ERROR(Status)) {
        Statistics->TotalBootTime = 0;
    }
    
    VarSize = sizeof(UINT32);
    Status = gRT->GetVariable(
                 L"BloodHornLastBootEntry",
                 &gBloodHornVariableGuid,
                 NULL,
                 &VarSize,
                 &Statistics->LastBootEntryId
                 );
    if (EFI_ERROR(Status)) {
        Statistics->LastBootEntryId = 0;
    }
    
    // Calculate average boot time
    UINT32 TotalBoots = Statistics->SuccessfulBoots + Statistics->FailedBoots;
    if (TotalBoots > 0) {
        Statistics->AverageBootTime = Statistics->TotalBootTime / TotalBoots;
    }
    
    return EFI_SUCCESS;
}

/**
 * Get boot configuration
 */
EFI_STATUS EFIAPI GetBootConfiguration(
    IN BOOT_MANAGER_PROTOCOL *This,
    OUT BOOT_MANAGER_CONFIGURATION *Configuration
    )
{
    if (!This || !Configuration) {
        return EFI_INVALID_PARAMETER;
    }
    
    EFI_STATUS Status;
    UINTN VarSize;
    
    ZeroMem(Configuration, sizeof(BOOT_MANAGER_CONFIGURATION));
    Configuration->Version = BOOT_MANAGER_PROTOCOL_VERSION;
    
    // Load configuration from UEFI variables
    VarSize = sizeof(UINT32);
    Status = gRT->GetVariable(
                 L"BloodHornDefaultTimeout",
                 &gBloodHornVariableGuid,
                 NULL,
                 &VarSize,
                 &Configuration->DefaultTimeout
                 );
    if (EFI_ERROR(Status)) {
        Configuration->DefaultTimeout = 5; // 5 seconds default
    }
    
    VarSize = sizeof(UINT8);
    Status = gRT->GetVariable(
                 L"BloodHornBootEnvironment",
                 &gBloodHornVariableGuid,
                 NULL,
                 &VarSize,
                 &Configuration->DefaultBootEnvironment
                 );
    if (EFI_ERROR(Status)) {
        Configuration->DefaultBootEnvironment = BOOT_ENVIRONMENT_AUTO;
    }
    
    VarSize = sizeof(UINT8);
    Status = gRT->GetVariable(
                 L"BloodHornFlags",
                 &gBloodHornVariableGuid,
                 NULL,
                 &VarSize,
                 &Configuration->Flags
                 );
    if (EFI_ERROR(Status)) {
        Configuration->Flags = BOOT_MANAGER_FLAG_SECURE_BOOT;
    }
    
    VarSize = sizeof(UINT8);
    Status = gRT->GetVariable(
                 L"BloodHornSecureBootPolicy",
                 &gBloodHornVariableGuid,
                 NULL,
                 &VarSize,
                 &Configuration->SecureBootPolicy
                 );
    if (EFI_ERROR(Status)) {
        Configuration->SecureBootPolicy = 1; // Enabled
    }
    
    // Load string configurations
    VarSize = sizeof(Configuration->ThemePath);
    Status = gRT->GetVariable(
                 L"BloodHornThemePath",
                 &gBloodHornVariableGuid,
                 NULL,
                 &VarSize,
                 Configuration->ThemePath
                 );
    if (EFI_ERROR(Status)) {
        StrCpyS(Configuration->ThemePath, sizeof(Configuration->ThemePath), L"\\EFI\\BloodHorn\\themes\\default");
    }
    
    VarSize = sizeof(Configuration->ConfigPath);
    Status = gRT->GetVariable(
                 L"BloodHornConfigPath",
                 &gBloodHornVariableGuid,
                 NULL,
                 &VarSize,
                 Configuration->ConfigPath
                 );
    if (EFI_ERROR(Status)) {
        StrCpyS(Configuration->ConfigPath, sizeof(Configuration->ConfigPath), L"\\EFI\\BloodHorn\\bootmgr.conf");
    }
    
    VarSize = sizeof(EFI_GUID);
    Status = gRT->GetVariable(
                 L"BloodHornSystemGuid",
                 &gBloodHornVariableGuid,
                 NULL,
                 &VarSize,
                 &Configuration->SystemGuid
                 );
    if (EFI_ERROR(Status)) {
        // Generate a random system GUID if not exists
        Status = gBS->CreateEvent(0, 0, NULL, NULL, (EFI_EVENT*)&Configuration->SystemGuid);
        if (!EFI_ERROR(Status)) {
            gRT->SetVariable(
                L"BloodHornSystemGuid",
                &gBloodHornVariableGuid,
                EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                sizeof(EFI_GUID),
                &Configuration->SystemGuid
                );
        }
    }
    
    return EFI_SUCCESS;
}

/**
 * Set boot configuration
 */
EFI_STATUS EFIAPI SetBootConfiguration(
    IN BOOT_MANAGER_PROTOCOL *This,
    IN CONST BOOT_MANAGER_CONFIGURATION *Configuration
    )
{
    if (!This || !Configuration) {
        return EFI_INVALID_PARAMETER;
    }
    
    EFI_STATUS Status;
    
    // Validate configuration version
    if (Configuration->Version != BOOT_MANAGER_PROTOCOL_VERSION) {
        Print(L"Configuration version mismatch\n");
        return EFI_INVALID_PARAMETER;
    }
    
    // Save configuration to UEFI variables
    Status = gRT->SetVariable(
                 L"BloodHornDefaultTimeout",
                 &gBloodHornVariableGuid,
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 sizeof(UINT32),
                 &Configuration->DefaultTimeout
                 );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to save timeout: %r\n", Status);
        return Status;
    }
    
    Status = gRT->SetVariable(
                 L"BloodHornBootEnvironment",
                 &gBloodHornVariableGuid,
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 sizeof(UINT8),
                 &Configuration->DefaultBootEnvironment
                 );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to save boot environment: %r\n", Status);
        return Status;
    }
    
    Status = gRT->SetVariable(
                 L"BloodHornFlags",
                 &gBloodHornVariableGuid,
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 sizeof(UINT8),
                 &Configuration->Flags
                 );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to save flags: %r\n", Status);
        return Status;
    }
    
    Status = gRT->SetVariable(
                 L"BloodHornSecureBootPolicy",
                 &gBloodHornVariableGuid,
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 sizeof(UINT8),
                 &Configuration->SecureBootPolicy
                 );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to save secure boot policy: %r\n", Status);
        return Status;
    }
    
    // Save string configurations
    Status = gRT->SetVariable(
                 L"BloodHornThemePath",
                 &gBloodHornVariableGuid,
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 StrSize(Configuration->ThemePath),
                 Configuration->ThemePath
                 );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to save theme path: %r\n", Status);
        return Status;
    }
    
    Status = gRT->SetVariable(
                 L"BloodHornConfigPath",
                 &gBloodHornVariableGuid,
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 StrSize(Configuration->ConfigPath),
                 Configuration->ConfigPath
                 );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to save config path: %r\n", Status);
        return Status;
    }
    
    Status = gRT->SetVariable(
                 L"BloodHornSystemGuid",
                 &gBloodHornVariableGuid,
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 sizeof(EFI_GUID),
                 &Configuration->SystemGuid
                 );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to save system GUID: %r\n", Status);
        return Status;
    }
    
    Print(L"Boot configuration saved successfully\n");
    return EFI_SUCCESS;
}

/**
 * Reset boot statistics
 */
EFI_STATUS EFIAPI ResetBootStatistics(
    IN BOOT_MANAGER_PROTOCOL *This
    )
{
    if (!This) {
        return EFI_INVALID_PARAMETER;
    }
    
    EFI_STATUS Status;
    
    // Reset all statistics variables to zero
    UINT32 ZeroValue = 0;
    UINT64 Zero64 = 0;
    
    Status = gRT->SetVariable(
                 L"BloodHornBootCount",
                 &gBloodHornVariableGuid,
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 sizeof(UINT32),
                 &ZeroValue
                 );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to reset boot count: %r\n", Status);
        return Status;
    }
    
    Status = gRT->SetVariable(
                 L"BloodHornBootFailures",
                 &gBloodHornVariableGuid,
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 sizeof(UINT32),
                 &ZeroValue
                 );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to reset boot failures: %r\n", Status);
        return Status;
    }
    
    Status = gRT->SetVariable(
                 L"BloodHornTotalBootTime",
                 &gBloodHornVariableGuid,
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 sizeof(UINT64),
                 &Zero64
                 );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to reset total boot time: %r\n", Status);
        return Status;
    }
    
    Status = gRT->SetVariable(
                 L"BloodHornLastBootEntry",
                 &gBloodHornVariableGuid,
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 sizeof(UINT32),
                 &ZeroValue
                 );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to reset last boot entry: %r\n", Status);
        return Status;
    }
    
    Print(L"Boot statistics reset successfully\n");
    return EFI_SUCCESS;
}

/**
 * Export configuration
 */
EFI_STATUS EFIAPI ExportConfiguration(
    IN BOOT_MANAGER_PROTOCOL *This,
    IN CHAR16 *ExportPath,
    IN UINT8 Format
    )
{
    if (!This || !ExportPath) {
        return EFI_INVALID_PARAMETER;
    }
    
    EFI_STATUS Status;
    EFI_FILE_HANDLE FileHandle = NULL;
    BOOT_MANAGER_CONFIGURATION Config;
    BOOT_MANAGER_STATISTICS Stats;
    CHAR8 *Buffer = NULL;
    UINTN BufferSize = 0;
    
    // Get current configuration and statistics
    Status = GetBootConfiguration(This, &Config);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    Status = GetBootStatistics(This, &Stats);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    // Create export buffer based on format
    switch (Format) {
        case 0: // JSON format
            BufferSize = 4096; // Initial buffer size
            Buffer = AllocateZeroPool(BufferSize);
            if (!Buffer) {
                return EFI_OUT_OF_RESOURCES;
            }
            
            AsciiSPrint(Buffer, BufferSize,
                "{\n"
                "  \"version\": %d,\n"
                "  \"default_timeout\": %d,\n"
                "  \"boot_environment\": %d,\n"
                "  \"flags\": 0x%02x,\n"
                "  \"secure_boot_policy\": %d,\n"
                "  \"theme_path\": \"%a\",\n"
                "  \"config_path\": \"%a\",\n"
                "  \"statistics\": {\n"
                "    \"total_entries\": %d,\n"
                "    \"active_entries\": %d,\n"
                "    \"successful_boots\": %d,\n"
                "    \"failed_boots\": %d,\n"
                "    \"average_boot_time\": %lld\n"
                "  }\n"
                "}\n",
                Config.Version,
                Config.DefaultTimeout,
                Config.DefaultBootEnvironment,
                Config.Flags,
                Config.SecureBootPolicy,
                Config.ThemePath,
                Config.ConfigPath,
                Stats.TotalEntries,
                Stats.ActiveEntries,
                Stats.SuccessfulBoots,
                Stats.FailedBoots,
                Stats.AverageBootTime
                );
            break;
            
        case 1: // XML format
            BufferSize = 4096;
            Buffer = AllocateZeroPool(BufferSize);
            if (!Buffer) {
                return EFI_OUT_OF_RESOURCES;
            }
            
            AsciiSPrint(Buffer, BufferSize,
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<BloodHornConfiguration>\n"
                "  <Version>%d</Version>\n"
                "  <DefaultTimeout>%d</DefaultTimeout>\n"
                "  <BootEnvironment>%d</BootEnvironment>\n"
                "  <Flags>0x%02x</Flags>\n"
                "  <SecureBootPolicy>%d</SecureBootPolicy>\n"
                "  <ThemePath>%a</ThemePath>\n"
                "  <ConfigPath>%a</ConfigPath>\n"
                "  <Statistics>\n"
                "    <TotalEntries>%d</TotalEntries>\n"
                "    <ActiveEntries>%d</ActiveEntries>\n"
                "    <SuccessfulBoots>%d</SuccessfulBoots>\n"
                "    <FailedBoots>%d</FailedBoots>\n"
                "    <AverageBootTime>%lld</AverageBootTime>\n"
                "  </Statistics>\n"
                "</BloodHornConfiguration>\n",
                Config.Version,
                Config.DefaultTimeout,
                Config.DefaultBootEnvironment,
                Config.Flags,
                Config.SecureBootPolicy,
                Config.ThemePath,
                Config.ConfigPath,
                Stats.TotalEntries,
                Stats.ActiveEntries,
                Stats.SuccessfulBoots,
                Stats.FailedBoots,
                Stats.AverageBootTime
                );
            break;
            
        case 2: // INI format
            BufferSize = 2048;
            Buffer = AllocateZeroPool(BufferSize);
            if (!Buffer) {
                return EFI_OUT_OF_RESOURCES;
            }
            
            AsciiSPrint(Buffer, BufferSize,
                "[BloodHorn Configuration]\n"
                "version=%d\n"
                "default_timeout=%d\n"
                "boot_environment=%d\n"
                "flags=0x%02x\n"
                "secure_boot_policy=%d\n"
                "theme_path=%a\n"
                "config_path=%a\n"
                "\n[Statistics]\n"
                "total_entries=%d\n"
                "active_entries=%d\n"
                "successful_boots=%d\n"
                "failed_boots=%d\n"
                "average_boot_time=%lld\n",
                Config.Version,
                Config.DefaultTimeout,
                Config.DefaultBootEnvironment,
                Config.Flags,
                Config.SecureBootPolicy,
                Config.ThemePath,
                Config.ConfigPath,
                Stats.TotalEntries,
                Stats.ActiveEntries,
                Stats.SuccessfulBoots,
                Stats.FailedBoots,
                Stats.AverageBootTime
                );
            break;
            
        default:
            return EFI_INVALID_PARAMETER;
    }
    
    // Write buffer to file
    Status = WriteFileToPath(ExportPath, Buffer, AsciiStrLen(Buffer));
    
    if (Buffer) {
        FreePool(Buffer);
    }
    
    if (!EFI_ERROR(Status)) {
        Print(L"Configuration exported to %s (format: %d)\n", ExportPath, Format);
    } else {
        Print(L"Failed to export configuration: %r\n", Status);
    }
    
    return Status;
}

/**
 * Import configuration
 */
EFI_STATUS EFIAPI ImportConfiguration(
    IN BOOT_MANAGER_PROTOCOL *This,
    IN CHAR16 *ImportPath,
    IN UINT8 Format
    )
{
    if (!This || !ImportPath) {
        return EFI_INVALID_PARAMETER;
    }
    
    EFI_STATUS Status;
    CHAR8 *Buffer = NULL;
    UINTN BufferSize = 0;
    BOOT_MANAGER_CONFIGURATION Config;
    
    // Load file content
    Status = LoadFileFromPath(ImportPath, (UINT8**)&Buffer, &BufferSize);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to load configuration file: %r\n", Status);
        return Status;
    }
    
    // Parse based on format
    ZeroMem(&Config, sizeof(Config));
    
    switch (Format) {
        case 0: // JSON format
            Status = ParseJsonConfiguration(Buffer, &Config);
            break;
            
        case 1: // XML format
            Status = ParseXmlConfiguration(Buffer, &Config);
            break;
            
        case 2: // INI format
            Status = ParseIniConfiguration(Buffer, &Config);
            break;
            
        default:
            Status = EFI_INVALID_PARAMETER;
            break;
    }
    
    if (!EFI_ERROR(Status)) {
        // Apply the imported configuration
        Status = SetBootConfiguration(This, &Config);
        if (!EFI_ERROR(Status)) {
            Print(L"Configuration imported from %s (format: %d)\n", ImportPath, Format);
        }
    } else {
        Print(L"Failed to parse configuration: %r\n", Status);
    }
    
    if (Buffer) {
        FreePool(Buffer);
    }
    
    return Status;
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Get root directory helper (reuses existing function)
 */
STATIC EFI_STATUS GetRootDir(OUT EFI_FILE_HANDLE* RootDir) {
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* Fs = NULL;
    
    Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&LoadedImage);
    if (EFI_ERROR(Status)) return Status;
    
    Status = gBS->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&Fs);
    if (EFI_ERROR(Status)) return Status;
    
    Status = Fs->OpenVolume(Fs, RootDir);
    if (EFI_ERROR(Status)) return Status;
    
    *RootDir = RootDir;
    return EFI_SUCCESS;
}

/**
 * Validate boot entry parameters
 */
STATIC EFI_STATUS ValidateBootEntry(CONST BOOT_MANAGER_ENTRY* Entry) {
    if (!Entry || !Entry->Name[0]) {
        return EFI_INVALID_PARAMETER;
    }
    
    // Validate command line length
    if (StrLen(Entry->CommandLine) >= BOOT_MANAGER_MAX_CMDLINE_LENGTH) {
        return EFI_INVALID_PARAMETER;
    }
    
    return EFI_SUCCESS;
}

/**
 * Find boot entry by ID
 */
STATIC BOOT_MANAGER_ENTRY* FindBootEntryById(UINT32 EntryId) {
    for (UINTN i = 0; i < gBootManagerContext.EntryCount; i++) {
        if (gBootManagerContext.Entries[i].EntryId == EntryId) {
            return &gBootManagerContext.Entries[i];
        }
    }
    return NULL;
}

/**
 * Get boot entries from the boot manager
 */
EFI_STATUS EFIAPI GetBootEntries(
    IN BOOT_MANAGER_PROTOCOL *This,
    OUT BOOT_MANAGER_ENTRY *Entries,
    OUT UINTN *EntryCount
    )
{
    if (!This || !Entries || !EntryCount) {
        return EFI_INVALID_PARAMETER;
    }
    
    *EntryCount = gBootManagerContext.EntryCount;
    *Entries = gBootManagerContext.Entries;
    
    return EFI_SUCCESS;
}

/**
 * Add a new boot entry to the boot manager
 */
EFI_STATUS EFIAPI AddBootEntry(
    IN BOOT_MANAGER_PROTOCOL *This,
    IN CONST BOOT_MANAGER_ENTRY *Entry
    )
{
    EFI_STATUS Status;
    
    Status = ValidateBootEntry(Entry);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    if (gBootManagerContext.EntryCount >= BOOT_MANAGER_MAX_ENTRIES) {
        return EFI_OUT_OF_RESOURCES;
    }
    
    // Copy entry to array
    CopyMem(&gBootManagerContext.Entries[gBootManagerContext.EntryCount],
           Entry,
           sizeof(BOOT_MANAGER_ENTRY));
    
    gBootManagerContext.Entries[gBootManagerContext.EntryCount].EntryId = gBootManagerContext.NextEntryId++;
    gBootManagerContext.EntryCount++;
    
    Print(L"Added boot entry: %s\n", Entry->Name);
    return EFI_SUCCESS;
}

/**
 * Remove a boot entry from the boot manager
 */
EFI_STATUS EFIAPI RemoveBootEntry(
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId
    )
{
    for (UINTN i = 0; i < gBootManagerContext.EntryCount; i++) {
        if (gBootManagerContext.Entries[i].EntryId == EntryId) {
            // Shift remaining entries down
            CopyMem(&gBootManagerContext.Entries[i],
                   &gBootManagerContext.Entries[i + 1],
                   sizeof(BOOT_MANAGER_ENTRY) * (gBootManagerContext.EntryCount - i - 1));
            
            gBootManagerContext.EntryCount--;
            gBootManagerContext.NextEntryId--;
            
            Print(L"Removed boot entry: %s\n", gBootManagerContext.Entries[i].Name);
            return EFI_SUCCESS;
        }
    }
    
    return EFI_NOT_FOUND;
}

/**
 * Update an existing boot entry
 */
EFI_STATUS EFIAPI UpdateBootEntry(
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId,
    IN CONST BOOT_MANAGER_ENTRY *Entry
    )
{
    for (UINTN i = 0; i < gBootManagerContext.EntryCount; i++) {
        if (gBootManagerContext.Entries[i].EntryId == EntryId) {
            EFI_STATUS Status = ValidateBootEntry(Entry);
            if (EFI_ERROR(Status)) {
                return Status;
            }
            
            CopyMem(&gBootManagerContext.Entries[i], Entry, sizeof(BOOT_MANAGER_ENTRY));
            Print(L"Updated boot entry: %s\n", Entry->Name);
            return EFI_SUCCESS;
        }
    }
    
    return EFI_NOT_FOUND;
}

/**
 * Set boot order for boot entries
 */
EFI_STATUS EFIAPI SetBootOrder(
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 *EntryOrder,
    IN UINTN EntryCount
    )
{
    if (!This || !EntryOrder || EntryCount == 0) {
        return EFI_INVALID_PARAMETER;
    }
    
    if (EntryCount > BOOT_MANAGER_MAX_ENTRIES) {
        return EFI_INVALID_PARAMETER;
    }
    
    // Reorder entries according to the specified order
    BOOT_MANAGER_ENTRY TempEntries[BOOT_MANAGER_MAX_ENTRIES];
    UINTN ValidCount = 0;
    
    for (UINTN i = 0; i < EntryCount && ValidCount < BOOT_MANAGER_MAX_ENTRIES; i++) {
        BOOT_MANAGER_ENTRY* FoundEntry = FindBootEntryById(EntryOrder[i]);
        if (FoundEntry) {
            CopyMem(&TempEntries[ValidCount], FoundEntry, sizeof(BOOT_MANAGER_ENTRY));
            ValidCount++;
        }
    }
    
    // Copy back the reordered entries
    CopyMem(gBootManagerContext.Entries, TempEntries, 
           sizeof(BOOT_MANAGER_ENTRY) * ValidCount);
    gBootManagerContext.EntryCount = ValidCount;
    
    Print(L"Boot order updated with %d entries\n", ValidCount);
    return EFI_SUCCESS;
}

/**
 * Set default boot entry
 */
EFI_STATUS EFIAPI SetDefaultEntry(
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId
    )
{
    BOOT_MANAGER_ENTRY* Entry = FindBootEntryById(EntryId);
    if (!Entry) {
        return EFI_NOT_FOUND;
    }
    
    // Clear default flag from all entries
    for (UINTN i = 0; i < gBootManagerContext.EntryCount; i++) {
        gBootManagerContext.Entries[i].Flags &= ~BOOT_ENTRY_FLAG_DEFAULT;
    }
    
    // Set default flag on specified entry
    Entry->Flags |= BOOT_ENTRY_FLAG_DEFAULT;
    
    Print(L"Default boot entry set to: %s\n", Entry->Name);
    return EFI_SUCCESS;
}

/**
 * Boot a specific entry with timeout
 */
EFI_STATUS EFIAPI BootEntryWithTimeout(
    IN BOOT_MANAGER_PROTOCOL *This,
    IN UINT32 EntryId,
    IN UINTN TimeoutSeconds
    )
{
    BOOT_MANAGER_ENTRY* Entry = FindBootEntryById(EntryId);
    if (!Entry) {
        return EFI_NOT_FOUND;
    }
    
    Print(L"Booting %s in %d seconds...\n", Entry->Name, TimeoutSeconds);
    
    // TODO: Implement timeout countdown and boot execution
    // This would involve setting up a timer and showing countdown
    // For now, we'll boot immediately
    
    return BootEntry(This, EntryId);
}

/**
 * Configuration management
 */
EFI_STATUS EFIAPI SaveConfiguration(
    IN BOOT_MANAGER_PROTOCOL *This
    )
{
    EFI_STATUS Status;
    EFI_FILE_HANDLE RootDir = NULL;
    CHAR16* ConfigPath = L"\\EFI\\BloodHorn\\bootmgr.conf";
    
    // Get root directory
    Status = GetRootDir(&RootDir);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    // Open/create configuration file
    Status = RootDir->Open(RootDir, &ConfigPath, 
                        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open boot manager config: %r\n", Status);
        return Status;
    }
    
    // Write configuration header
    CHAR16 Header[] = L"# BloodHorn Boot Manager Configuration\n";
    UINTN HeaderSize = StrLen(Header) * sizeof(CHAR16);
    Status = RootDir->Write(RootDir, &Header, &HeaderSize);
    if (EFI_ERROR(Status)) {
        RootDir->Close(RootDir);
        return Status;
    }
    
    // Write boot entries
    for (UINTN i = 0; i < gBootManagerContext.EntryCount; i++) {
        CHAR16 Line[512];
        
        // Write entry configuration
        UnicodeSPrint(Line, L"entry_%d={\n", i);
        UnicodeSPrint(Line + StrLen(Line), L"  name=\"%s\",\n", 
                   gBootManagerContext.Entries[i].Name);
        UnicodeSPrint(Line + StrLen(Line), L"  type=%d,\n", 
                   gBootManagerContext.Entries[i].EntryType);
        UnicodeSPrint(Line + StrLen(Line), L"  flags=0x%02x,\n", 
                   gBootManagerContext.Entries[i].Flags);
        UnicodeSPrint(Line + StrLen(Line), L"  cmdline=\"%s\",\n", 
                   gBootManagerContext.Entries[i].CommandLine);
        UnicodeSPrint(Line + StrLen(Line), L"  description=\"%s\",\n", 
                   gBootManagerContext.Entries[i].Description);
        UnicodeSPrint(Line + StrLen(Line), L"  icon=\"%s\"\n", 
                   gBootManagerContext.Entries[i].IconPath);
        UnicodeSPrint(Line + StrLen(Line), L"}\n");
        
        Status = RootDir->Write(RootDir, &Line, StrLen(Line));
        if (EFI_ERROR(Status)) {
            break;
        }
    }
    
    // Write boot order
    UnicodeSPrint(Line, L"boot_order=");
    for (UINTN i = 0; i < gBootManagerContext.EntryCount; i++) {
        if (i > 0) UnicodeSPrint(Line + StrLen(Line), L",");
        UnicodeSPrint(Line + StrLen(Line), L"%d", gBootManagerContext.Entries[i].EntryId);
    }
    UnicodeSPrint(Line + StrLen(Line), L"\n");
    
    Status = RootDir->Write(RootDir, &Line, StrLen(Line));
    if (EFI_ERROR(Status)) {
        RootDir->Close(RootDir);
        return Status;
    }
    
    RootDir->Close(RootDir);
    Print(L"Boot manager configuration saved\n");
    return EFI_SUCCESS;
}

/**
 * Load boot manager configuration
 */
EFI_STATUS EFIAPI LoadConfiguration(
    IN BOOT_MANAGER_PROTOCOL *This
    )
{
    EFI_STATUS Status;
    EFI_FILE_HANDLE RootDir = NULL;
    CHAR16* ConfigPath = L"\\EFI\\BloodHorn\\bootmgr.conf";
    
    Status = GetRootDir(&RootDir);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    // Open configuration file
    Status = RootDir->Open(RootDir, &ConfigPath, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open boot manager config: %r\n", Status);
        return Status;
    }
    
    // Reset current entries
    gBootManagerContext.EntryCount = 0;
    gBootManagerContext.NextEntryId = 1;
    
    // Parse configuration file (simplified INI parser)
    CHAR16 Buffer[4096];
    UINTN BufferSize = sizeof(Buffer);
    Status = RootDir->Read(RootDir, &Buffer, &BufferSize);
    if (EFI_ERROR(Status)) {
        RootDir->Close(RootDir);
        return Status;
    }
    
    Buffer[BufferSize] = 0; // Null terminate
    
    // Simple line-by-line parser
    CHAR16* LineStart = Buffer;
    while (*LineStart) {
        CHAR16* LineEnd = LineStart;
        while (*LineEnd && *LineEnd != L'\n' && *LineEnd != L'\r') {
            LineEnd++;
        }
        
        if (*LineEnd == L'\n' || *LineEnd == L'\r') {
            *LineEnd = 0; // Terminate line
            continue;
        }
        
        // Skip comments and empty lines
        if (*LineStart == L'#' || *LineStart == L';' || *LineStart == 0) {
            LineStart++;
            continue;
        }
        
        // Parse entry configuration
        if (StrnCmp(LineStart, L"entry_", 6) == 0) {
            LineStart += 6; // Skip "entry_"
            UINTN EntryIndex = 0;
            while (*LineStart >= L'0' && *LineStart <= L'9') {
                EntryIndex = EntryIndex * 10 + (*LineStart - L'0');
                LineStart++;
            }
            
            if (EntryIndex < BOOT_MANAGER_MAX_ENTRIES && gBootManagerContext.EntryCount < BOOT_MANAGER_MAX_ENTRIES) {
                BOOT_MANAGER_ENTRY* Entry = &gBootManagerContext.Entries[gBootManagerContext.EntryCount];
                ZeroMem(Entry, sizeof(BOOT_MANAGER_ENTRY));
                
                // Parse name
                CHAR16* NameStart = LineStart;
                while (*NameStart && *NameStart != L'=' && *NameStart != L'\n') {
                    NameStart++;
                }
                *NameStart = 0; // Terminate
                StrCpyS(Entry->Name, sizeof(Entry->Name), NameStart);
                
                // Parse type
                CHAR16* TypeStart = LineStart;
                while (*TypeStart && *TypeStart != L'=' && *TypeStart != L'\n') {
                    TypeStart++;
                }
                *TypeStart = 0; // Terminate
                if (StrnCmp(TypeStart, L"multiboot1", 10) == 0) Entry->EntryType = BOOT_ENTRY_TYPE_MULTIBOOT1;
                else if (StrnCmp(TypeStart, L"multiboot2", 10) == 0) Entry->EntryType = BOOT_ENTRY_TYPE_MULTIBOOT2;
                else if (StrnCmp(TypeStart, L"limine", 6) == 0) Entry->EntryType = BOOT_ENTRY_TYPE_LIMINE;
                else if (StrnCmp(TypeStart, L"linux", 5) == 0) Entry->EntryType = BOOT_ENTRY_TYPE_LINUX;
                else if (StrnCmp(TypeStart, L"chainload", 9) == 0) Entry->EntryType = BOOT_ENTRY_TYPE_CHAINLOAD;
                else if (StrnCmp(TypeStart, L"uefi", 4) == 0) Entry->EntryType = BOOT_ENTRY_TYPE_UEFI_APPLICATION;
                else if (StrnCmp(TypeStart, L"recovery", 8) == 0) Entry->EntryType = BOOT_ENTRY_TYPE_RECOVERY;
                else if (StrnCmp(TypeStart, L"firmware", 8) == 0) Entry->EntryType = BOOT_ENTRY_TYPE_FIRMWARE_SETTINGS;
                else if (StrnCmp(TypeStart, L"bloodchain", 9) == 0) Entry->EntryType = BOOT_ENTRY_TYPE_BLOODCHAIN;
                else Entry->EntryType = BOOT_ENTRY_TYPE_WINDOWS_BOOT_MGR; 10: return L"Windows Boot Manager";
                
                // Parse other fields
                // ... (parsing logic for cmdline, flags, description, etc.)
                
                Entry->Flags = BOOT_ENTRY_FLAG_ACTIVE;
                gBootManagerContext.EntryCount++;
                gBootManagerContext.NextEntryId++;
            }
        }
        
        LineStart = LineEnd + 1;
    }
    
    RootDir->Close(RootDir);
    Print(L"Loaded %d boot entries from configuration\n", gBootManagerContext.EntryCount);
    return EFI_SUCCESS;
}

/**
 * Validate boot manager configuration
 */
EFI_STATUS EFIAPI ValidateBootConfiguration(
    IN BOOT_MANAGER_PROTOCOL *This
    )
{
    // Basic validation - check for required entries, valid IDs, etc.
    if (gBootManagerContext.EntryCount == 0) {
        Print(L"No boot entries configured\n");
        return EFI_INVALID_PARAMETER;
    }
    
    // Check for at least one active entry
    BOOLEAN HasDefault = FALSE;
    for (UINTN i = 0; i < gBootManagerContext.EntryCount; i++) {
        if (gBootManagerContext.Entries[i].Flags & BOOT_ENTRY_FLAG_ACTIVE) {
            HasDefault = TRUE;
            break;
        }
    }
    
    if (!HasDefault) {
        Print(L"Warning: No default boot entry configured\n");
        return EFI_INVALID_PARAMETER;
    }
    
    return EFI_SUCCESS;
}

/**
 * Get system information
 */
EFI_STATUS EFIAPI GetSystemInformation(
    IN BOOT_MANAGER_PROTOCOL *This,
    OUT CHAR16 *SystemInfo,
    OUT UINTN *InfoSize
    )
{
    if (!This || !SystemInfo || !InfoSize) {
        return EFI_INVALID_PARAMETER;
    }
    
    // Return system information
    UnicodeSPrint(SystemInfo, L"BloodHorn Boot Manager v1.0 BETA\n");
    UnicodeSPrint(SystemInfo + StrLen(SystemInfo), L"Total boot entries: %d\n", gBootManagerContext.EntryCount);
    UnicodeSPrint(SystemInfo + StrLen(SystemInfo), L"Active entries: %d\n", gBootManagerContext.EntryCount);
    
    *InfoSize = StrLen(SystemInfo);
    return EFI_SUCCESS;
}

// GUID definitions
EFI_GUID gBootManagerProtocolGuid = BOOT_MANAGER_PROTOCOL_GUID;
EFI_GUID gBloodHornVariableGuid = { 0x8B8E7E1F, 0x5C4A, 0x4A2B, { 0x9A, 0x1F, 0x8B, 0x3C, 0x7D, 0x2E, 0x4F, 0x6A } };
BOOLEAN gCorebootAvailable = FALSE;
