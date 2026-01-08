/*
 * openfirmware.h - OpenFirmware platform support for BloodHorn
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef _OPENFIRMWARE_H_
#define _OPENFIRMWARE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../libb/include/bloodhorn/bootinfo.h"

// OpenFirmware client interface structure
struct ofw_client_interface {
    void* entry_point;          // OFW entry point
    uint32_t version;           // OFW version
    uint32_t revision;          // OFW revision
} __attribute__((packed));

// OpenFirmware argument structure for method calls
struct ofw_args {
    const char* service;        // Service name
    uint32_t nargs;             // Number of input arguments
    uint32_t nret;              // Number of return arguments
    uint32_t args[12];          // Input/output arguments
} __attribute__((packed));

// OpenFirmware property structure
struct ofw_property {
    char* name;                 // Property name
    uint32_t length;            // Property length
    void* value;                // Property value
    struct ofw_property* next; // Next property in list
};

// OpenFirmware node structure
struct ofw_node {
    uint32_t phandle;           // Package handle
    char* name;                 // Node name
    char* type;                 // Node type
    struct ofw_property* properties; // Property list
    struct ofw_node* parent;    // Parent node
    struct ofw_node* child;     // First child node
    struct ofw_node* sibling;   // Next sibling node
    struct ofw_node* next;      // Next node in traversal
};

// OpenFirmware memory region structure
struct ofw_memory {
    uint64_t start;             // Start address
    uint64_t size;              // Region size
    uint32_t type;              // Memory type
};

// OpenFirmware memory types
#define OFW_MEMORY_TYPE_RAM     0
#define OFW_MEMORY_TYPE_ROM     1
#define OFW_MEMORY_TYPE_MMIO    2
#define OFW_MEMORY_TYPE_RESERVED 3

// OpenFirmware standard services
#define OFW_SERVICE_TEST        "test"
#define OFW_SERVICE_PEER        "peer"
#define OFW_SERVICE_CHILD        "child"
#define OFW_SERVICE_PARENT      "parent"
#define OFW_SERVICE_INSTANCE_TO_PACKAGE "instance-to-package"
#define OFW_SERVICE_PACKAGE_TO_PATH "package-to-path"
#define OFW_SERVICE_GETPROP     "getprop"
#define OFW_SERVICE_SETPROP     "setprop"
#define OFW_SERVICE_NEXTPROP    "nextprop"
#define OFW_SERVICE_PROPLEN     "proplen"
#define OFW_SERVICE_FINDDEVICE  "finddevice"
#define OFW_SERVICE_INSTANCE_TO_PATH "instance-to-path"
#define OFW_SERVICE_WRITE       "write"
#define OFW_SERVICE_READ        "read"
#define OFW_SERVICE_SEEK        "seek"
#define OFW_SERVICE_OPEN        "open"
#define OFW_SERVICE_CLOSE       "close"
#define OFW_SERVICE_BOOT        "boot"
#define OFW_SERVICE_ENTER       "enter"
#define OFW_SERVICE_EXIT        "exit"
#define OFW_SERVICE_QUIT        "quit"
#define OFW_SERVICE_CHAIN       "chain"
#define OFW_SERVICE_MILLI_TO_TICKS "milliseconds"
#define OFW_SERVICE_TICKS       "ticks"

// OpenFirmware return codes
#define OFW_SUCCESS             0
#define OFW_FAILURE            -1
#define OFW_INVALID_ARGUMENT   -2
#define OFW_NOT_FOUND          -3
#define OFW_NO_MEMORY          -4
#define OFW_IO_ERROR           -5
#define OFW_UNSUPPORTED        -6

// OpenFirmware standard paths
#define OFW_PATH_CHOSEN        "/chosen"
#define OFW_PATH_OPTIONS       "/options"
#define OFW_PATH_MEMORY        "/memory"
#define OFW_PATH_CPUS          "/cpus"
#define OFW_PATH_ALIASES       "/aliases"
#define OFW_PATH_OPENPROM      "/openprom"

// OpenFirmware standard properties
#define OFW_PROP_DEVICE_TYPE   "device_type"
#define OFW_PROP_COMPATIBLE    "compatible"
#define OFW_PROP_MODEL         "model"
#define OFW_PROP_REG           "reg"
#define OFW_PROP_RANGES        "ranges"
#define OFW_PROP_INTERRUPTS     "interrupts"
#define OFW_PROP_INTERRUPT_PARENT "interrupt-parent"
#define OFW_PROP_ADDRESS_CELLS "#address-cells"
#define OFW_PROP_SIZE_CELLS    "#size-cells"
#define OFW_PROP_PHANDLE       "phandle"
#define OFW_PROP_LINUX_PHANDLE "linux,phandle"

// OpenFirmware boot arguments
struct ofw_boot_args {
    uint32_t client_interface;  // Client interface pointer
    uint32_t args;              // Arguments pointer
    uint32_t entry;             // Entry point
    uint32_t memsize;           // Memory size
    uint32_t libc;              // LibC base
} __attribute__((packed));

// Function prototypes
bh_status_t ofw_detect_platform(void);
bh_status_t ofw_initialize_platform(bh_boot_info_t* boot_info);
bh_status_t ofw_get_client_interface(struct ofw_client_interface** ci);
bh_status_t ofw_call_method(const char* service, int nargs, int nret, ...);
bh_status_t ofw_find_device(const char* path, uint32_t* phandle);
bh_status_t ofw_get_prop(uint32_t phandle, const char* propname, void* buffer, size_t* size);
bh_status_t ofw_set_prop(uint32_t phandle, const char* propname, const void* buffer, size_t size);
bh_status_t ofw_next_prop(uint32_t phandle, const char* prevprop, char* nextprop, size_t size);
bh_status_t ofw_prop_len(uint32_t phandle, const char* propname, size_t* length);
bh_status_t ofw_package_to_path(uint32_t phandle, char* buffer, size_t* size);
bh_status_t ofw_instance_to_package(uint32_t ihandle, uint32_t* phandle);

// OpenFirmware tree traversal
bh_status_t ofw_get_root_node(struct ofw_node** root);
bh_status_t ofw_get_child_node(struct ofw_node* parent, struct ofw_node** child);
bh_status_t ofw_get_peer_node(struct ofw_node* node, struct ofw_node** peer);
bh_status_t ofw_get_parent_node(struct ofw_node* node, struct ofw_node** parent);
bh_status_t ofw_find_node_by_path(const char* path, struct ofw_node** node);
bh_status_t ofw_find_node_by_property(const char* propname, const char* value, struct ofw_node** node);
bh_status_t ofw_find_node_by_compatible(const char* compatible, struct ofw_node** node);

// OpenFirmware memory management
bh_status_t ofw_get_memory_map(struct ofw_memory** memory, size_t* count);
bh_status_t ofw_claim_memory(uint64_t virt, uint64_t size, uint32_t align);
bh_status_t ofw_release_memory(uint64_t virt, uint64_t size);
bh_status_t ofw_map_physical(uint64_t phys, uint64_t virt, uint64_t size);

// OpenFirmware I/O
bh_status_t ofw_open(const char* path, uint32_t* ihandle);
bh_status_t ofw_close(uint32_t ihandle);
bh_status_t ofw_read(uint32_t ihandle, void* buffer, size_t size, size_t* bytes_read);
bh_status_t ofw_write(uint32_t ihandle, const void* buffer, size_t size, size_t* bytes_written);
bh_status_t ofw_seek(uint32_t ihandle, uint64_t position);

// OpenFirmware console
bh_status_t ofw_console_init(void);
void ofw_putc(char c);
void ofw_puts(const char* s);
int ofw_getc(void);
int ofw_tstc(void);

// OpenFirmware boot
bh_status_t ofw_get_boot_args(struct ofw_boot_args** args);
bh_status_t ofw_get_chosen_property(const char* propname, void* buffer, size_t* size);
bh_status_t ofw_get_command_line(char** cmdline, size_t* size);
bh_status_t ofw_get_initrd_info(uint64_t* start, uint64_t* end);
bh_status_t ofw_boot_kernel(const char* path, const char* args);

// OpenFirmware timing
bh_status_t ofw_get_time(uint64_t* seconds, uint64_t* microseconds);
bh_status_t ofw_milliseconds_to_ticks(uint32_t ms, uint32_t* ticks);
bh_status_t ofw_get_ticks(uint32_t* ticks);
void ofw_delay(uint32_t ms);

// OpenFirmware debugging
bh_status_t ofw_print_tree(void);
bh_status_t ofw_print_node(struct ofw_node* node, int depth);
bh_status_t ofw_print_properties(uint32_t phandle);

// OpenFirmware utilities
bh_status_t ofw_interpret(const char* forth_code, uint32_t* result);
bh_status_t ofw_set_callback(uint32_t* callback);
bh_status_t ofw_test(const char* service, uint32_t* missing);

// OpenFirmware error handling
const char* ofw_error_string(bh_status_t status);

// OpenFirmware architecture-specific functions
bh_status_t ofw_arch_init(void);
bh_status_t ofw_arch_cleanup(void);
void ofw_reset(void);
void ofw_power_off(void);

#endif /* _OPENFIRMWARE_H_ */
