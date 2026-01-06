/*
 * BootManagerLib.c
 *
 * Boot Manager Library implementation for BloodHorn Bootloader
 * 
 * This library provides the core functionality for managing boot entries,
 * configuration, and boot Manager Protocol operations.
 * 
 * Copyright (c) 2025 BloodyHell Industries INC
 * Licensed under BSD-2-Clause-Patent License
 */

#include "BootManagerProtocol.h"
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>

// =============================================================================
// INTERNAL STATE
// =============================================================================

static BOOT_MANAGER_PROTOCOL* gBootManagerProtocol = NULL;

// =============================================================================
// BOOT MANAGER LIBRARY IMPLEMENTATION
// =============================================================================

/**
 * Initialize the Boot Manager Library
 */
EFI_STATUS EFIAPI BootManagerLibInitialize(VOID) {
    // Initialize global protocol pointer
    gBootManagerProtocol = NULL;
    
    Print(L"Boot Manager Library initialized\n");
    return EFI_SUCCESS;
}

/**
 * Get the Boot Manager Protocol instance
 */
EFI_STATUS EFIAPI BootManagerLibGetProtocol(
    OUT BOOT_MANAGER_PROTOCOL** Protocol
    )
{
    if (!Protocol) {
        return EFI_INVALID_PARAMETER;
    }
    
    *Protocol = gBootManagerProtocol;
    return EFI_SUCCESS;
}

/**
 * Set the Boot Manager Protocol instance
 */
EFI_STATUS EFIAPI BootManagerLibSetProtocol(
    IN BOOT_MANAGER_PROTOCOL* Protocol
    )
{
    if (!Protocol) {
        return EFI_INVALID_PARAMETER;
    }
    
    gBootManagerProtocol = Protocol;
    return EFI_SUCCESS;
}
