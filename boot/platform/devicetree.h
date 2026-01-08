/*
 * devicetree.h - Unified device tree support for BloodHorn
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef _DEVICETREE_H_
#define _DEVICETREE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../libb/include/bloodhorn/bootinfo.h"

// Flattened Device Tree (FDT) constants
#define FDT_MAGIC               0xd00dfeed
#define FDT_VERSION             17
#define FDT_LAST_COMPAT_VERSION 16

// FDT token values
#define FDT_BEGIN_NODE          0x1
#define FDT_END_NODE            0x2
#define FDT_PROP                0x3
#define FDT_NOP                 0x4
#define FDT_END                 0x9

// FDT header structure
struct fdt_header {
    uint32_t magic;             // Magic word FDT_MAGIC
    uint32_t totalsize;         // Total size of DT block
    uint32_t off_dt_struct;     // Offset to structure block
    uint32_t off_dt_strings;    // Offset to strings block
    uint32_t off_mem_rsvmap;    // Offset to memory reserve map
    uint32_t version;           // Format version
    uint32_t last_comp_version; // Last compatible version
    uint32_t boot_cpuid_phys;   // Physical CPU ID we're booting on
    uint32_t size_dt_strings;   // Size of strings block
    uint32_t size_dt_struct;    // Size of structure block
} __attribute__((packed));

// FDT memory reserve entry
struct fdt_reserve_entry {
    uint64_t address;           // Physical address
    uint64_t size;              // Size
} __attribute__((packed));

// FDT property structure
struct fdt_property {
    uint32_t tag;               // Tag (FDT_PROP)
    uint32_t len;               // Length of property data
    uint32_t nameoff;           // Offset to name in strings block
    uint8_t data[];             // Property data
} __attribute__((packed));

// Device tree node structure
struct device_tree_node {
    char* name;                 // Node name
    char* full_path;            // Full path from root
    uint32_t phandle;           // Package handle
    struct device_tree_property* properties; // Property list
    struct device_tree_node* parent;          // Parent node
    struct device_tree_node* children;        // First child
    struct device_tree_node* sibling;         // Next sibling
    struct device_tree_node* next;             // Next in traversal
    uint32_t address_cells;     // #address-cells value
    uint32_t size_cells;        // #size-cells value
    uint32_t interrupt_cells;   // #interrupt-cells value
    uint32_t interrupt_parent;  // interrupt-parent value
};

// Device tree property structure
struct device_tree_property {
    char* name;                 // Property name
    void* value;                // Property value
    uint32_t length;            // Property length
    struct device_tree_property* next; // Next property
};

// Device tree iterator structure
struct device_tree_iterator {
    struct device_tree_node* current;
    struct device_tree_node* root;
    uint32_t flags;             // Iterator flags
    void* context;              // User context
};

// Iterator flags
#define DT_ITERATE_CHILDREN     (1 << 0)
#define DT_ITERATE_SIBLINGS      (1 << 1)
#define DT_ITERATE_PROPERTIES    (1 << 2)
#define DT_ITERATE_RECURSIVE     (1 << 3)
#define DT_ITERATE_LEAVES_ONLY   (1 << 4)

// Device tree memory region
struct dt_memory_region {
    uint64_t address;           // Physical address
    uint64_t size;              // Region size
    uint32_t type;              // Memory type
    char* name;                 // Region name
};

// Device tree CPU information
struct dt_cpu_info {
    uint32_t reg;               // CPU register/number
    char* compatible;           // Compatible string
    char* model;                // Model string
    uint32_t clock_frequency;   // Clock frequency
    uint32_t timebase_frequency; // Timebase frequency
    bool enabled;               // CPU enabled status
};

// Device tree interrupt information
struct dt_interrupt_info {
    uint32_t controller;        // Interrupt controller phandle
    uint32_t interrupt;         // Interrupt number
    uint32_t flags;             // Interrupt flags
    uint32_t type;              // Interrupt type
};

// Device tree address translation
struct dt_address_translation {
    uint64_t bus_address;       // Bus address
    uint64_t cpu_address;       // CPU address
    uint64_t size;              // Size
    uint32_t flags;             // Translation flags
};

// Device tree GPIO information
struct dt_gpio_info {
    uint32_t controller;        // GPIO controller phandle
    uint32_t gpio;              // GPIO number
    uint32_t flags;             // GPIO flags
    char* label;                // GPIO label
};

// Device tree clock information
struct dt_clock_info {
    uint32_t controller;        // Clock controller phandle
    uint32_t clock;             // Clock ID
    uint32_t flags;             // Clock flags
    uint32_t frequency;         // Clock frequency
};

// Device tree regulator information
struct dt_regulator_info {
    char* name;                 // Regulator name
    uint32_t min_voltage;       // Minimum voltage
    uint32_t max_voltage;       // Maximum voltage
    uint32_t current;           // Current limit
    bool enabled;               // Regulator enabled
};

// Device tree reset information
struct dt_reset_info {
    uint32_t controller;        // Reset controller phandle
    uint32_t reset;             // Reset ID
    uint32_t flags;             // Reset flags
};

// Device tree DMA information
struct dt_dma_info {
    uint32_t controller;        // DMA controller phandle
    uint32_t channel;           // DMA channel
    uint32_t flags;             // DMA flags
};

// Device tree phy information
struct dt_phy_info {
    uint32_t controller;        // PHY controller phandle
    uint32_t phy;               // PHY address
    uint32_t flags;             // PHY flags
    char* device_type;          // PHY device type
};

// Device tree PCI information
struct dt_pci_info {
    uint64_t bus_address;       // PCI bus address
    uint64_t cpu_address;       // CPU address
    uint64_t size;              // Size
    uint32_t flags;             // PCI flags
    uint32_t bus_number;        // Bus number
    uint32_t device_number;     // Device number
    uint32_t function_number;   // Function number
};

// Device tree I2C information
struct dt_i2c_info {
    uint32_t controller;        // I2C controller phandle
    uint32_t address;          // I2C address
    uint32_t flags;            // I2C flags
    uint32_t frequency;        // I2C frequency
};

// Device tree SPI information
struct dt_spi_info {
    uint32_t controller;        // SPI controller phandle
    uint32_t chip_select;       // Chip select
    uint32_t max_frequency;     // Maximum frequency
    uint32_t mode;              // SPI mode
};

// Device tree PWM information
struct dt_pwm_info {
    uint32_t controller;        // PWM controller phandle
    uint32_t pwm;               // PWM channel
    uint32_t period;           // PWM period
    uint32_t duty_cycle;       // PWM duty cycle
};

// Device tree thermal information
struct dt_thermal_info {
    uint32_t sensor;            // Thermal sensor phandle
    int32_t temperature;        // Current temperature
    int32_t trip_points[8];     // Trip point temperatures
    uint32_t trip_count;        // Number of trip points
    bool passive;               // Passive cooling
    bool active;                // Active cooling
};

// Device tree backlight information
struct dt_backlight_info {
    uint32_t controller;        // Backlight controller phandle
    uint32_t brightness;        // Current brightness
    uint32_t max_brightness;    // Maximum brightness
    bool enabled;               // Backlight enabled
};

// Device tree display information
struct dt_display_info {
    uint32_t width;             // Display width
    uint32_t height;            // Display height
    uint32_t bpp;               // Bits per pixel
    uint32_t refresh_rate;      // Refresh rate
    char* format;               // Pixel format
    struct dt_backlight_info backlight; // Backlight info
};

// Device tree sound information
struct dt_sound_info {
    uint32_t controller;        // Sound controller phandle
    uint32_t format;            // Audio format
    uint32_t channels;          // Number of channels
    uint32_t sample_rate;       // Sample rate
    uint32_t bits_per_sample;   // Bits per sample
};

// Device tree video information
struct dt_video_info {
    uint32_t controller;        // Video controller phandle
    struct dt_display_info display; // Display info
    uint32_t framebuffer;       // Framebuffer address
    uint32_t framebuffer_size;  // Framebuffer size
};

// Device tree network information
struct dt_network_info {
    uint32_t controller;        // Network controller phandle
    uint8_t mac_address[6];     // MAC address
    uint32_t speed;             // Link speed
    bool full_duplex;           // Full duplex
    bool link_up;               // Link status
};

// Device tree storage information
struct dt_storage_info {
    uint32_t controller;        // Storage controller phandle
    char* device_type;          // Device type
    uint64_t size;              // Storage size
    uint32_t block_size;        // Block size
    bool removable;             // Removable media
};

// Device tree input information
struct dt_input_info {
    uint32_t controller;        // Input controller phandle
    char* device_type;          // Input device type
    uint32_t buttons;           // Number of buttons
    uint32_t axes;              // Number of axes
    bool has_touch;             // Touch support
};

// Device tree power management information
struct dt_power_info {
    uint32_t controller;        // Power controller phandle
    bool can_suspend;           // Suspend support
    bool can_hibernate;         // Hibernate support
    bool can_power_off;         // Power off support
    uint32_t wakeup_sources;    // Wakeup sources
};

// Device tree security information
struct dt_security_info {
    uint32_t controller;        // Security controller phandle
    bool secure_boot;           // Secure boot enabled
    bool trusted_execution;     // Trusted execution
    bool hardware_crypto;       // Hardware crypto
    uint32_t key_size;          // Key size
};

// Device tree debugging information
struct dt_debug_info {
    uint32_t controller;        // Debug controller phandle
    bool jtag_enabled;          // JTAG enabled
    bool swd_enabled;           // SWD enabled
    bool trace_enabled;         // Trace enabled
    uint32_t debug_port;        // Debug port
};

// Device tree validation information
struct dt_validation_info {
    bool valid;                 // Tree is valid
    uint32_t errors;            // Number of errors
    uint32_t warnings;          // Number of warnings
    char* error_message;        // Error message
    char* warning_message;      // Warning message
};

// Function prototypes for device tree operations

// Core device tree operations
bh_status_t dt_load_from_blob(const void* blob, size_t size, struct device_tree_node** root);
bh_status_t dt_load_from_platform(void** blob, size_t* size);
bh_status_t dt_save_to_blob(const struct device_tree_node* root, void** blob, size_t* size);
bh_status_t dt_validate(const struct device_tree_node* root, struct dt_validation_info* info);
bh_status_t dt_cleanup(struct device_tree_node* root);

// Node operations
bh_status_t dt_find_node(const struct device_tree_node* root, const char* path, struct device_tree_node** node);
bh_status_t dt_find_node_by_property(const struct device_tree_node* root, const char* prop_name, 
                                     const void* prop_value, size_t prop_size, struct device_tree_node** node);
bh_status_t dt_find_node_by_compatible(const struct device_tree_node* root, const char* compatible, 
                                       struct device_tree_node** node);
bh_status_t dt_find_node_by_phandle(const struct device_tree_node* root, uint32_t phandle, 
                                    struct device_tree_node** node);
bh_status_t dt_add_node(struct device_tree_node* parent, const char* name, struct device_tree_node** node);
bh_status_t dt_remove_node(struct device_tree_node* parent, struct device_tree_node* node);
bh_status_t dt_move_node(struct device_tree_node* node, struct device_tree_node* new_parent);

// Property operations
bh_status_t dt_get_property(const struct device_tree_node* node, const char* name, 
                           void** value, size_t* length);
bh_status_t dt_set_property(struct device_tree_node* node, const char* name, 
                           const void* value, size_t length);
bh_status_t dt_remove_property(struct device_tree_node* node, const char* name);
bh_status_t dt_get_string_property(const struct device_tree_node* node, const char* name, 
                                   char** string);
bh_status_t dt_set_string_property(struct device_tree_node* node, const char* name, const char* string);
bh_status_t dt_get_u32_property(const struct device_tree_node* node, const char* name, uint32_t* value);
bh_status_t dt_set_u32_property(struct device_tree_node* node, const char* name, uint32_t value);
bh_status_t dt_get_u64_property(const struct device_tree_node* node, const char* name, uint64_t* value);
bh_status_t dt_set_u64_property(struct device_tree_node* node, const char* name, uint64_t value);
bh_status_t dt_get_bool_property(const struct device_tree_node* node, const char* name, bool* value);
bh_status_t dt_set_bool_property(struct device_tree_node* node, const char* name, bool value);

// Address translation
bh_status_t dt_translate_address(const struct device_tree_node* node, uint64_t bus_address, 
                                uint64_t* cpu_address, uint64_t* size);
bh_status_t dt_translate_addresses(const struct device_tree_node* node, const uint64_t* bus_addresses, 
                                   uint64_t* cpu_addresses, uint64_t* sizes, size_t count);

// Memory operations
bh_status_t dt_get_memory_regions(const struct device_tree_node* root, struct dt_memory_region** regions, 
                                  size_t* count);
bh_status_t dt_add_memory_region(struct device_tree_node* root, const struct dt_memory_region* region);
bh_status_t dt_remove_memory_region(struct device_tree_node* root, uint64_t address);

// CPU operations
bh_status_t dt_get_cpu_info(const struct device_tree_node* root, struct dt_cpu_info** cpus, size_t* count);
bh_status_t dt_get_cpu_by_reg(const struct device_tree_node* root, uint32_t reg, struct dt_cpu_info* cpu);
bh_status_t dt_get_boot_cpu(const struct device_tree_node* root, struct dt_cpu_info* cpu);

// Interrupt operations
bh_status_t dt_get_interrupts(const struct device_tree_node* node, struct dt_interrupt_info** interrupts, 
                               size_t* count);
bh_status_t dt_get_interrupt_parent(const struct device_tree_node* node, struct device_tree_node** parent);
bh_status_t dt_map_interrupt(const struct device_tree_node* node, const struct dt_interrupt_info* interrupt, 
                            uint32_t* hwirq);

// GPIO operations
bh_status_t dt_get_gpios(const struct device_tree_node* node, struct dt_gpio_info** gpios, size_t* count);
bh_status_t dt_get_gpio_by_name(const struct device_tree_node* node, const char* name, struct dt_gpio_info* gpio);

// Clock operations
bh_status_t dt_get_clocks(const struct device_tree_node* node, struct dt_clock_info** clocks, size_t* count);
bh_status_t dt_get_clock_by_name(const struct device_tree_node* node, const char* name, struct dt_clock_info* clock);
bh_status_t dt_enable_clock(const struct device_tree_node* node, uint32_t clock);
bh_status_t dt_disable_clock(const struct device_tree_node* node, uint32_t clock);

// Regulator operations
bh_status_t dt_get_regulators(const struct device_tree_node* node, struct dt_regulator_info** regulators, 
                              size_t* count);
bh_status_t dt_get_regulator_by_name(const struct device_tree_node* node, const char* name, 
                                     struct dt_regulator_info* regulator);
bh_status_t dt_enable_regulator(const struct device_tree_node* node, const char* name);
bh_status_t dt_disable_regulator(const struct device_tree_node* node, const char* name);

// Reset operations
bh_status_t dt_get_resets(const struct device_tree_node* node, struct dt_reset_info** resets, size_t* count);
bh_status_t dt_get_reset_by_name(const struct device_tree_node* node, const char* name, struct dt_reset_info* reset);
bh_status_t dt_assert_reset(const struct device_tree_node* node, const char* name);
bh_status_t dt_deassert_reset(const struct device_tree_node* node, const char* name);

// DMA operations
bh_status_t dt_get_dmas(const struct device_tree_node* node, struct dt_dma_info** dmas, size_t* count);
bh_status_t dt_get_dma_by_name(const struct device_tree_node* node, const char* name, struct dt_dma_info* dma);

// PHY operations
bh_status_t dt_get_phys(const struct device_tree_node* node, struct dt_phy_info** phys, size_t* count);
bh_status_t dt_get_phy_by_name(const struct device_tree_node* node, const char* name, struct dt_phy_info* phy);

// PCI operations
bh_status_t dt_get_pci_ranges(const struct device_tree_node* node, struct dt_pci_info** ranges, size_t* count);
bh_status_t dt_translate_pci_address(const struct device_tree_node* node, uint64_t pci_address, 
                                     uint64_t* cpu_address, uint64_t* size);

// I2C operations
bh_status_t dt_get_i2c_devices(const struct device_tree_node* node, struct dt_i2c_info** devices, size_t* count);
bh_status_t dt_get_i2c_device_by_address(const struct device_tree_node* node, uint32_t address, 
                                         struct dt_i2c_info* device);

// SPI operations
bh_status_t dt_get_spi_devices(const struct device_tree_node* node, struct dt_spi_info** devices, size_t* count);
bh_status_t dt_get_spi_device_by_chip_select(const struct device_tree_node* node, uint32_t chip_select, 
                                            struct dt_spi_info* device);

// PWM operations
bh_status_t dt_get_pwms(const struct device_tree_node* node, struct dt_pwm_info** pwms, size_t* count);
bh_status_t dt_get_pwm_by_name(const struct device_tree_node* node, const char* name, struct dt_pwm_info* pwm);

// Thermal operations
bh_status_t dt_get_thermal_zones(const struct device_tree_node* root, struct dt_thermal_info** zones, 
                                size_t* count);
bh_status_t dt_get_thermal_zone_by_name(const struct device_tree_node* root, const char* name, 
                                       struct dt_thermal_info* zone);
bh_status_t dt_get_temperature(const struct device_tree_node* node, int32_t* temperature);

// Display operations
bh_status_t dt_get_displays(const struct device_tree_node* root, struct dt_display_info** displays, 
                           size_t* count);
bh_status_t dt_get_display_by_name(const struct device_tree_node* root, const char* name, 
                                   struct dt_display_info* display);
bh_status_t dt_get_framebuffer(const struct device_tree_node* node, uint32_t* address, uint32_t* size);

// Sound operations
bh_status_t dt_get_sound_devices(const struct device_tree_node* root, struct dt_sound_info** devices, 
                                size_t* count);
bh_status_t dt_get_sound_device_by_name(const struct device_tree_node* root, const char* name, 
                                        struct dt_sound_info* device);

// Video operations
bh_status_t dt_get_video_devices(const struct device_tree_node* root, struct dt_video_info** devices, 
                                size_t* count);
bh_status_t dt_get_video_device_by_name(const struct device_tree_node* root, const char* name, 
                                        struct dt_video_info* device);

// Network operations
bh_status_t dt_get_network_devices(const struct device_tree_node* root, struct dt_network_info** devices, 
                                  size_t* count);
bh_status_t dt_get_network_device_by_name(const struct device_tree_node* root, const char* name, 
                                          struct dt_network_info* device);

// Storage operations
bh_status_t dt_get_storage_devices(const struct device_tree_node* root, struct dt_storage_info** devices, 
                                   size_t* count);
bh_status_t dt_get_storage_device_by_name(const struct device_tree_node* root, const char* name, 
                                         struct dt_storage_info* device);

// Input operations
bh_status_t dt_get_input_devices(const struct device_tree_node* root, struct dt_input_info** devices, 
                                size_t* count);
bh_status_t dt_get_input_device_by_name(const struct device_tree_node* root, const char* name, 
                                        struct dt_input_info* device);

// Power management operations
bh_status_t dt_get_power_info(const struct device_tree_node* root, struct dt_power_info* info);
bh_status_t dt_set_power_state(const struct device_tree_node* node, const char* state);

// Security operations
bh_status_t dt_get_security_info(const struct device_tree_node* root, struct dt_security_info* info);
bh_status_t dt_verify_secure_boot(const struct device_tree_node* root, bool* verified);

// Debug operations
bh_status_t dt_get_debug_info(const struct device_tree_node* root, struct dt_debug_info* info);
bh_status_t dt_enable_debug(const struct device_tree_node* node, bool enable);

// Iterator operations
bh_status_t dt_iterator_init(struct device_tree_iterator* iterator, struct device_tree_node* root, 
                            uint32_t flags);
bh_status_t dt_iterator_next(struct device_tree_iterator* iterator, struct device_tree_node** node);
bh_status_t dt_iterator_reset(struct device_tree_iterator* iterator);
void dt_iterator_cleanup(struct device_tree_iterator* iterator);

// Utility operations
bh_status_t dt_print_tree(const struct device_tree_node* root);
bh_status_t dt_print_node(const struct device_tree_node* node, int depth);
bh_status_t dt_print_property(const struct device_tree_property* property);
bh_status_t dt_compare_trees(const struct device_tree_node* tree1, const struct device_tree_node* tree2, 
                             bool* identical);
bh_status_t dt_merge_trees(struct device_tree_node* dest, const struct device_tree_node* src);

// Platform-specific operations
bh_status_t dt_load_from_uboot(void** blob, size_t* size);
bh_status_t dt_load_from_openfirmware(void** blob, size_t* size);
bh_status_t dt_save_to_uboot(const void* blob, size_t size);
bh_status_t dt_save_to_openfirmware(const void* blob, size_t size);

// Error handling
const char* dt_error_string(bh_status_t status);
bh_status_t dt_get_last_error(char* buffer, size_t size);

#endif /* _DEVICETREE_H_ */
