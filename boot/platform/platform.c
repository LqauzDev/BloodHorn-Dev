/*
 * platform.c - Unified platform abstraction layer implementation for BloodHorn
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "platform.h"
#include "uboot.h"
#include "openfirmware.h"
#include "../libb/include/bloodhorn/bootinfo.h"
#include "../Arch32/powerpc.h"

// Platform manager instance
struct platform_manager platform_mgr = {0};

// Forward declarations for platform operations
static bh_status_t uboot_platform_detect(void);
static bh_status_t uboot_platform_initialize(bh_boot_info_t* boot_info);
static bh_status_t uboot_platform_cleanup(void);
static void uboot_platform_putc(char c);
static void uboot_platform_puts(const char* s);
static int uboot_platform_getc(void);
static int uboot_platform_tstc(void);
static bh_status_t uboot_platform_get_memory_map(bh_memory_map_t* memory_map);
static void* uboot_platform_malloc(size_t size);
static void uboot_platform_free(void* ptr);
static void* uboot_platform_realloc(void* ptr, size_t size);
static bh_status_t uboot_platform_get_device_tree(void** fdt, size_t* size);
static bh_status_t uboot_platform_get_property(const char* node_path, const char* prop_name, 
                                                void* buffer, size_t* size);
static bh_status_t uboot_platform_set_property(const char* node_path, const char* prop_name, 
                                                const void* value, size_t size);
static bh_status_t uboot_platform_get_command_line(char** cmdline, size_t* size);
static bh_status_t uboot_platform_get_initrd_info(void** initrd, size_t* size);
static bh_status_t uboot_platform_load_kernel(const char* path, void** kernel, size_t* size);
static bh_status_t uboot_platform_boot_kernel(void* kernel, size_t size, const char* cmdline);
static uint64_t uboot_platform_get_time(void);
static void uboot_platform_delay(uint32_t ms);
static void uboot_platform_udelay(uint32_t us);
static void uboot_platform_reset(void);
static void uboot_platform_power_off(void);
static bh_status_t uboot_platform_arch_init(void);
static bh_status_t uboot_platform_arch_cleanup(void);
static bh_status_t uboot_platform_print_info(void);
static bh_status_t uboot_platform_debug_print(const char* format, ...);

static bh_status_t ofw_platform_detect(void);
static bh_status_t ofw_platform_initialize(bh_boot_info_t* boot_info);
static bh_status_t ofw_platform_cleanup(void);
static void ofw_platform_putc(char c);
static void ofw_platform_puts(const char* s);
static int ofw_platform_getc(void);
static int ofw_platform_tstc(void);
static bh_status_t ofw_platform_get_memory_map(bh_memory_map_t* memory_map);
static void* ofw_platform_malloc(size_t size);
static void ofw_platform_free(void* ptr);
static void* ofw_platform_realloc(void* ptr, size_t size);
static bh_status_t ofw_platform_get_device_tree(void** fdt, size_t* size);
static bh_status_t ofw_platform_get_property(const char* node_path, const char* prop_name, 
                                             void* buffer, size_t* size);
static bh_status_t ofw_platform_set_property(const char* node_path, const char* prop_name, 
                                             const void* value, size_t size);
static bh_status_t ofw_platform_get_command_line(char** cmdline, size_t* size);
static bh_status_t ofw_platform_get_initrd_info(void** initrd, size_t* size);
static bh_status_t ofw_platform_load_kernel(const char* path, void** kernel, size_t* size);
static bh_status_t ofw_platform_boot_kernel(void* kernel, size_t size, const char* cmdline);
static uint64_t ofw_platform_get_time(void);
static void ofw_platform_delay(uint32_t ms);
static void ofw_platform_udelay(uint32_t us);
static void ofw_platform_reset(void);
static void ofw_platform_power_off(void);
static bh_status_t ofw_platform_arch_init(void);
static bh_status_t ofw_platform_arch_cleanup(void);
static bh_status_t ofw_platform_print_info(void);
static bh_status_t ofw_platform_debug_print(const char* format, ...);

// U-Boot platform operations
static const struct platform_operations uboot_ops = {
    .detect = uboot_platform_detect,
    .initialize = uboot_platform_initialize,
    .cleanup = uboot_platform_cleanup,
    .putc = uboot_platform_putc,
    .puts = uboot_platform_puts,
    .getc = uboot_platform_getc,
    .tstc = uboot_platform_tstc,
    .get_memory_map = uboot_platform_get_memory_map,
    .malloc = uboot_platform_malloc,
    .free = uboot_platform_free,
    .realloc = uboot_platform_realloc,
    .get_device_tree = uboot_platform_get_device_tree,
    .get_property = uboot_platform_get_property,
    .set_property = uboot_platform_set_property,
    .get_command_line = uboot_platform_get_command_line,
    .get_initrd_info = uboot_platform_get_initrd_info,
    .load_kernel = uboot_platform_load_kernel,
    .boot_kernel = uboot_platform_boot_kernel,
    .get_time = uboot_platform_get_time,
    .delay = uboot_platform_delay,
    .udelay = uboot_platform_udelay,
    .reset = uboot_platform_reset,
    .power_off = uboot_platform_power_off,
    .arch_init = uboot_platform_arch_init,
    .arch_cleanup = uboot_platform_arch_cleanup,
    .print_info = uboot_platform_print_info,
    .debug_print = uboot_platform_debug_print,
};

// OpenFirmware platform operations
static const struct platform_operations ofw_ops = {
    .detect = ofw_platform_detect,
    .initialize = ofw_platform_initialize,
    .cleanup = ofw_platform_cleanup,
    .putc = ofw_platform_putc,
    .puts = ofw_platform_puts,
    .getc = ofw_platform_getc,
    .tstc = ofw_platform_tstc,
    .get_memory_map = ofw_platform_get_memory_map,
    .malloc = ofw_platform_malloc,
    .free = ofw_platform_free,
    .realloc = ofw_platform_realloc,
    .get_device_tree = ofw_platform_get_device_tree,
    .get_property = ofw_platform_get_property,
    .set_property = ofw_platform_set_property,
    .get_command_line = ofw_platform_get_command_line,
    .get_initrd_info = ofw_platform_get_initrd_info,
    .load_kernel = ofw_platform_load_kernel,
    .boot_kernel = ofw_platform_boot_kernel,
    .get_time = ofw_platform_get_time,
    .delay = ofw_platform_delay,
    .udelay = ofw_platform_udelay,
    .reset = ofw_platform_reset,
    .power_off = ofw_platform_power_off,
    .arch_init = ofw_platform_arch_init,
    .arch_cleanup = ofw_platform_arch_cleanup,
    .print_info = ofw_platform_print_info,
    .debug_print = ofw_platform_debug_print,
};

// Platform descriptors
static struct platform_descriptor uboot_platform = {
    .type = BH_PLATFORM_UBOOT,
    .name = "U-Boot",
    .description = "U-Boot bootloader platform support",
    .ops = &uboot_ops,
    .priority = 100,
};

static struct platform_descriptor ofw_platform = {
    .type = BH_PLATFORM_OPENFIRMWARE,
    .name = "OpenFirmware",
    .description = "IEEE 1275 OpenFirmware platform support",
    .ops = &ofw_ops,
    .priority = 90,
};

// Initialize platform manager
bh_status_t platform_manager_init(void) {
    if (platform_mgr.initialized) {
        return BH_STATUS_SUCCESS;
    }
    
    platform_mgr.registered_platforms = NULL;
    platform_mgr.platform_count = 0;
    platform_mgr.current_platform = NULL;
    platform_mgr.initialized = false;
    
    // Register built-in platforms
    bh_status_t status = platform_register(&uboot_platform);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    status = platform_register(&ofw_platform);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    platform_mgr.initialized = true;
    return BH_STATUS_SUCCESS;
}

// Shutdown platform manager
bh_status_t platform_manager_shutdown(void) {
    if (!platform_mgr.initialized) {
        return BH_STATUS_SUCCESS;
    }
    
    // Cleanup current platform
    if (platform_mgr.current_platform) {
        if (platform_mgr.current_platform->ops->cleanup) {
            platform_mgr.current_platform->ops->cleanup();
        }
        platform_mgr.current_platform = NULL;
    }
    
    // Free registered platforms array
    if (platform_mgr.registered_platforms) {
        platform_free(platform_mgr.registered_platforms);
        platform_mgr.registered_platforms = NULL;
    }
    
    platform_mgr.platform_count = 0;
    platform_mgr.initialized = false;
    
    return BH_STATUS_SUCCESS;
}

// Register a platform
bh_status_t platform_register(const struct platform_descriptor* platform) {
    if (!platform || !platform->ops) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Check if platform is already registered
    for (size_t i = 0; i < platform_mgr.platform_count; i++) {
        if (platform_mgr.registered_platforms[i].type == platform->type) {
            return BH_STATUS_ALREADY_EXISTS;
        }
    }
    
    // Reallocate platforms array
    size_t new_size = (platform_mgr.platform_count + 1) * sizeof(struct platform_descriptor);
    struct platform_descriptor* new_platforms = platform_realloc(platform_mgr.registered_platforms, new_size);
    if (!new_platforms) {
        return BH_STATUS_NO_MEMORY;
    }
    
    platform_mgr.registered_platforms = new_platforms;
    
    // Add new platform
    platform_mgr.registered_platforms[platform_mgr.platform_count] = *platform;
    platform_mgr.platform_count++;
    
    // Sort platforms by priority (highest first)
    for (size_t i = 0; i < platform_mgr.platform_count - 1; i++) {
        for (size_t j = i + 1; j < platform_mgr.platform_count; j++) {
            if (platform_mgr.registered_platforms[j].priority > platform_mgr.registered_platforms[i].priority) {
                struct platform_descriptor temp = platform_mgr.registered_platforms[i];
                platform_mgr.registered_platforms[i] = platform_mgr.registered_platforms[j];
                platform_mgr.registered_platforms[j] = temp;
            }
        }
    }
    
    return BH_STATUS_SUCCESS;
}

// Unregister a platform
bh_status_t platform_unregister(bh_platform_type_t type) {
    if (!platform_mgr.initialized) {
        return BH_STATUS_NOT_INITIALIZED;
    }
    
    // Find platform
    size_t index = SIZE_MAX;
    for (size_t i = 0; i < platform_mgr.platform_count; i++) {
        if (platform_mgr.registered_platforms[i].type == type) {
            index = i;
            break;
        }
    }
    
    if (index == SIZE_MAX) {
        return BH_STATUS_NOT_FOUND;
    }
    
    // If this is the current platform, cleanup first
    if (platform_mgr.current_platform && platform_mgr.current_platform->type == type) {
        if (platform_mgr.current_platform->ops->cleanup) {
            platform_mgr.current_platform->ops->cleanup();
        }
        platform_mgr.current_platform = NULL;
    }
    
    // Remove platform from array
    for (size_t i = index; i < platform_mgr.platform_count - 1; i++) {
        platform_mgr.registered_platforms[i] = platform_mgr.registered_platforms[i + 1];
    }
    
    platform_mgr.platform_count--;
    
    // Reallocate array if needed
    if (platform_mgr.platform_count > 0) {
        size_t new_size = platform_mgr.platform_count * sizeof(struct platform_descriptor);
        struct platform_descriptor* new_platforms = platform_realloc(platform_mgr.registered_platforms, new_size);
        if (new_platforms) {
            platform_mgr.registered_platforms = new_platforms;
        }
    } else {
        platform_free(platform_mgr.registered_platforms);
        platform_mgr.registered_platforms = NULL;
    }
    
    return BH_STATUS_SUCCESS;
}

// Detect all platforms
bh_status_t platform_detect_all(void) {
    if (!platform_mgr.initialized) {
        bh_status_t status = platform_manager_init();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    // Try each platform in priority order
    for (size_t i = 0; i < platform_mgr.platform_count; i++) {
        if (platform_mgr.registered_platforms[i].ops->detect) {
            bh_status_t status = platform_mgr.registered_platforms[i].ops->detect();
            if (status == BH_STATUS_SUCCESS) {
                platform_mgr.current_platform = &platform_mgr.registered_platforms[i];
                return BH_STATUS_SUCCESS;
            }
        }
    }
    
    return BH_STATUS_NOT_FOUND;
}

// Initialize platform
bh_status_t platform_initialize(bh_boot_info_t* boot_info) {
    if (!platform_mgr.initialized) {
        bh_status_t status = platform_manager_init();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    if (!boot_info) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // If no current platform, try to detect one
    if (!platform_mgr.current_platform) {
        bh_status_t status = platform_detect_all();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    // Initialize current platform
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->initialize) {
        return platform_mgr.current_platform->ops->initialize(boot_info);
    }
    
    return BH_STATUS_NOT_AVAILABLE;
}

// Platform cleanup
bh_status_t platform_cleanup(void) {
    if (!platform_mgr.initialized) {
        return BH_STATUS_NOT_INITIALIZED;
    }
    
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->cleanup) {
        bh_status_t status = platform_mgr.current_platform->ops->cleanup();
        platform_mgr.current_platform = NULL;
        return status;
    }
    
    return BH_STATUS_NOT_AVAILABLE;
}

// Query functions
bh_platform_type_t platform_get_type(void) {
    if (platform_mgr.current_platform) {
        return platform_mgr.current_platform->type;
    }
    return BH_PLATFORM_UNKNOWN;
}

const char* platform_get_name(void) {
    if (platform_mgr.current_platform) {
        return platform_mgr.current_platform->name;
    }
    return "Unknown";
}

const struct platform_operations* platform_get_operations(void) {
    if (platform_mgr.current_platform) {
        return platform_mgr.current_platform->ops;
    }
    return NULL;
}

bool platform_is_initialized(void) {
    return platform_mgr.initialized && platform_mgr.current_platform != NULL;
}

// U-Boot platform operation implementations
static bh_status_t uboot_platform_detect(void) {
    return uboot_detect_platform();
}

static bh_status_t uboot_platform_initialize(bh_boot_info_t* boot_info) {
    return uboot_initialize_platform(boot_info);
}

static bh_status_t uboot_platform_cleanup(void) {
    return uboot_arch_cleanup();
}

static void uboot_platform_putc(char c) {
    uboot_putc(c);
}

static void uboot_platform_puts(const char* s) {
    uboot_puts(s);
}

static int uboot_platform_getc(void) {
    return uboot_getc();
}

static int uboot_platform_tstc(void) {
    return uboot_tstc();
}

static bh_status_t uboot_platform_get_memory_map(bh_memory_map_t* memory_map) {
    if (!memory_map) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Get board information
    struct uboot_bdinfo* bd;
    bh_status_t status = uboot_get_bdinfo(&bd);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    // Add main memory region
    bh_memory_range_t memory_range = {0};
    memory_range.start = bd->bi_memstart;
    memory_range.end = bd->bi_memstart + bd->bi_memsize;
    memory_range.type = BH_MEMORY_TYPE_RAM;
    memory_range.flags = BH_MEMORY_FLAG_AVAILABLE;
    
    return bh_memory_map_add_range(memory_map, &memory_range);
}

static void* uboot_platform_malloc(size_t size) {
    return uboot_malloc(size);
}

static void uboot_platform_free(void* ptr) {
    uboot_free(ptr);
}

static void* uboot_platform_realloc(void* ptr, size_t size) {
    return uboot_realloc(ptr, size);
}

static bh_status_t uboot_platform_get_device_tree(void** fdt, size_t* size) {
    return uboot_get_device_tree(fdt, size);
}

static bh_status_t uboot_platform_get_property(const char* node_path, const char* prop_name, 
                                                void* buffer, size_t* size) {
    if (!node_path || !prop_name || !buffer || !size) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    void* fdt;
    size_t fdt_size;
    bh_status_t status = uboot_get_device_tree(&fdt, &fdt_size);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    return uboot_fdt_getprop(fdt, node_path, prop_name, buffer, size);
}

static bh_status_t uboot_platform_set_property(const char* node_path, const char* prop_name, 
                                                const void* value, size_t size) {
    if (!node_path || !prop_name || !value) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    void* fdt;
    size_t fdt_size;
    bh_status_t status = uboot_get_device_tree(&fdt, &fdt_size);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    return uboot_fdt_setprop(fdt, node_path, prop_name, value, size);
}

static bh_status_t uboot_platform_get_command_line(char** cmdline, size_t* size) {
    return uboot_get_command_line(cmdline, size);
}

static bh_status_t uboot_platform_get_initrd_info(void** initrd, size_t* size) {
    return uboot_get_initrd_info(initrd, size);
}

static bh_status_t uboot_platform_load_kernel(const char* path, void** kernel, size_t* size) {
    // This would implement kernel loading from U-Boot filesystem
    // For now, return not implemented
    return BH_STATUS_NOT_IMPLEMENTED;
}

static bh_status_t uboot_platform_boot_kernel(void* kernel, size_t size, const char* cmdline) {
    // This would implement kernel booting using U-Boot bootm command
    // For now, return not implemented
    return BH_STATUS_NOT_IMPLEMENTED;
}

static uint64_t uboot_platform_get_time(void) {
    return uboot_get_timer();
}

static void uboot_platform_delay(uint32_t ms) {
    uboot_mdelay(ms);
}

static void uboot_platform_udelay(uint32_t us) {
    uboot_udelay(us);
}

static void uboot_platform_reset(void) {
    uboot_reset();
}

static void uboot_platform_power_off(void) {
    uboot_power_off();
}

static bh_status_t uboot_platform_arch_init(void) {
    return uboot_arch_init();
}

static bh_status_t uboot_platform_arch_cleanup(void) {
    return uboot_arch_cleanup();
}

static bh_status_t uboot_platform_print_info(void) {
    return uboot_print_info();
}

static bh_status_t uboot_platform_debug_print(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    // Simple implementation - just print to console
    char buffer[512];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    if (len > 0) {
        uboot_puts(buffer);
    }
    
    va_end(args);
    return BH_STATUS_SUCCESS;
}

// OpenFirmware platform operation implementations
static bh_status_t ofw_platform_detect(void) {
    return ofw_detect_platform();
}

static bh_status_t ofw_platform_initialize(bh_boot_info_t* boot_info) {
    return ofw_initialize_platform(boot_info);
}

static bh_status_t ofw_platform_cleanup(void) {
    return ofw_arch_cleanup();
}

static void ofw_platform_putc(char c) {
    ofw_putc(c);
}

static void ofw_platform_puts(const char* s) {
    ofw_puts(s);
}

static int ofw_platform_getc(void) {
    return ofw_getc();
}

static int ofw_platform_tstc(void) {
    return ofw_tstc();
}

static bh_status_t ofw_platform_get_memory_map(bh_memory_map_t* memory_map) {
    if (!memory_map) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Find memory node
    uint32_t memory_phandle;
    bh_status_t status = ofw_find_device(OFW_PATH_MEMORY, &memory_phandle);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    // Get available memory property
    uint32_t memory_regions[64];
    size_t size = sizeof(memory_regions);
    
    status = ofw_get_prop(memory_phandle, "available", memory_regions, &size);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    // Parse memory regions (each region is 2 cells: start, size)
    int region_count = size / (2 * sizeof(uint32_t));
    for (int i = 0; i < region_count; i++) {
        bh_memory_range_t memory_range = {0};
        memory_range.start = memory_regions[i * 2];
        memory_range.end = memory_range.start + memory_regions[i * 2 + 1];
        memory_range.type = BH_MEMORY_TYPE_RAM;
        memory_range.flags = BH_MEMORY_FLAG_AVAILABLE;
        
        status = bh_memory_map_add_range(memory_map, &memory_range);
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    return BH_STATUS_SUCCESS;
}

static void* ofw_platform_malloc(size_t size) {
    // OpenFirmware doesn't provide malloc, use claim memory
    uint64_t virt = 0;
    bh_status_t status = ofw_claim_memory(virt, size, 8); // 8-byte alignment
    if (status == BH_STATUS_SUCCESS) {
        return (void*)virt;
    }
    return NULL;
}

static void ofw_platform_free(void* ptr) {
    // OpenFirmware doesn't provide free, use release memory
    if (ptr) {
        // We would need to track the size to release properly
        // For now, this is a limitation
    }
}

static void* ofw_platform_realloc(void* ptr, size_t size) {
    // Simple realloc implementation
    void* new_ptr = ofw_platform_malloc(size);
    if (new_ptr && ptr) {
        // Copy old data (we don't know the old size, so this is limited)
        // This is a simplified implementation
        ofw_platform_free(ptr);
    }
    return new_ptr;
}

static bh_status_t ofw_platform_get_device_tree(void** fdt, size_t* size) {
    if (!fdt || !size) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // OpenFirmware device tree is the live tree, not a blob
    // We would need to flatten it to return a blob
    // For now, return not implemented
    return BH_STATUS_NOT_IMPLEMENTED;
}

static bh_status_t ofw_platform_get_property(const char* node_path, const char* prop_name, 
                                             void* buffer, size_t* size) {
    if (!node_path || !prop_name || !buffer || !size) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    uint32_t phandle;
    bh_status_t status = ofw_find_device(node_path, &phandle);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    return ofw_get_prop(phandle, prop_name, buffer, size);
}

static bh_status_t ofw_platform_set_property(const char* node_path, const char* prop_name, 
                                             const void* value, size_t size) {
    if (!node_path || !prop_name || !value) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    uint32_t phandle;
    bh_status_t status = ofw_find_device(node_path, &phandle);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    return ofw_set_prop(phandle, prop_name, value, size);
}

static bh_status_t ofw_platform_get_command_line(char** cmdline, size_t* size) {
    return ofw_get_command_line(cmdline, size);
}

static bh_status_t ofw_platform_get_initrd_info(void** initrd, size_t* size) {
    uint64_t initrd_start, initrd_end;
    bh_status_t status = ofw_get_initrd_info(&initrd_start, &initrd_end);
    if (status == BH_STATUS_SUCCESS) {
        *initrd = (void*)initrd_start;
        *size = initrd_end - initrd_start;
    }
    return status;
}

static bh_status_t ofw_platform_load_kernel(const char* path, void** kernel, size_t* size) {
    // This would implement kernel loading from OpenFirmware filesystem
    return BH_STATUS_NOT_IMPLEMENTED;
}

static bh_status_t ofw_platform_boot_kernel(void* kernel, size_t size, const char* cmdline) {
    // This would implement kernel booting using OpenFirmware boot method
    return BH_STATUS_NOT_IMPLEMENTED;
}

static uint64_t ofw_platform_get_time(void) {
    uint32_t ticks;
    bh_status_t status = ofw_get_ticks(&ticks);
    if (status == BH_STATUS_SUCCESS) {
        return ticks;
    }
    return 0;
}

static void ofw_platform_delay(uint32_t ms) {
    ofw_delay(ms);
}

static void ofw_platform_udelay(uint32_t us) {
    ofw_delay(us / 1000);
}

static void ofw_platform_reset(void) {
    ofw_reset();
}

static void ofw_platform_power_off(void) {
    ofw_power_off();
}

static bh_status_t ofw_platform_arch_init(void) {
    return ofw_arch_init();
}

static bh_status_t ofw_platform_arch_cleanup(void) {
    return ofw_arch_cleanup();
}

static bh_status_t ofw_platform_print_info(void) {
    ofw_puts("BloodHorn OpenFirmware Platform Information\r\n");
    ofw_puts("========================================\r\n");
    ofw_puts("OpenFirmware detected and initialized\r\n");
    return BH_STATUS_SUCCESS;
}

static bh_status_t ofw_platform_debug_print(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    // Simple implementation - just print to console
    char buffer[512];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    if (len > 0) {
        ofw_puts(buffer);
    }
    
    va_end(args);
    return BH_STATUS_SUCCESS;
}
