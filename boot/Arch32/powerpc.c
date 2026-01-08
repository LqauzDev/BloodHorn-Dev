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
#include "../platform/platform.h"
#include "../platform/devicetree.h"

// CPU information structure
static struct ppc_cpu_info cpu_info;

// MMU context
static struct ppc_mmu_context mmu_ctx;

// Platform-specific data
static bool platform_detected = false;
static bh_platform_type_t current_platform = BH_PLATFORM_UNKNOWN;
static struct device_tree_node* dt_root = NULL;
static void* device_tree_blob = NULL;
static size_t device_tree_size = 0;

// Boot information from platform
static char* platform_cmdline = NULL;
static size_t platform_cmdline_size = 0;
static void* platform_initrd = NULL;
static size_t platform_initrd_size = 0;

// Platform detection and initialization
static bh_status_t ppc_detect_platform(void) {
    if (platform_detected) {
        return BH_STATUS_SUCCESS;
    }
    
    // Initialize platform manager
    bh_status_t status = platform_manager_init();
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    // Detect platform
    status = platform_detect_all();
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    current_platform = platform_get_type();
    platform_detected = true;
    
    return BH_STATUS_SUCCESS;
}

static bh_status_t ppc_initialize_platform(void) {
    if (!platform_detected) {
        bh_status_t status = ppc_detect_platform();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    // Initialize platform
    bh_boot_info_t boot_info = {0};
    bh_status_t status = platform_initialize(&boot_info);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    // Get device tree from platform
    status = platform_get_device_tree(&device_tree_blob, &device_tree_size);
    if (status == BH_STATUS_SUCCESS && device_tree_blob) {
        // Parse device tree
        status = dt_load_from_blob(device_tree_blob, device_tree_size, &dt_root);
        if (status != BH_STATUS_SUCCESS) {
            // Try to load from platform directly
            status = dt_load_from_platform(&device_tree_blob, &device_tree_size);
            if (status == BH_STATUS_SUCCESS) {
                status = dt_load_from_blob(device_tree_blob, device_tree_size, &dt_root);
            }
        }
    }
    
    // Get command line from platform
    status = platform_get_command_line(&platform_cmdline, &platform_cmdline_size);
    if (status != BH_STATUS_SUCCESS) {
        platform_cmdline = NULL;
        platform_cmdline_size = 0;
    }
    
    // Get initrd information from platform
    status = platform_get_initrd_info(&platform_initrd, &platform_initrd_size);
    if (status != BH_STATUS_SUCCESS) {
        platform_initrd = NULL;
        platform_initrd_size = 0;
    }
    
    return BH_STATUS_SUCCESS;
}

static bh_status_t ppc_setup_platform_memory(void) {
    if (!platform_detected) {
        bh_status_t status = ppc_initialize_platform();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    // Get memory map from device tree if available
    if (dt_root) {
        struct dt_memory_region* regions;
        size_t count;
        bh_status_t status = dt_get_memory_regions(dt_root, &regions, &count);
        if (status == BH_STATUS_SUCCESS && regions) {
            // Use device tree memory regions
            // This would update the MMU setup based on device tree
            for (size_t i = 0; i < count; i++) {
                // Configure memory regions from device tree
                // This is a simplified implementation
                (void)regions[i]; // Suppress unused variable warning
            }
            platform_free(regions);
            return BH_STATUS_SUCCESS;
        }
    }
    
    // Fallback to platform memory map
    bh_memory_map_t memory_map = {0};
    bh_status_t status = platform_get_memory_map(&memory_map);
    if (status == BH_STATUS_SUCCESS) {
        // Use platform memory map
        // This would update the MMU setup based on platform info
        return BH_STATUS_SUCCESS;
    }
    
    return BH_STATUS_NOT_FOUND;
}

static bh_status_t ppc_setup_platform_console(void) {
    if (!platform_detected) {
        bh_status_t status = ppc_initialize_platform();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    // Platform console is already initialized during platform initialization
    return BH_STATUS_SUCCESS;
}

static bh_status_t ppc_setup_platform_timers(void) {
    if (!platform_detected) {
        bh_status_t status = ppc_initialize_platform();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    // Use platform time functions
    uint64_t time = platform_get_time();
    if (time != 0) {
        // Platform provides time, use it
        return BH_STATUS_SUCCESS;
    }
    
    // Fallback to PowerPC timebase
    return BH_STATUS_SUCCESS;
}

static bh_status_t ppc_parse_platform_cpu_info(void) {
    if (!dt_root) {
        return BH_STATUS_NOT_FOUND;
    }
    
    // Find CPU nodes
    struct device_tree_node* cpu_node;
    bh_status_t status = dt_find_node_by_compatible(dt_root, "cpu", &cpu_node);
    if (status != BH_STATUS_SUCCESS) {
        // Try alternative compatible strings
        const char* cpu_compatibles[] = {
            "ibm,powerpc-cpu",
            "fsl,powerpc-cpu",
            "cpu",
            NULL
        };
        
        for (int i = 0; cpu_compatibles[i]; i++) {
            status = dt_find_node_by_compatible(dt_root, cpu_compatibles[i], &cpu_node);
            if (status == BH_STATUS_SUCCESS) {
                break;
            }
        }
        
        if (status != BH_STATUS_SUCCESS) {
            return BH_STATUS_NOT_FOUND;
        }
    }
    
    // Get CPU properties
    uint32_t clock_frequency = 0;
    dt_get_u32_property(cpu_node, "clock-frequency", &clock_frequency);
    
    uint32_t timebase_frequency = 0;
    dt_get_u32_property(cpu_node, "timebase-frequency", &timebase_frequency);
    
    char* model = NULL;
    dt_get_string_property(cpu_node, "model", &model);
    if (model) {
        // Update CPU info with device tree data
        // This would enhance the CPU detection
        platform_free(model);
    }
    
    return BH_STATUS_SUCCESS;
}

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
    
    // Enhance with platform information
    ppc_parse_platform_cpu_info();
}

// Initialize MMU with proper page tables
static void ppc_init_mmu(void) {
    // Try to use platform memory information first
    bh_status_t status = ppc_setup_platform_memory();
    if (status == BH_STATUS_SUCCESS) {
        // Platform provided memory map, use it
        return;
    }
    
    // Fallback to default MMU setup
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
    // Try to use platform timer information
    bh_status_t status = ppc_setup_platform_timers();
    if (status == BH_STATUS_SUCCESS) {
        // Platform provided timer setup
        return;
    }
    
    // Fallback to default timer setup
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
static void ppc_init_platform_hardware(void) {
    // Initialize platform first
    bh_status_t status = ppc_initialize_platform();
    if (status != BH_STATUS_SUCCESS) {
        // Platform initialization failed, use defaults
        return;
    }
    
    // Use device tree for hardware configuration if available
    if (dt_root) {
        // Configure hardware based on device tree
        struct device_tree_node* soc_node;
        status = dt_find_node_by_compatible(dt_root, "simple-bus", &soc_node);
        if (status == BH_STATUS_SUCCESS) {
            // Found SoC node, configure devices from device tree
            // This would configure UART, interrupt controller, etc.
        }
        
        // Find and configure UART from device tree
        struct device_tree_node* uart_node;
        status = dt_find_node_by_compatible(dt_root, "ns16550", &uart_node);
        if (status != BH_STATUS_SUCCESS) {
            // Try other compatible strings
            const char* uart_compatibles[] = {
                "serial",
                "console",
                NULL
            };
            
            for (int i = 0; uart_compatibles[i]; i++) {
                status = dt_find_node_by_compatible(dt_root, uart_compatibles[i], &uart_node);
                if (status == BH_STATUS_SUCCESS) {
                    break;
                }
            }
        }
        
        if (status == BH_STATUS_SUCCESS) {
            // Configure UART from device tree
            uint64_t reg_address = 0;
            uint32_t reg_size = 0;
            
            void* reg_value;
            size_t reg_length;
            status = dt_get_property(uart_node, "reg", &reg_value, &reg_length);
            if (status == BH_STATUS_SUCCESS && reg_length >= 16) {
                uint32_t* reg_data = (uint32_t*)reg_value;
                reg_address = ((uint64_t)dt_be32_to_cpu(reg_data[0]) << 32) | dt_be32_to_cpu(reg_data[1]);
                reg_size = ((uint64_t)dt_be32_to_cpu(reg_data[2]) << 32) | dt_be32_to_cpu(reg_data[3]);
                
                // Configure UART at device tree address
                volatile uint32_t* uart = (volatile uint32_t*)reg_address;
                
                // 16550 UART initialization sequence
                *uart = 0x80;           // Enable DLAB
                *uart = 1;              // Divisor LSB (115200 baud)
                *(uart + 1) = 0;        // Divisor MSB
                *uart = 0x03;           // 8N1
                *(uart + 3) = 0x03;     // Enable FIFO, clear them
                *(uart + 1) = 0x01;     // Enable receive data available interrupt
            }
        }
        
        return;
    }
    
    // Fallback to platform-specific hardware initialization
    // Detect platform type from PVR
    uint32_t pvr = mfspr(SPR_PVR);
    uint32_t platform_id = pvr >> 16;

    // Common UART initialization for 16550/NS16550 compatible UARTs
    volatile uint32_t* uart = (volatile uint32_t*)0x800001F8;  // Default UART address
    
    // Configure UART based on platform
    switch (platform_id) {
        case 0x0039:  // PPC440
            uart = (volatile uint32_t*)0xEF600300;  // PPC440 UART0
            break;
        case 0x0040:  // PPC405
            uart = (volatile uint32_t*)0xEF600300;  // PPC405 UART0
            break;
        case 0x004A:  // PPC460
            uart = (volatile uint32_t*)0x4EF600300; // PPC460 UART0
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
            volatile uint32_t* uic_base = (volatile uint32_t*)0xEF600700;
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
            uic_base = (volatile uint32_t*)0xEF600800;
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
            volatile uint32_t* mem_ctrl = (volatile uint32_t*)0x4F800000;
            mem_ctrl[0x00/4] = 0x80808080;  // SDRAM0_CFG0
            mem_ctrl[0x10/4] = 0x00000000;  // SDRAM0_CFG1
            mem_ctrl[0x20/4] = 0x00000000;  // SDRAM0_CFG2
            break;
            
        case 0x0040:  // PPC405
            // Configure memory controller for PPC405
            mem_ctrl = (volatile uint32_t*)0x4F800000;
            mem_ctrl[0x00/4] = 0x00000000;  // SDRAM0_CFG0
            mem_ctrl[0x10/4] = 0x00000000;  // SDRAM0_CFG1
            break;
    }
    
    // PCI/PCIe controller initialization if present
    if (platform_id == 0x004A) {  // PPC460 has PCIe
        volatile uint32_t* pcie_cfg = (volatile uint32_t*)0xA0000000;
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
    
    // Initialize platform first
    bh_status_t status = ppc_initialize_platform();
    if (status != BH_STATUS_SUCCESS) {
        // Platform initialization failed, continue with defaults
    }
    
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
    ppc_init_platform_hardware();
    
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
    // Use platform console if available
    if (platform_detected && platform_is_initialized()) {
        platform_putc(c);
        return;
    }
    
    // Fallback to direct UART access
    volatile uint32_t *uart = (volatile uint32_t *)0x800001F8;
    while (!(*uart & 0x20)); // Wait for THRE
    *uart = c;
    if (c == '\n') ppc_putc('\r');
}

void ppc_puts(const char *s) {
    // Use platform console if available
    if (platform_detected && platform_is_initialized()) {
        platform_puts(s);
        return;
    }
    
    // Fallback to direct UART access
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
    // Use platform power off if available
    if (platform_detected && platform_is_initialized()) {
        platform_power_off();
        return;
    }
    
    // Fallback to platform-specific power off
    for(;;) asm volatile("wait");
}

void ppc_reset(void) {
    // Use platform reset if available
    if (platform_detected && platform_is_initialized()) {
        platform_reset();
        return;
    }
    
    // Fallback to platform-specific reset
    for(;;) asm volatile("wait");
}
