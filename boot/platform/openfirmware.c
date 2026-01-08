/*
 * openfirmware.c - OpenFirmware platform support implementation for BloodHorn
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "openfirmware.h"
#include "../libb/include/bloodhorn/bootinfo.h"
#include "../Arch32/powerpc.h"

// Global OpenFirmware data
static struct ofw_client_interface* ofw_ci = NULL;
static struct ofw_boot_args* ofw_boot_args = NULL;
static struct ofw_node* ofw_root = NULL;
static bool ofw_detected = false;
static uint32_t ofw_stdout_ihandle = 0;
static uint32_t ofw_stdin_ihandle = 0;

// OpenFirmware entry point (provided by firmware)
extern uint32_t ofw_entry_point;

// Helper function to call OpenFirmware methods
static bh_status_t ofw_call_method_va(const char* service, int nargs, int nret, va_list args) {
    if (!ofw_ci || !service) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    struct ofw_args ofw_args;
    ofw_args.service = service;
    ofw_args.nargs = nargs;
    ofw_args.nret = nret;
    
    // Copy input arguments
    for (int i = 0; i < nargs && i < 6; i++) {
        ofw_args.args[i] = va_arg(args, uint32_t);
    }
    
    // Call OpenFirmware
    uint32_t result = ((uint32_t (*)(struct ofw_args*))ofw_ci->entry_point)(&ofw_args);
    
    // Check return value
    if (result != OFW_SUCCESS) {
        return BH_STATUS_IO_ERROR;
    }
    
    // Check return arguments
    if (nret > 0 && ofw_args.args[nargs] != OFW_SUCCESS) {
        return BH_STATUS_IO_ERROR;
    }
    
    return BH_STATUS_SUCCESS;
}

// Platform detection
bh_status_t ofw_detect_platform(void) {
    // Check if we have OpenFirmware boot arguments
    if (!ofw_boot_args) {
        // Try to locate boot arguments in known locations
        // On PowerPC, boot arguments are typically passed in r3-r5
        extern uint32_t ofw_boot_args_ptr;
        if (ofw_boot_args_ptr) {
            ofw_boot_args = (struct ofw_boot_args*)ofw_boot_args_ptr;
        }
    }
    
    if (!ofw_boot_args) {
        return BH_STATUS_NOT_FOUND;
    }
    
    // Validate boot arguments
    if (!ofw_boot_args->client_interface || !ofw_boot_args->args) {
        return BH_STATUS_INVALID_DATA;
    }
    
    ofw_ci = (struct ofw_client_interface*)ofw_boot_args->client_interface;
    
    // Test OpenFirmware availability
    uint32_t test_result;
    bh_status_t status = ofw_call_method("test", 1, 2, "test-method", &test_result);
    if (status == BH_STATUS_SUCCESS) {
        ofw_detected = true;
        return BH_STATUS_SUCCESS;
    }
    
    return BH_STATUS_NOT_FOUND;
}

// Initialize OpenFirmware platform
bh_status_t ofw_initialize_platform(bh_boot_info_t* boot_info) {
    if (!ofw_detected) {
        bh_status_t status = ofw_detect_platform();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    if (!boot_info) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Set platform type
    boot_info->platform.type = BH_PLATFORM_OPENFIRMWARE;
    boot_info->platform.arch = BH_ARCH_POWERPC;
    
    // Copy platform information
    if (ofw_ci) {
        strcpy(boot_info->platform.platform_name, "OpenFirmware");
        strcpy(boot_info->platform.firmware_vendor, "IEEE 1275");
        snprintf(boot_info->platform.firmware_version, sizeof(boot_info->platform.firmware_version),
                "%d.%d", ofw_ci->version, ofw_ci->revision);
    }
    
    // Initialize console
    bh_status_t status = ofw_console_init();
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    // Build device tree
    status = ofw_get_root_node(&ofw_root);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    // Setup memory map
    status = ofw_setup_memory_map(boot_info);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    // Get boot arguments
    status = ofw_get_boot_arguments(boot_info);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    return BH_STATUS_SUCCESS;
}

// Get client interface
bh_status_t ofw_get_client_interface(struct ofw_client_interface** ci) {
    if (!ci) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    if (!ofw_detected) {
        bh_status_t status = ofw_detect_platform();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    *ci = ofw_ci;
    return ofw_ci ? BH_STATUS_SUCCESS : BH_STATUS_NOT_FOUND;
}

// Call OpenFirmware method
bh_status_t ofw_call_method(const char* service, int nargs, int nret, ...) {
    va_list args;
    va_start(args, nret);
    bh_status_t status = ofw_call_method_va(service, nargs, nret, args);
    va_end(args);
    return status;
}

// Find device by path
bh_status_t ofw_find_device(const char* path, uint32_t* phandle) {
    if (!path || !phandle) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    return ofw_call_method("finddevice", 1, 2, path, phandle);
}

// Get property
bh_status_t ofw_get_prop(uint32_t phandle, const char* propname, void* buffer, size_t* size) {
    if (!propname || !buffer || !size) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    uint32_t actual_size;
    bh_status_t status = ofw_call_method("getprop", 3, 2, phandle, propname, buffer, &actual_size);
    
    if (status == BH_STATUS_SUCCESS) {
        *size = actual_size;
    }
    
    return status;
}

// Set property
bh_status_t ofw_set_prop(uint32_t phandle, const char* propname, const void* buffer, size_t size) {
    if (!propname || !buffer) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    return ofw_call_method("setprop", 4, 1, phandle, propname, buffer, size);
}

// Get next property
bh_status_t ofw_next_prop(uint32_t phandle, const char* prevprop, char* nextprop, size_t size) {
    if (!nextprop) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    return ofw_call_method("nextprop", 3, 1, phandle, prevprop, nextprop);
}

// Get property length
bh_status_t ofw_prop_len(uint32_t phandle, const char* propname, size_t* length) {
    if (!propname || !length) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    uint32_t len;
    bh_status_t status = ofw_call_method("proplen", 2, 1, phandle, propname, &len);
    
    if (status == BH_STATUS_SUCCESS) {
        *length = len;
    }
    
    return status;
}

// Convert package to path
bh_status_t ofw_package_to_path(uint32_t phandle, char* buffer, size_t* size) {
    if (!buffer || !size) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    uint32_t actual_size;
    bh_status_t status = ofw_call_method("package-to-path", 2, 2, phandle, buffer, &actual_size);
    
    if (status == BH_STATUS_SUCCESS) {
        *size = actual_size;
    }
    
    return status;
}

// Convert instance to package
bh_status_t ofw_instance_to_package(uint32_t ihandle, uint32_t* phandle) {
    if (!phandle) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    return ofw_call_method("instance-to-package", 1, 2, ihandle, phandle);
}

// Initialize console
bh_status_t ofw_console_init(void) {
    // Find standard I/O devices
    uint32_t chosen_phandle;
    bh_status_t status = ofw_find_device(OFW_PATH_CHOSEN, &chosen_phandle);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    // Get stdout handle
    size_t size = sizeof(ofw_stdout_ihandle);
    status = ofw_get_prop(chosen_phandle, "stdout", &ofw_stdout_ihandle, &size);
    if (status != BH_STATUS_SUCCESS) {
        ofw_stdout_ihandle = 0; // Use default
    }
    
    // Get stdin handle
    size = sizeof(ofw_stdin_ihandle);
    status = ofw_get_prop(chosen_phandle, "stdin", &ofw_stdin_ihandle, &size);
    if (status != BH_STATUS_SUCCESS) {
        ofw_stdin_ihandle = 0; // Use default
    }
    
    return BH_STATUS_SUCCESS;
}

// Console output
void ofw_putc(char c) {
    if (ofw_stdout_ihandle) {
        size_t bytes_written;
        ofw_write(ofw_stdout_ihandle, &c, 1, &bytes_written);
    }
}

void ofw_puts(const char* s) {
    if (ofw_stdout_ihandle && s) {
        size_t len = strlen(s);
        size_t bytes_written;
        ofw_write(ofw_stdout_ihandle, s, len, &bytes_written);
    }
}

// Console input
int ofw_getc(void) {
    if (ofw_stdin_ihandle) {
        char c;
        size_t bytes_read;
        bh_status_t status = ofw_read(ofw_stdin_ihandle, &c, 1, &bytes_read);
        if (status == BH_STATUS_SUCCESS && bytes_read == 1) {
            return c;
        }
    }
    return -1;
}

int ofw_tstc(void) {
    if (ofw_stdin_ihandle) {
        // This would need a specific OFW method to test for input
        // For now, return 0 (no input available)
        return 0;
    }
    return 0;
}

// Open device
bh_status_t ofw_open(const char* path, uint32_t* ihandle) {
    if (!path || !ihandle) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    return ofw_call_method("open", 1, 2, path, ihandle);
}

// Close device
bh_status_t ofw_close(uint32_t ihandle) {
    return ofw_call_method("close", 1, 1, ihandle);
}

// Read from device
bh_status_t ofw_read(uint32_t ihandle, void* buffer, size_t size, size_t* bytes_read) {
    if (!buffer || !bytes_read) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    uint32_t actual_bytes;
    bh_status_t status = ofw_call_method("read", 3, 2, ihandle, buffer, size, &actual_bytes);
    
    if (status == BH_STATUS_SUCCESS) {
        *bytes_read = actual_bytes;
    }
    
    return status;
}

// Write to device
bh_status_t ofw_write(uint32_t ihandle, const void* buffer, size_t size, size_t* bytes_written) {
    if (!buffer || !bytes_written) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    uint32_t actual_bytes;
    bh_status_t status = ofw_call_method("write", 3, 2, ihandle, buffer, size, &actual_bytes);
    
    if (status == BH_STATUS_SUCCESS) {
        *bytes_written = actual_bytes;
    }
    
    return status;
}

// Seek in device
bh_status_t ofw_seek(uint32_t ihandle, uint64_t position) {
    uint32_t pos_hi = position >> 32;
    uint32_t pos_lo = position & 0xFFFFFFFF;
    
    return ofw_call_method("seek", 3, 1, ihandle, pos_hi, pos_lo);
}

// Get boot arguments
bh_status_t ofw_get_boot_args(struct ofw_boot_args** args) {
    if (!args) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    if (!ofw_detected) {
        bh_status_t status = ofw_detect_platform();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    *args = ofw_boot_args;
    return ofw_boot_args ? BH_STATUS_SUCCESS : BH_STATUS_NOT_FOUND;
}

// Get chosen property
bh_status_t ofw_get_chosen_property(const char* propname, void* buffer, size_t* size) {
    uint32_t chosen_phandle;
    bh_status_t status = ofw_find_device(OFW_PATH_CHOSEN, &chosen_phandle);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    return ofw_get_prop(chosen_phandle, propname, buffer, size);
}

// Get command line
bh_status_t ofw_get_command_line(char** cmdline, size_t* size) {
    if (!cmdline || !size) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    char cmdline_buffer[1024];
    size_t cmdline_size = sizeof(cmdline_buffer);
    
    bh_status_t status = ofw_get_chosen_property("bootargs", cmdline_buffer, &cmdline_size);
    if (status == BH_STATUS_SUCCESS && cmdline_size > 0) {
        *cmdline = strdup(cmdline_buffer);
        *size = cmdline_size;
        return BH_STATUS_SUCCESS;
    }
    
    *cmdline = NULL;
    *size = 0;
    return BH_STATUS_NOT_FOUND;
}

// Get initrd information
bh_status_t ofw_get_initrd_info(uint64_t* start, uint64_t* end) {
    if (!start || !end) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    uint64_t initrd_start, initrd_end;
    size_t size = sizeof(initrd_start);
    
    bh_status_t status = ofw_get_chosen_property("linux,initrd-start", &initrd_start, &size);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    size = sizeof(initrd_end);
    status = ofw_get_chosen_property("linux,initrd-end", &initrd_end, &size);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    *start = initrd_start;
    *end = initrd_end;
    
    return BH_STATUS_SUCCESS;
}

// Setup memory map
bh_status_t ofw_setup_memory_map(bh_boot_info_t* boot_info) {
    if (!boot_info) {
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
        
        status = bh_memory_map_add_range(&boot_info->memory_map, &memory_range);
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    return BH_STATUS_SUCCESS;
}

// Get boot arguments and populate boot info
bh_status_t ofw_get_boot_arguments(bh_boot_info_t* boot_info) {
    if (!boot_info) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Get command line
    char* cmdline;
    size_t cmdline_size;
    bh_status_t status = ofw_get_command_line(&cmdline, &cmdline_size);
    if (status == BH_STATUS_SUCCESS) {
        boot_info->command_line = cmdline;
        BH_BOOTINFO_SET_FLAG(boot_info, BH_BOOT_FLAG_CMDLINE_PRESENT);
    }
    
    // Get initrd information
    uint64_t initrd_start, initrd_end;
    status = ofw_get_initrd_info(&initrd_start, &initrd_end);
    if (status == BH_STATUS_SUCCESS) {
        bh_module_info_t initrd_module = {0};
        initrd_module.type = BH_MODULE_TYPE_INITRD;
        initrd_module.start = initrd_start;
        initrd_module.end = initrd_end;
        initrd_module.size = initrd_end - initrd_start;
        strcpy(initrd_module.name, "initrd");
        
        status = bh_bootinfo_add_module(boot_info, &initrd_module);
        if (status == BH_STATUS_SUCCESS) {
            BH_BOOTINFO_SET_FLAG(boot_info, BH_BOOT_FLAG_MODULES_PRESENT);
        }
    }
    
    return BH_STATUS_SUCCESS;
}

// Timing functions
bh_status_t ofw_milliseconds_to_ticks(uint32_t ms, uint32_t* ticks) {
    if (!ticks) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    return ofw_call_method("milliseconds", 1, 2, ms, ticks);
}

bh_status_t ofw_get_ticks(uint32_t* ticks) {
    if (!ticks) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    return ofw_call_method("ticks", 0, 1, ticks);
}

void ofw_delay(uint32_t ms) {
    uint32_t ticks;
    if (ofw_milliseconds_to_ticks(ms, &ticks) == BH_STATUS_SUCCESS) {
        uint32_t start_ticks;
        if (ofw_get_ticks(&start_ticks) == BH_STATUS_SUCCESS) {
            uint32_t current_ticks;
            do {
                ofw_get_ticks(&current_ticks);
            } while ((current_ticks - start_ticks) < ticks);
        }
    }
}

// Architecture-specific initialization
bh_status_t ofw_arch_init(void) {
    // Initialize PowerPC-specific OpenFirmware features
    return BH_STATUS_SUCCESS;
}

bh_status_t ofw_arch_cleanup(void) {
    // Cleanup PowerPC-specific OpenFirmware features
    return BH_STATUS_SUCCESS;
}

void ofw_reset(void) {
    // Use OpenFirmware reset method
    ofw_call_method("reset-all", 0, 0);
}

void ofw_power_off(void) {
    // Use OpenFirmware power off method
    ofw_call_method("power-off", 0, 0);
}

// Error string conversion
const char* ofw_error_string(bh_status_t status) {
    switch (status) {
        case BH_STATUS_SUCCESS:
            return "Success";
        case BH_STATUS_INVALID_PARAMETER:
            return "Invalid parameter";
        case BH_STATUS_NOT_FOUND:
            return "Not found";
        case BH_STATUS_IO_ERROR:
            return "I/O error";
        case BH_STATUS_NO_MEMORY:
            return "Out of memory";
        case BH_STATUS_UNSUPPORTED:
            return "Unsupported operation";
        default:
            return "Unknown error";
    }
}
