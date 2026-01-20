# BloodHorn Installation Guide

This guide explains how to build and install BloodHorn bootloader using the automated build system.

## Quick Start

```bash
# Clone the repository
git clone https://codeberg.org/PacHashs/BloodHorn.git
cd BloodHorn

# Build with EDK2 (automatically downloads dependencies)
make x64

# Or build all architectures
make all

# Install BloodHorn.efi
make install
```

## Build System

The new Makefile automatically handles EDK2 setup and compilation:

- **Automatic EDK2 download**: Clones and sets up EDK2 environment
- **Multi-architecture support**: Build for x64, IA32, ARM64, RISC-V, LoongArch
- **Dependency management**: Handles all submodules and toolchain setup
- **Clean targets**: Easy cleanup and rebuild options

### Build Targets

- `make x64` - Build for X64 (default)
- `make ia32` - Build for IA32
- `make aarch64` - Build for ARM64
- `make riscv64` - Build for RISC-V 64-bit
- `make loongarch64` - Build for LoongArch 64-bit
- `make all` - Build all architectures
- `make clean` - Clean build artifacts
- `make install` - Install BloodHorn.efi
- `make distclean` - Clean everything including EDK2

### Build Variables

- `EDK2_URL` - EDK2 repository URL
- `EDK2_BRANCH` - EDK2 branch to use
- `BUILD_DIR` - Build directory
- `TOOLCHAIN` - EDK2 toolchain (GCC5, CLANG, etc.)

## Installation

### UEFI Installation

```bash
# Method 1: EFI System Partition (Recommended)
sudo mount /dev/sda1 /mnt/efi
sudo mkdir -p /mnt/efi/EFI/BloodHorn
sudo cp BloodHorn.efi /mnt/efi/EFI/BloodHorn/
sudo efibootmgr -c -d /dev/sda -p 1 -l "\\EFI\\BloodHorn\\BloodHorn.efi" -L "BloodHorn"
sudo efibootmgr -o 0000,0001,0002  # BloodHorn first
sudo umount /mnt/efi

# Method 2: Replace Default Bootloader
sudo cp /boot/efi/EFI/BOOT/BOOTX64.EFI /boot/efi/EFI/BOOT/BOOTX64.EFI.backup
sudo cp BloodHorn.efi /boot/efi/EFI/BOOT/BOOTX64.EFI
sudo reboot
```

### USB Installation

```bash
# Create bootable USB
sudo dd if=BloodHorn.efi of=/dev/sdb bs=4M status=progress
sudo sync
sudo eject /dev/sdb
```

### Network Boot

Set up PXE/TFTP server and configure DHCP to serve BloodHorn.efi to network boot clients.

## Configuration

Place `bloodhorn.ini` or `bloodhorn.json` in the same directory as BloodHorn.efi to customize:

```ini
[theme]
background_color=0x1A1A2E
header_color=0x2D2D4F
language=en

[security]
secure_boot=enabled
verify_signatures=true
```

## Troubleshooting

### Common Issues

**BloodHorn won't boot:**
- Check UEFI settings and disable Secure Boot if needed
- Verify EFI partition is FAT32 formatted
- Ensure BloodHorn.efi is in correct EFI directory

**Build failures:**
- Run `make distclean` then rebuild
- Check EDK2 environment setup
- Verify all dependencies are installed

**Network boot issues:**
- Verify DHCP/TFTP server configuration
- Check network connectivity
- Test with manual TFTP download

For detailed troubleshooting and advanced installation options, see the source code documentation.