/*
 * uboot.c - U-Boot platform support implementation for BloodHorn
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "uboot.h"
#include "../libb/include/bloodhorn/bootinfo.h"
#include "../Arch32/powerpc.h"

// Global U-Boot data pointers
static struct uboot_global_data* gd = NULL;
static struct uboot_bdinfo* bd = NULL;
static void* fdt_blob = NULL;
static size_t fdt_size = 0;
static bool uboot_detected = false;

// U-Boot global data register locations (PowerPC specific)
#define UBOOT_GD_REGISTER  0x00000000  // Typically stored in a known register
#define UBOOT_MAGIC_ADDR   0x00000000  // Magic number location

// External functions (provided by U-Boot or platform)
extern struct uboot_global_data* uboot_get_global_data_ptr(void);
extern void uboot_console_init(void);
extern void uboot_serial_putc(char c);
extern int uboot_serial_getc(void);
extern int uboot_serial_tstc(void);

// Platform detection
bh_status_t uboot_detect_platform(void) {
    // Check for U-Boot signature in known locations
    volatile uint32_t* magic_ptr = (volatile uint32_t*)UBOOT_MAGIC_ADDR;
    
    // Look for U-Boot magic number
    if (*magic_ptr == 0x55424F4F) {  // "UBOO"
        uboot_detected = true;
        
        // Try to get global data pointer
        gd = uboot_get_global_data_ptr();
        if (gd && gd->flags & 0x01) {  // Check if global data is valid
            bd = (struct uboot_bdinfo*)gd->bd;
            if (gd->fdt_addr && gd->fdt_size) {
                fdt_blob = (void*)gd->fdt_addr;
                fdt_size = gd->fdt_size;
            }
            return BH_STATUS_SUCCESS;
        }
    }
    
    // Alternative detection: check for U-Boot environment
    if (gd && gd->env_valid) {
        uboot_detected = true;
        return BH_STATUS_SUCCESS;
    }
    
    return BH_STATUS_NOT_FOUND;
}

// Initialize U-Boot platform
bh_status_t uboot_initialize_platform(bh_boot_info_t* boot_info) {
    if (!uboot_detected) {
        bh_status_t status = uboot_detect_platform();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    if (!boot_info) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Set platform type
    boot_info->platform.type = BH_PLATFORM_UBOOT;
    boot_info->platform.arch = BH_ARCH_POWERPC;
    
    // Copy platform information
    if (gd) {
        snprintf(boot_info->platform.platform_name, sizeof(boot_info->platform.platform_name),
                "U-Boot %d.%d", (gd->flags >> 16) & 0xFF, gd->flags & 0xFF);
        strcpy(boot_info->platform.firmware_vendor, "U-Boot");
        snprintf(boot_info->platform.firmware_version, sizeof(boot_info->platform.firmware_version),
                "%d.%d", (gd->flags >> 16) & 0xFF, gd->flags & 0xFF);
    }
    
    // Setup memory map
    bh_status_t status = uboot_setup_memory_map(boot_info);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    // Setup console
    uboot_setup_console();
    
    // Parse boot information if available
    struct uboot_boot_info* uboot_info = (struct uboot_boot_info*)gd->jt;
    if (uboot_info && uboot_info->magic == 0x55424F4F) {
        status = uboot_parse_boot_info(uboot_info, boot_info);
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    return BH_STATUS_SUCCESS;
}

// Parse U-Boot boot information
bh_status_t uboot_parse_boot_info(const struct uboot_boot_info* uboot_info, bh_boot_info_t* boot_info) {
    if (!uboot_info || !boot_info) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Validate magic number
    if (uboot_info->magic != 0x55424F4F) {
        return BH_STATUS_INVALID_DATA;
    }
    
    // Check version compatibility
    if (uboot_info->version < UBOOT_VERSION_MIN) {
        return BH_STATUS_UNSUPPORTED;
    }
    
    // Process command line
    if (uboot_info->flags & UBOOT_FLAG_CMDLINE_VALID && uboot_info->cmdline_addr) {
        char* cmdline = (char*)uboot_info->cmdline_addr;
        if (cmdline && uboot_info->cmdline_size > 0) {
            boot_info->command_line = cmdline;
            BH_BOOTINFO_SET_FLAG(boot_info, BH_BOOT_FLAG_CMDLINE_PRESENT);
        }
    }
    
    // Process initrd
    if (uboot_info->flags & UBOOT_FLAG_INITRD_VALID && uboot_info->initrd_addr) {
        bh_module_info_t initrd_module = {0};
        initrd_module.type = BH_MODULE_TYPE_INITRD;
        initrd_module.start = uboot_info->initrd_addr;
        initrd_module.end = uboot_info->initrd_addr + uboot_info->initrd_size;
        initrd_module.size = uboot_info->initrd_size;
        strcpy(initrd_module.name, "initrd");
        
        bh_status_t status = bh_bootinfo_add_module(boot_info, &initrd_module);
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
        
        BH_BOOTINFO_SET_FLAG(boot_info, BH_BOOT_FLAG_MODULES_PRESENT);
    }
    
    // Process device tree
    if (uboot_info->flags & UBOOT_FLAG_FDT_VALID && uboot_info->device_tree_addr) {
        fdt_blob = (void*)uboot_info->device_tree_addr;
        fdt_size = uboot_info->device_tree_size;
    }
    
    // Check for 64-bit flag
    if (uboot_info->flags & UBOOT_FLAG_64BIT) {
        boot_info->platform.arch = BH_ARCH_POWERPC64;
    }
    
    return BH_STATUS_SUCCESS;
}

// Get U-Boot global data
bh_status_t uboot_get_global_data(struct uboot_global_data** global_data) {
    if (!global_data) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    if (!uboot_detected) {
        bh_status_t status = uboot_detect_platform();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    *global_data = gd;
    return gd ? BH_STATUS_SUCCESS : BH_STATUS_NOT_FOUND;
}

// Get board information
bh_status_t uboot_get_bdinfo(struct uboot_bdinfo** board_info) {
    if (!board_info) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    if (!uboot_detected) {
        bh_status_t status = uboot_detect_platform();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    *board_info = bd;
    return bd ? BH_STATUS_SUCCESS : BH_STATUS_NOT_FOUND;
}

// Get device tree
bh_status_t uboot_get_device_tree(void** fdt, size_t* size) {
    if (!fdt || !size) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    if (!uboot_detected) {
        bh_status_t status = uboot_detect_platform();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    *fdt = fdt_blob;
    *size = fdt_size;
    
    return fdt_blob ? BH_STATUS_SUCCESS : BH_STATUS_NOT_FOUND;
}

// Get command line
bh_status_t uboot_get_command_line(char** cmdline, size_t* size) {
    if (!cmdline || !size) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    if (!bd) {
        bh_status_t status = uboot_get_bdinfo(&bd);
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    *cmdline = bd->bi_cmdline;
    *size = strlen(bd->bi_cmdline);
    
    return BH_STATUS_SUCCESS;
}

// Get initrd information
bh_status_t uboot_get_initrd(void** initrd, size_t* size) {
    if (!initrd || !size) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Initrd location varies by platform, check common locations
    // This is a simplified implementation
    *initrd = NULL;
    *size = 0;
    
    return BH_STATUS_NOT_IMPLEMENTED;
}

// Setup memory map for U-Boot
bh_status_t uboot_setup_memory_map(bh_boot_info_t* boot_info) {
    if (!boot_info || !bd) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Add main memory region
    bh_memory_range_t memory_range = {0};
    memory_range.start = bd->bi_memstart;
    memory_range.end = bd->bi_memstart + bd->bi_memsize;
    memory_range.type = BH_MEMORY_TYPE_RAM;
    memory_range.flags = BH_MEMORY_FLAG_AVAILABLE;
    
    bh_status_t status = bh_memory_map_add_range(&boot_info->memory_map, &memory_range);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    // Add flash memory if present
    if (bd->bi_flashsize > 0) {
        memory_range.start = bd->bi_flashstart;
        memory_range.end = bd->bi_flashstart + bd->bi_flashsize;
        memory_range.type = BH_MEMORY_TYPE_FLASH;
        memory_range.flags = BH_MEMORY_FLAG_READONLY;
        
        status = bh_memory_map_add_range(&boot_info->memory_map, &memory_range);
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    // Add SRAM if present
    if (bd->bi_sramsize > 0) {
        memory_range.start = bd->bi_sramstart;
        memory_range.end = bd->bi_sramstart + bd->bi_sramsize;
        memory_range.type = BH_MEMORY_TYPE_SRAM;
        memory_range.flags = BH_MEMORY_FLAG_AVAILABLE;
        
        status = bh_memory_map_add_range(&boot_info->memory_map, &memory_range);
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    return BH_STATUS_SUCCESS;
}

// Setup console
bh_status_t uboot_setup_console(void) {
    if (gd && gd->have_console) {
        uboot_console_init();
        return BH_STATUS_SUCCESS;
    }
    return BH_STATUS_NOT_AVAILABLE;
}

// Print U-Boot information
bh_status_t uboot_print_info(void) {
    if (!uboot_detected) {
        return BH_STATUS_NOT_FOUND;
    }
    
    uboot_puts("BloodHorn U-Boot Platform Information\r\n");
    uboot_puts("=====================================\r\n");
    
    if (gd) {
        uboot_puts("U-Boot detected\r\n");
        uboot_puts("Console available: ");
        uboot_puts(gd->have_console ? "Yes\r\n" : "No\r\n");
        uboot_puts("Environment valid: ");
        uboot_puts(gd->env_valid ? "Yes\r\n" : "No\r\n");
        
        if (gd->fdt_addr) {
            uboot_puts("Device tree: ");
            uboot_puts("Available\r\n");
        }
    }
    
    if (bd) {
        uboot_puts("Memory: ");
        // Simple hex print (would need proper conversion)
        uboot_puts("Available\r\n");
        
        if (bd->bi_cmdline[0]) {
            uboot_puts("Command line: ");
            uboot_puts(bd->bi_cmdline);
            uboot_puts("\r\n");
        }
    }
    
    return BH_STATUS_SUCCESS;
}

// Console I/O functions
void uboot_putc(char c) {
    if (gd && gd->have_console) {
        uboot_serial_putc(c);
    }
}

void uboot_puts(const char* s) {
    while (*s) {
        uboot_putc(*s++);
    }
}

int uboot_getc(void) {
    if (gd && gd->have_console) {
        return uboot_serial_getc();
    }
    return -1;
}

int uboot_tstc(void) {
    if (gd && gd->have_console) {
        return uboot_serial_tstc();
    }
    return 0;
}

// Memory allocation (simplified - uses U-Boot's malloc if available)
void* uboot_malloc(size_t size) {
    // This would interface with U-Boot's malloc implementation
    // For now, return NULL to indicate not implemented
    return NULL;
}

void uboot_free(void* ptr) {
    // Interface with U-Boot's free implementation
}

void* uboot_realloc(void* ptr, size_t size) {
    // Interface with U-Boot's realloc implementation
    return NULL;
}

// Timing functions
uint64_t uboot_get_timer(void) {
    // Use PowerPC timebase
    return ppc_mftb();
}

void uboot_udelay(unsigned long usec) {
    // Simple delay using timebase
    uint64_t start = ppc_mftb();
    uint64_t ticks = (usec * 512) / 1000000; // Assuming 512MHz timebase
    
    while ((ppc_mftb() - start) < ticks) {
        ppc_ppc_pause();
    }
}

void uboot_mdelay(unsigned long msec) {
    uboot_udelay(msec * 1000);
}

// Architecture-specific initialization
bh_status_t uboot_arch_init(void) {
    // Initialize PowerPC-specific U-Boot features
    return BH_STATUS_SUCCESS;
}

bh_status_t uboot_arch_cleanup(void) {
    // Cleanup PowerPC-specific U-Boot features
    return BH_STATUS_SUCCESS;
}

void uboot_reset(void) {
    // Use PowerPC reset mechanism
    ppc_reset();
}

void uboot_power_off(void) {
    // Use PowerPC power off mechanism
    ppc_power_off();
}
