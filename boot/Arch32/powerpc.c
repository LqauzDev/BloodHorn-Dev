/*
 * PowerPC (32/64-bit) Architecture Support
 * 
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for more details.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "powerpc.h"
#include "../boot.h"

// CPU information structure
static struct ppc_cpu_info cpu_info;

// MMU context
static struct ppc_mmu_context mmu_ctx;

// Exception frame for nested exceptions
static struct ppc_exception_frame *current_frame;

// Initialize CPU features and capabilities
static void ppc_detect_cpu(void) {
    uint32_t pvr = mfspr(SPR_PVR);
    uint32_t pvr_ver = pvr >> 16;
    
    memset(&cpu_info, 0, sizeof(cpu_info));
    cpu_info.pvr = pvr;
    cpu_info.version = pvr_ver;
    cpu_info.revision = pvr & 0xFFFF;
    
    // Detect CPU features
    uint64_t msr = mfmsr();
    cpu_info.is_64bit = !!(msr & MSR_64BIT);
    
    // Check for FPU
    cpu_info.has_fpu = !!(msr & MSR_FP);
    
    // Check for AltiVec/VMX
    if (msr & MSR_VEC) {
        uint32_t vrsave;
        asm volatile("mfspr %0, 256" : "=r"(vrsave));
        cpu_info.has_altivec = (vrsave != 0);
    }
    
    // Check for VSX
    if (msr & MSR_VSX) {
        uint64_t vsx_support;
        asm volatile("mfspr %0, 2" : "=r"(vsx_support));
        cpu_info.has_vsx = !!(vsx_support & (1ULL << 63));
    }
    
    // Detect cache info
    uint32_t l1cfar = 0, l1cfbr = 0;
    asm volatile("mfspr %0, 1017" : "=r"(l1cfar));
    asm volatile("mfspr %0, 1019" : "=r"(l1cfbr));
    
    cpu_info.l1_cache_line_size = 1 << ((l1cfbr >> 16) & 0x1F);
    cpu_info.icache_size = (l1cfar & 0x7FF) * cpu_info.l1_cache_line_size;
    cpu_info.dcache_size = ((l1cfbr >> 24) & 0x7FF) * cpu_info.l1_cache_line_size;
}

// Initialize MMU with proper page tables
static void ppc_init_mmu(void) {
    // Allocate page tables (aligned to 64KB for SDR1)
    static uint64_t pagetable[8192] __attribute__((aligned(65536)));
    
    // Clear page tables
    memset(pagetable, 0, sizeof(pagetable));
    
    // Set up 1:1 mapping for first 4GB with 4KB pages
    for (uint64_t i = 0; i < 1024; i++) {
        uint64_t pte = (i << 22) | PPC_TLB_VALID | PPC_TLB_WRITE | PPC_TLB_EXEC;
        if (i < 16) // First 64MB as cache-inhibited
            pte |= PPC_TLB_GUARD;
        pagetable[i] = pte;
    }
    
    // Configure SDR1 (HTABORG and HTABSIZE)
    uint64_t htaborg = (uint64_t)pagetable;
    uint32_t htabmask = (sizeof(pagetable) / (16 * 1024)) - 1;
    uint64_t sdr1 = (htaborg & ~0xFFFFF) | (htabmask & 0x1F);
    
    // Save MMU context
    mmu_ctx.sdr1 = sdr1;
    mmu_ctx.htaborg = htaborg;
    mmu_ctx.htabmask = htabmask;
    
    // Set up segment registers for 1:1 mapping
    for (int i = 0; i < 16; i++) {
        mmu_ctx.vsid[i] = (uint64_t)i << 28;
        mmu_ctx.sr[i] = (i << 28) | 0x80000000; // VSID | Ks=1
    }
    
    // Initialize VSID multiplier (PowerPC hashing)
    mmu_ctx.vsid_multiplier = 0x9E3779B9; // Golden ratio
}

// Initialize cache
static void ppc_init_cache(void) {
    // Invalidate and enable caches
    uint32_t hid0;
    asm volatile("mfspr %0, 1008" : "=r"(hid0));
    
    // Enable caches if not already enabled
    if (!(hid0 & 0xC0000000)) {
        // Invalidate caches
        asm volatile("iccci 0,0");
        asm volatile("dccci 0,0");
        
        // Enable caches
        hid0 |= 0xC0000000; // ICE | DCE
        asm volatile("mtspr 1008, %0" : : "r"(hid0));
    }
}

// Initialize timers
static void ppc_init_timers(void) {
    // Set up Decrementer
    uint64_t tb_freq = 512000000; // 512MHz typical for PowerPC
    uint32_t decr_val = tb_freq / 100; // 10ms tick
    mtspr(SPR_DEC, decr_val);
    
    // Enable Decrementer interrupt
    uint64_t msr = mfmsr();
    msr |= MSR_EE; // Enable external interrupts
    mtmsr(msr);
}

// Exception handlers
__attribute__((noreturn)) void ppc_handle_system_reset(struct ppc_exception_frame *frame) {
    // Save exception frame
    struct ppc_exception_frame *prev_frame = current_frame;
    current_frame = frame;
    
    // TODO: Handle system reset
    
    // Restore previous frame
    current_frame = prev_frame;
    
    // Should never return
    for(;;) asm volatile("wait");
}

// More exception handlers...

// Initialize exception handling
static void ppc_init_exceptions(void) {
    // Set up IVPR (Interrupt Vector Prefix Register)
    uint64_t ivpr = (uint64_t)&ppc_handle_system_reset & ~0xFFFF;
    mtspr(SPR_IVPR, ivpr);
    
    // Set up IVORs (Interrupt Vector Offset Registers)
    mtspr(SPR_IVOR0, (uint64_t)&ppc_handle_system_reset - ivpr);
    // ... set up other IVORs
    
    // Enable machine check recovery
    uint64_t msr = mfmsr();
    msr |= MSR_RI; // Recoverable interrupt
    mtmsr(msr);
}

// Platform hardware initialization
static void ppc_init_platform(void) {
    // Detect platform type from PVR
    uint32_t pvr = mfspr(SPR_PVR);
    uint32_t platform_id = pvr >> 16;
    
    // Common UART initialization for 16550/NS16550 compatible UARTs
    volatile uint32_t *uart = (volatile uint32_t *)0x800001F8;  // Default UART address
    
    // Configure UART based on platform
    switch (platform_id) {
        case 0x0039:  // PPC440
            uart = (volatile uint32_t *)0xEF600300;  // PPC440 UART0
            break;
        case 0x0040:  // PPC405
            uart = (volatile uint32_t *)0xEF600300;  // PPC405 UART0
            break;
        case 0x004A:  // PPC460
            uart = (volatile uint32_t *)0x4EF600300; // PPC460 UART0
            break;
    }
    
    // 16550 UART initialization sequence
    *uart = 0x80;           // Enable DLAB
    *uart = 1;              // Divisor LSB (115200 baud)
    *(uart + 1) = 0;        // Divisor MSB
    *uart = 0x03;           // 8N1
    *(uart + 3) = 0x03;     // Enable FIFO, clear them
    *(uart + 1) = 0x01;     // Enable receive data available interrupt
    
    // Initialize interrupt controller
    switch (platform_id) {
        case 0x0039:  // PPC440
            // Configure UIC (Universal Interrupt Controller)
            volatile uint32_t *uic_base = (volatile uint32_t *)0xEF600700;
            uic_base[0x10/4] = 0x7FFFFFFF;  // UIC_MSR: Mask all interrupts
            uic_base[0x18/4] = 0x00000000;  // UIC_SR: Clear all pending
            uic_base[0x1C/4] = 0x00000000;  // UIC_ER: Disable all
            uic_base[0x20/4] = 0x00000000;  // UIC_CR: All level-triggered
            uic_base[0x30/4] = 0x00000000;  // UIC_PR: All normal priority
            uic_base[0x38/4] = 0x00000000;  // UIC_VR: No vector numbers
            uic_base[0x1C/4] = 0xFFFFFFFF;  // UIC_ER: Enable all
            break;
            
        case 0x0040:  // PPC405
            // Configure UIC for PPC405
            uic_base = (volatile uint32_t *)0xEF600800;
            uic_base[0x10/4] = 0x7FFFFFFF;
            uic_base[0x18/4] = 0x00000000;
            uic_base[0x1C/4] = 0x00000000;
            uic_base[0x20/4] = 0x00000000;
            uic_base[0x30/4] = 0x00000000;
            uic_base[0x38/4] = 0x00000000;
            uic_base[0x1C/4] = 0xFFFFFFFF;
            break;
    }
    
    // Memory controller initialization
    switch (platform_id) {
        case 0x0039:  // PPC440
            // Configure SDRAM controller
            volatile uint32_t *mem_ctrl = (volatile uint32_t *)0x4F800000;
            mem_ctrl[0x00/4] = 0x80808080;  // SDRAM0_CFG0
            mem_ctrl[0x10/4] = 0x00000000;  // SDRAM0_CFG1
            mem_ctrl[0x20/4] = 0x00000000;  // SDRAM0_CFG2
            break;
            
        case 0x0040:  // PPC405
            // Configure memory controller for PPC405
            mem_ctrl = (volatile uint32_t *)0x4F800000;
            mem_ctrl[0x00/4] = 0x00000000;  // SDRAM0_CFG0
            mem_ctrl[0x10/4] = 0x00000000;  // SDRAM0_CFG1
            break;
    }
    
    // PCI/PCIe controller initialization if present
    if (platform_id == 0x004A) {  // PPC460 has PCIe
        volatile uint32_t *pcie_cfg = (volatile uint32_t *)0xA0000000;
        pcie_cfg[0x00/4] = 0x00000001;  // Enable memory and I/O space
        pcie_cfg[0x04/4] = 0x00000000;  // Clear status
        pcie_cfg[0x08/4] = 0x00000000;  // Disable all interrupts
    }
}

// Main CPU initialization
void ppc_early_init(void) {
    // Disable interrupts
    uint64_t msr = mfmsr();
    mtmsr(msr & ~MSR_EE);
    
    // Detect CPU features
    ppc_detect_cpu();
    
    // Initialize caches
    ppc_init_cache();
    
    // Initialize MMU
    ppc_init_mmu();
    
    // Set up SDR1
    mtspr(SPR_SDR1, mmu_ctx.sdr1);
    
    // Initialize exception handling
    ppc_init_exceptions();
    
    // Initialize timers
    ppc_init_timers();
    
    // Initialize platform hardware
    ppc_init_platform();
    
    // Enable FPU if available
    if (cpu_info.has_fpu) {
        msr = mfmsr();
        msr |= MSR_FP;
        mtmsr(msr);
    }
    
    // Enable AltiVec if available
    if (cpu_info.has_altivec) {
        msr = mfmsr();
        msr |= MSR_VEC;
        mtmsr(msr);
    }
    
    // Enable VSX if available
    if (cpu_info.has_vsx) {
        msr = mfmsr();
        msr |= MSR_VSX;
        mtmsr(msr);
    }
}

// Main entry point from assembly
__attribute__((noreturn)) void ppc_main(void) {
    // Initialize CPU and hardware
    ppc_early_init();
    
    // Print boot message
    const char *msg = "BloodHorn PPC64 Bootloader\r\n";
    for (const char *p = msg; *p; p++)
        ppc_putc(*p);
    
    // TODO: Parse device tree, load kernel, etc.
    
    // Halt CPU if we get here
    for(;;) asm volatile("wait");
}

// Console I/O
void ppc_putc(char c) {
    volatile uint32_t *uart = (volatile uint32_t *)0x800001F8;
    while (!(*uart & 0x20)); // Wait for THRE
    *uart = c;
    if (c == '\n') ppc_putc('\r');
}

void ppc_puts(const char *s) {
    while (*s) ppc_putc(*s++);
}

// Memory management
void *ppc_map_io(void *phys, size_t size, uint32_t flags) {
    // TODO: Implement I/O mapping
    return phys; // 1:1 mapping for now
}

void ppc_unmap_io(void *virt, size_t size) {
    // TODO: Implement I/O unmapping
}

// Power management
void ppc_power_off(void) {
    // Platform-specific power off
    for(;;) asm volatile("wait");
}

void ppc_reset(void) {
    // Platform-specific reset
    for(;;) asm volatile("wait");
}
