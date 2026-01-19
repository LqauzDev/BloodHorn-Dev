# BloodHorn Coreboot Configuration File
# This file contains configuration options for Coreboot firmware integration

# Coreboot source configuration
COREBOOT_VERSION=v4.22
COREBOOT_SOURCE_URL=https://github.com/coreboot/coreboot.git
COREBOOT_CONFIG=mainboard/google/octopus

# Payload configuration
PAYLOAD_NAME=BloodHorn
PAYLOAD_BINARY=bloodhorn.elf
PAYLOAD_CMDLINE="console=ttyS0,115200n8 root=/dev/sda1 ro quiet splash"

# Build options
BUILD_TYPE=release
ENABLE_DEBUG=false
ENABLE_TESTS=false
ENABLE_CCACHE=true

# Hardware configuration
CPU_MICROCODE_UPDATE=true
SMM_SUPPORT=true
ACPI_SUPPORT=true
SMBIOS_SUPPORT=true

# Graphics configuration
FRAMEBUFFER_KEEP_VESA_MODE=false
FRAMEBUFFER_NATIVE_RESOLUTION=1920x1080
FRAMEBUFFER_DEPTH=32

# Security configuration
TPM_SUPPORT=true
SECURE_BOOT=true
MEASURED_BOOT=true
VERIFIED_BOOT=true

# Device support
USB_SUPPORT=true
SATA_SUPPORT=true
NVME_SUPPORT=true
NETWORK_SUPPORT=true
PCI_SUPPORT=true

# Mainboard specific options
MAINBOARD_VENDOR=google
MAINBOARD_MODEL=octopus
MAINBOARD_VARIANT=octopus

# Build directories
BUILD_DIR=build
OUTPUT_DIR=output
TOOLCHAIN_DIR=/opt/coreboot-sdk

# Compiler options
CC=gcc
CFLAGS=-O2 -pipe -Wall -Werror -march=x86-64 -mtune=generic
LDFLAGS=-Wl,--gc-sections -Wl,--strip-all

# Coreboot make options (only set if not already defined by board config)
COREBOOT_MAKE_OPTS=" \
    CPUS=$(nproc) \
    CONFIG_PAYLOAD_FILE=$(BUILD_DIR)/$(PAYLOAD_BINARY) \
    CONFIG_PAYLOAD_CMDLINE=\"$(PAYLOAD_CMDLINE)\" \
    CONFIG_CCACHE=$(ENABLE_CCACHE) \
"
# Optional features - only add if coreboot version supports them
ifeq ($(shell test -f $(COREBOOT_SOURCE_DIR)/src/security/tpm/tss/tcg-2.0/Makefile.inc && echo yes),yes)
COREBOOT_MAKE_OPTS += "CONFIG_TPM2=$(TPM_SUPPORT)"
endif
ifeq ($(shell test -f $(COREBOOT_SOURCE_DIR)/src/security/verified_boot/Makefile.inc && echo yes),yes)
COREBOOT_MAKE_OPTS += "CONFIG_VERIFIED_BOOT=$(VERIFIED_BOOT)"
endif
ifeq ($(shell test -f $(COREBOOT_SOURCE_DIR)/src/drivers/usb/Makefile.inc && echo yes),yes)
COREBOOT_MAKE_OPTS += "CONFIG_USB=$(USB_SUPPORT)"
endif
ifeq ($(shell test -f $(COREBOOT_SOURCE_DIR)/src/drivers/sata/Makefile.inc && echo yes),yes)
COREBOOT_MAKE_OPTS += "CONFIG_SATA=$(SATA_SUPPORT)"
endif
ifeq ($(shell test -f $(COREBOOT_SOURCE_DIR)/src/drivers/nvme/Makefile.inc && echo yes),yes)
COREBOOT_MAKE_OPTS += "CONFIG_NVME=$(NVME_SUPPORT)"
endif
ifeq ($(shell test -f $(COREBOOT_SOURCE_DIR)/src/drivers/pci/Makefile.inc && echo yes),yes)
COREBOOT_MAKE_OPTS += "CONFIG_PCI=$(PCI_SUPPORT)"
endif
