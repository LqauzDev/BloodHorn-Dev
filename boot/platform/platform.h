/*
 * platform.h - Unified platform abstraction layer for BloodHorn
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../libb/include/bloodhorn/bootinfo.h"

// Platform operations structure - unified interface for all platforms
struct platform_operations {
    // Platform detection and initialization
    bh_status_t (*detect)(void);
    bh_status_t (*initialize)(bh_boot_info_t* boot_info);
    bh_status_t (*cleanup)(void);
    
    // Console operations
    void (*putc)(char c);
    void (*puts)(const char* s);
    int (*getc)(void);
    int (*tstc)(void);
    
    // Memory operations
    bh_status_t (*get_memory_map)(bh_memory_map_t* memory_map);
    void* (*malloc)(size_t size);
    void (*free)(void* ptr);
    void* (*realloc)(void* ptr, size_t size);
    
    // Device tree operations
    bh_status_t (*get_device_tree)(void** fdt, size_t* size);
    bh_status_t (*get_property)(const char* node_path, const char* prop_name, 
                                void* buffer, size_t* size);
    bh_status_t (*set_property)(const char* node_path, const char* prop_name, 
                                const void* value, size_t size);
    
    // Boot operations
    bh_status_t (*get_command_line)(char** cmdline, size_t* size);
    bh_status_t (*get_initrd_info)(void** initrd, size_t* size);
    bh_status_t (*load_kernel)(const char* path, void** kernel, size_t* size);
    bh_status_t (*boot_kernel)(void* kernel, size_t size, const char* cmdline);
    
    // Timing operations
    uint64_t (*get_time)(void);
    void (*delay)(uint32_t ms);
    void (*udelay)(uint32_t us);
    
    // Power management
    void (*reset)(void);
    void (*power_off)(void);
    
    // Platform-specific operations
    bh_status_t (*arch_init)(void);
    bh_status_t (*arch_cleanup)(void);
    
    // Debug and information
    bh_status_t (*print_info)(void);
    bh_status_t (*debug_print)(const char* format, ...);
};

// Platform descriptor structure
struct platform_descriptor {
    bh_platform_type_t type;
    const char* name;
    const char* description;
    const struct platform_operations* ops;
    uint32_t priority;  // Higher priority platforms are checked first
};

// Platform manager state
struct platform_manager {
    struct platform_descriptor* current_platform;
    struct platform_descriptor* registered_platforms;
    size_t platform_count;
    bool initialized;
};

// Global platform manager instance
extern struct platform_manager platform_mgr;

// Platform registration
bh_status_t platform_register(const struct platform_descriptor* platform);
bh_status_t platform_unregister(bh_platform_type_t type);

// Platform detection and initialization
bh_status_t platform_detect_all(void);
bh_status_t platform_initialize(bh_boot_info_t* boot_info);
bh_status_t platform_cleanup(void);

// Platform operations (delegated to current platform)
static inline bh_status_t platform_putc(char c) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->putc) {
        platform_mgr.current_platform->ops->putc(c);
        return BH_STATUS_SUCCESS;
    }
    return BH_STATUS_NOT_AVAILABLE;
}

static inline bh_status_t platform_puts(const char* s) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->puts) {
        platform_mgr.current_platform->ops->puts(s);
        return BH_STATUS_SUCCESS;
    }
    return BH_STATUS_NOT_AVAILABLE;
}

static inline int platform_getc(void) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->getc) {
        return platform_mgr.current_platform->ops->getc();
    }
    return -1;
}

static inline int platform_tstc(void) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->tstc) {
        return platform_mgr.current_platform->ops->tstc();
    }
    return 0;
}

static inline bh_status_t platform_get_memory_map(bh_memory_map_t* memory_map) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->get_memory_map) {
        return platform_mgr.current_platform->ops->get_memory_map(memory_map);
    }
    return BH_STATUS_NOT_AVAILABLE;
}

static inline void* platform_malloc(size_t size) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->malloc) {
        return platform_mgr.current_platform->ops->malloc(size);
    }
    return NULL;
}

static inline void platform_free(void* ptr) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->free) {
        platform_mgr.current_platform->ops->free(ptr);
    }
}

static inline void* platform_realloc(void* ptr, size_t size) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->realloc) {
        return platform_mgr.current_platform->ops->realloc(ptr, size);
    }
    return NULL;
}

static inline bh_status_t platform_get_device_tree(void** fdt, size_t* size) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->get_device_tree) {
        return platform_mgr.current_platform->ops->get_device_tree(fdt, size);
    }
    return BH_STATUS_NOT_AVAILABLE;
}

static inline bh_status_t platform_get_property(const char* node_path, const char* prop_name, 
                                                 void* buffer, size_t* size) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->get_property) {
        return platform_mgr.current_platform->ops->get_property(node_path, prop_name, buffer, size);
    }
    return BH_STATUS_NOT_AVAILABLE;
}

static inline bh_status_t platform_set_property(const char* node_path, const char* prop_name, 
                                                 const void* value, size_t size) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->set_property) {
        return platform_mgr.current_platform->ops->set_property(node_path, prop_name, value, size);
    }
    return BH_STATUS_NOT_AVAILABLE;
}

static inline bh_status_t platform_get_command_line(char** cmdline, size_t* size) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->get_command_line) {
        return platform_mgr.current_platform->ops->get_command_line(cmdline, size);
    }
    return BH_STATUS_NOT_AVAILABLE;
}

static inline bh_status_t platform_get_initrd_info(void** initrd, size_t* size) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->get_initrd_info) {
        return platform_mgr.current_platform->ops->get_initrd_info(initrd, size);
    }
    return BH_STATUS_NOT_AVAILABLE;
}

static inline bh_status_t platform_load_kernel(const char* path, void** kernel, size_t* size) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->load_kernel) {
        return platform_mgr.current_platform->ops->load_kernel(path, kernel, size);
    }
    return BH_STATUS_NOT_AVAILABLE;
}

static inline bh_status_t platform_boot_kernel(void* kernel, size_t size, const char* cmdline) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->boot_kernel) {
        return platform_mgr.current_platform->ops->boot_kernel(kernel, size, cmdline);
    }
    return BH_STATUS_NOT_AVAILABLE;
}

static inline uint64_t platform_get_time(void) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->get_time) {
        return platform_mgr.current_platform->ops->get_time();
    }
    return 0;
}

static inline void platform_delay(uint32_t ms) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->delay) {
        platform_mgr.current_platform->ops->delay(ms);
    }
}

static inline void platform_udelay(uint32_t us) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->udelay) {
        platform_mgr.current_platform->ops->udelay(us);
    }
}

static inline void platform_reset(void) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->reset) {
        platform_mgr.current_platform->ops->reset();
    }
}

static inline void platform_power_off(void) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->power_off) {
        platform_mgr.current_platform->ops->power_off();
    }
}

static inline bh_status_t platform_arch_init(void) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->arch_init) {
        return platform_mgr.current_platform->ops->arch_init();
    }
    return BH_STATUS_NOT_AVAILABLE;
}

static inline bh_status_t platform_arch_cleanup(void) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->arch_cleanup) {
        return platform_mgr.current_platform->ops->arch_cleanup();
    }
    return BH_STATUS_NOT_AVAILABLE;
}

static inline bh_status_t platform_print_info(void) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->print_info) {
        return platform_mgr.current_platform->ops->print_info();
    }
    return BH_STATUS_NOT_AVAILABLE;
}

static inline bh_status_t platform_debug_print(const char* format, ...) {
    if (platform_mgr.current_platform && platform_mgr.current_platform->ops->debug_print) {
        va_list args;
        va_start(args, format);
        bh_status_t status = platform_mgr.current_platform->ops->debug_print(format, args);
        va_end(args);
        return status;
    }
    return BH_STATUS_NOT_AVAILABLE;
}

// Platform query functions
bh_platform_type_t platform_get_type(void);
const char* platform_get_name(void);
const struct platform_operations* platform_get_operations(void);
bool platform_is_initialized(void);

// Platform-specific initialization functions
bh_status_t platform_init_uboot(void);
bh_status_t platform_init_openfirmware(void);
bh_status_t platform_init_uefi(void);
bh_status_t platform_init_coreboot(void);
bh_status_t platform_init_bios(void);

// Platform manager initialization
bh_status_t platform_manager_init(void);
bh_status_t platform_manager_shutdown(void);

#endif /* _PLATFORM_H_ */
