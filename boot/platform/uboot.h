/*
 * uboot.h - U-Boot platform support for BloodHorn
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef _UBOOT_H_
#define _UBOOT_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../libb/include/bloodhorn/bootinfo.h"

// U-Boot signature and version information
#define UBOOT_MAGIC_SIGNATURE    0x55424F4F544F4F42ULL  // "BOOTOOBU"
#define UBOOT_VERSION_MIN        0x20230000  // Minimum supported U-Boot version

// U-Boot boot information structure (as passed from U-Boot)
struct uboot_boot_info {
    uint32_t magic;              // Magic number "UBOOT"
    uint32_t version;            // U-Boot version
    uint32_t device_tree_addr;  // Physical address of device tree
    uint32_t device_tree_size;  // Size of device tree
    uint32_t cmdline_addr;       // Physical address of command line
    uint32_t cmdline_size;       // Size of command line
    uint32_t initrd_addr;        // Physical address of initrd
    uint32_t initrd_size;        // Size of initrd
    uint32_t memory_size;        // Total memory size
    uint32_t flags;              // Boot flags
    uint32_t reserved[8];       // Reserved for future use
} __attribute__((packed));

// U-Boot boot flags
#define UBOOT_FLAG_CMDLINE_VALID   (1 << 0)
#define UBOOT_FLAG_INITRD_VALID    (1 << 1)
#define UBOOT_FLAG_FDT_VALID       (1 << 2)
#define UBOOT_FLAG_SECURE_BOOT     (1 << 3)
#define UBOOT_FLAG_ELF_BOOT        (1 << 4)
#define UBOOT_FLAG_64BIT           (1 << 5)

// U-Boot global data structure (simplified)
struct uboot_global_data {
    uint32_t flags;              // Global flags
    uint32_t baudrate;           // Serial baud rate
    uint32_t have_console;       // Console is available
    uint32_t env_addr;           // Environment address
    uint32_t env_valid;          // Environment is valid
    uint32_t fdt_addr;           // Device tree address
    uint32_t fdt_size;           // Device tree size
    uint32_t relocaddr;          // Relocation address
    uint32_t start_addr_sp;      // Stack pointer
    uint32_t ram_size;           // RAM size
    uint32_t mon_len;            // Monitor length
    uint32_t irq_sp;             // IRQ stack pointer
    uint32_t bd;                 // Board data pointer
    uint32_t jt;                 // Jump table
    uint32_t post_word;          // POST word
    uint32_t arch;               // Architecture
    uint32_t cpu;                // CPU type
    uint32_t board_type;         // Board type
    uint32_t board_rev;          // Board revision
    uint32_t reserved[16];       // Reserved fields
} __attribute__((packed));

// U-Boot board information structure
struct uboot_bdinfo {
    uint32_t bi_memstart;        // Memory start
    uint32_t bi_memsize;         // Memory size
    uint32_t bi_flashstart;      // Flash start
    uint32_t bi_flashsize;       // Flash size
    uint32_t bi_flashoffset;     // Flash offset
    uint32_t bi_sramstart;       // SRAM start
    uint32_t bi_sramsize;        // SRAM size
    uint32_t bi_bootflags;       // Boot flags
    uint32_t bi_ip_addr;         // IP address
    uint32_t bi_ethspeed;        // Ethernet speed
    uint32_t bi_intfreq;         // Internal frequency
    uint32_t bi_busfreq;         // Bus frequency
    uint32_t bi_baudrate;        // Baud rate
    char bi_cmdline[256];        // Command line
    uint32_t bi_boot_device;     // Boot device
    uint32_t bi_boot_file[32];   // Boot file name
    uint32_t bi_dram[8];         // DRAM banks
} __attribute__((packed));

// Function prototypes
bh_status_t uboot_detect_platform(void);
bh_status_t uboot_initialize_platform(bh_boot_info_t* boot_info);
bh_status_t uboot_parse_boot_info(const struct uboot_boot_info* uboot_info, bh_boot_info_t* boot_info);
bh_status_t uboot_get_global_data(struct uboot_global_data** gd);
bh_status_t uboot_get_bdinfo(struct uboot_bdinfo** bd);
bh_status_t uboot_get_device_tree(void** fdt, size_t* size);
bh_status_t uboot_get_command_line(char** cmdline, size_t* size);
bh_status_t uboot_get_initrd(void** initrd, size_t* size);
bh_status_t uboot_setup_memory_map(bh_boot_info_t* boot_info);
bh_status_t uboot_setup_console(void);
bh_status_t uboot_print_info(void);

// U-Boot environment variable access
bh_status_t uboot_env_get(const char* name, char* value, size_t size);
bh_status_t uboot_env_set(const char* name, const char* value);
bh_status_t uboot_env_list(void (*callback)(const char* name, const char* value, void* data), void* data);

// U-Boot device tree utilities
bh_status_t uboot_fdt_getprop(const void* fdt, const char* path, const char* prop, void* value, size_t* size);
bh_status_t uboot_fdt_setprop(void* fdt, const char* path, const char* prop, const void* value, size_t size);
bh_status_t uboot_fdt_get_node_offset(const void* fdt, const char* path, int* offset);
bh_status_t uboot_fdt_get_string(const void* fdt, int offset, const char* prop, char* str, size_t size);

// U-Boot console I/O
void uboot_putc(char c);
void uboot_puts(const char* s);
int uboot_getc(void);
int uboot_tstc(void);

// U-Boot memory allocation
void* uboot_malloc(size_t size);
void uboot_free(void* ptr);
void* uboot_realloc(void* ptr, size_t size);

// U-Boot timing
uint64_t uboot_get_timer(void);
void uboot_udelay(unsigned long usec);
void uboot_mdelay(unsigned long msec);

// U-Boot architecture-specific functions
bh_status_t uboot_arch_init(void);
bh_status_t uboot_arch_cleanup(void);
void uboot_reset(void);
void uboot_power_off(void);

#endif /* _UBOOT_H_ */
