/*
 * powerpc.S - PowerPC (32/64-bit) entry point and low-level routines
 * 
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for more details.
 */

#include "powerpc.h"

// Define section attributes
.section ".text.boot"
.align 3
.global _start

// Entry point
_start:
    // Save the boot parameters
    mflr    %r31        // Save return address
    mr      %r30, %r3   // Save device tree pointer (r3)
    
    // Set up initial stack
    lis     %r1, _stack_top@h
    ori     %r1, %r1, _stack_top@l
    addi    %r1, %r1, -16 // Initial stack frame
    
    // Clear BSS section
    lis     %r3, _bss_start@h
    ori     %r3, %r3, _bss_start@l
    lis     %r4, _bss_end@h
    ori     %r4, %r4, _bss_end@l
    li      %r5, 0
    subf    %r4, %r3, %r4
    bl      memset
    
    // Call C entry point
    mr      %r3, %r30   // Pass device tree pointer
    bl      ppc_main
    
    // Should never return
    b       .

// Exception handlers
.section ".text.exceptions"
.align 4

// System Reset Exception
.global _ppc_exc_system_reset
_ppc_exc_system_reset:
    // Save registers
    stwu    %r1, -256(%r1)
    // Save all GPRs
    stw     %r0, 0x00(%r1)
    stw     %r2, 0x04(%r1)
    // ... save other registers ...
    
    // Call C handler
    bl      ppc_handle_system_reset
    
    // Restore registers and return
    // ... restore registers ...
    addi    %r1, %r1, 256
    rfi

// Machine Check Exception
.global _ppc_exc_machine_check
_ppc_exc_machine_check:
    // Similar to above, but for machine check
    b       .

// Data Storage Interrupt
.global _ppc_exc_dsi
_ppc_exc_dsi:
    // Handle DSI
    b       .

// Instruction Storage Interrupt
.global _ppc_exc_isi
_ppc_exc_isi:
    // Handle ISI
    b       .

// External Interrupt
.global _ppc_exc_external
_ppc_exc_external:
    // Handle external interrupt
    b       .

// Utility functions
.section ".text"

// Enable interrupts
global ppc_enable_interrupts
ppc_enable_interrupts:
    mfmsr   %r3
    ori     %r3, %r3, 0x8000  // Set EE bit
    mtmsr   %r3
    isync
    blr

// Disable interrupts
global ppc_disable_interrupts
ppc_disable_interrupts:
    mfmsr   %r3
    li      %r4, 0x8000
    andc    %r3, %r3, %r4     // Clear EE bit
    mtmsr   %r3
    isync
    blr

// Get MSR
global ppc_get_msr
ppc_get_msr:
    mfmsr   %r3
    blr

// Set MSR
global ppc_set_msr
ppc_set_msr:
    mtmsr   %r3
    isync
    blr

// Synchronize
global ppc_sync
ppc_sync:
    sync
    blr

// Instruction synchronize
global ppc_isync
ppc_isync:
    isync
    blr

// TLB invalidate entry
global ppc_tlbie
ppc_tlbie:
    tlbie   %r3
    sync
    blr

// TLB sync
global ppc_tlbsync
ppc_tlbsync:
    eieio
    tlbsync
    sync
    blr

// Cache operations
global ppc_icbi
ppc_icbi:
    icbi    0, %r3
    isync
    blr

global ppc_dcbf
ppc_dcbf:
    dcbf    0, %r3
    sync
    blr

global ppc_dcbst
ppc_dcbst:
    dcbst   0, %r3
    sync
    blr

global ppc_dcbz
ppc_dcbz:
    dcbz    0, %r3
    blr

// Memory copy (r3=dest, r4=src, r5=size)
global memcpy
memcpy:
    cmpwi   %r5, 0
    beqlr
    mtctr   %r5
    addi    %r6, %r3, -1
    addi    %r4, %r4, -1
1:  lbzu    %r0, 1(%r4)
    stbu    %r0, 1(%r6)
    bdnz    1b
    blr

// Memory set (r3=ptr, r4=value, r5=size)
global memset
memset:
    cmpwi   %r5, 0
    beqlr
    mtctr   %r5
    addi    %r6, %r3, -1
1:  stbu    %r4, 1(%r6)
    bdnz    1b
    blr
