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
CFLAGS=-O2 -pipe -Wall -Werror -march=native
LDFLAGS=-Wl,--gc-sections -Wl,--strip-all

# Coreboot make options
COREBOOT_MAKE_OPTS=" \
    CPUS=$(nproc) \
    CONFIG_PAYLOAD_FILE=$(BUILD_DIR)/$(PAYLOAD_BINARY) \
    CONFIG_PAYLOAD_CMDLINE=\"$(PAYLOAD_CMDLINE)\" \
    CONFIG_CCACHE=$(ENABLE_CCACHE) \
    CONFIG_TPM2=$(TPM_SUPPORT) \
    CONFIG_SECURE_BOOT=$(SECURE_BOOT) \
    CONFIG_MEASURED_BOOT=$(MEASURED_BOOT) \
    CONFIG_VERIFIED_BOOT=$(VERIFIED_BOOT) \
    CONFIG_USB=$(USB_SUPPORT) \
    CONFIG_SATA=$(SATA_SUPPORT) \
    CONFIG_NVME=$(NVME_SUPPORT) \
    CONFIG_NETWORK=$(NETWORK_SUPPORT) \
    CONFIG_PCI=$(PCI_SUPPORT) \
"
