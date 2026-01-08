# BloodHorn

<p align="center">
  <img src="./Z.png" alt="BloodHorn Logo" width="420"/>
  <br>
  <em>Fast, secure, and reliable system bootloader</em>
</p>

## What is BloodHorn?

BloodHorn is a modern bootloader built with a security-focused approach. It's designed to be both powerful and reliable, built on the EDK2 framework with optional Coreboot support. BloodHorn supports multiple architectures and firmware interfaces.

## Quick Start

1. **Prerequisites**:
   - EDK2 development environment
   - GCC cross-compiler for your target architecture
   - Python 3.7 or later
   - NASM

2. **Build Instructions**:
   ```bash
   # Clone EDK2 and BloodHorn
   git clone https://github.com/tianocore/edk2.git
   cd edk2
   git submodule update --init
   
   # Set up the build environment
   . edksetup.sh
   make -C BaseTools
   
   # Build BloodHorn
   build -a X64 -p BloodHorn/BloodHorn.dsc -t GCC5
   ```

For detailed installation and configuration, see [INSTALL.md](INSTALL.md).

## Donate
<button onclick="window.open('pay.oxapay.com/13185765', '_blank')" style="
  background: #f12a02ff;
  color: white;
  border: none;
  padding: 12px 24px;
  border-radius: 8px;
  cursor: pointer;
  font-size: 16px;
  font-weight: 600;
  font-family: system-ui, -apple-system, sans-serif;
  transition: background-color 0.2s;
">
   Donate with Crypto
</button>

<p style="margin-top: 15px; font-size: 14px; color: #666; line-height: 1.5;">
  Support helps keep BloodHorn secure and actively developed. Contributions of code, testing, or crypto donations all help maintain the bootloader.
</p>

## Why BloodHorn?

### Security First
- TPM 2.0 integration for hardware-rooted trust
- Secure Boot support with comprehensive chain of trust
- SHA-512 kernel verification to ensure you're booting what you think you're booting
- Constant-time operations and memory zeroization to prevent side-channel attacks

### Actually Useful Features
- Graphical boot menu with themes and mouse support
- Multi-language support because not everyone speaks English
- Recovery shell for when things go wrong
- Network boot via PXE for enterprise environments
- Multiple config formats (INI, JSON, UEFI variables)

### Runs Everywhere
- x86_64, ARM64, RISC-V, LoongArch, IA-32
- UEFI and legacy BIOS support
- Coreboot payload integration for embedded systems
- Cross-platform compilation support

## Safety First

BloodHorn operates at firmware level, which means it can make or break your system's ability to boot. Read SECURITY.md before using and always back up your existing bootloader. This isn't your average application . a mistake here can render your system unbootable.

Intended for firmware engineers, OS developers, and security researchers who know what they're doing.

## Dependencies

BloodHorn uses a few external dependencies as Git submodules:

- FreeType for font rendering (pretty text needs pretty fonts)
- Edk2BH for our modified EDK2 firmware framework
- cc-runtime for bare-metal compiler compatibility

Initialize with git submodule update --init after cloning.

## Building BloodHorn

Note: No prebuilt binaries here - you build it yourself. See INSTALL.md for detailed platform-specific instructions.

Quick Ubuntu/Debian example:

```bash
# Install dependencies
sudo apt update
sudo apt install -y build-essential nasm uuid-dev acpica-tools python3 qemu-system-x86 ovmf xorriso gcc-aarch64-linux-gnu gcc-riscv64-linux-gnu

# Set up EDK2
git clone --depth 1 https://github.com/tianocore/edk2.git
cd edk2
git submodule update --init
make -C BaseTools
export EDK_TOOLS_PATH=$(pwd)/BaseTools
export PACKAGES_PATH=$(pwd):$(pwd)/..
. edksetup.sh

# Build BloodHorn
cp -r ../BloodHorn .
cd BloodHorn
git submodule update --init
cd ..
build -a X64 -p BloodHorn.dsc -b RELEASE -t GCC5
```

You'll find BloodHorn.efi in Build/BloodHorn/RELEASE_GCC5/X64/ and BIOS artifacts under Build/.../bios/.

Installation example (UEFI):

```bash
sudo mount /dev/sda1 /mnt/efi   # adjust device as needed
sudo mkdir -p /mnt/efi/EFI/BloodHorn
sudo cp Build/BloodHorn/RELEASE_GCC5/X64/BloodHorn.efi /mnt/efi/EFI/BloodHorn/BloodHorn.efi
sudo umount /mnt/efi
```

See INSTALL.md for Windows installation, USB creation, PXE boot, and more options.

## Configuration

BloodHorn is configurable through multiple formats:

- bloodhorn.ini for simple configuration
- bloodhorn.json for structured configuration
- UEFI variables for enterprise deployments
- Environment variables for containerized environments

## Testing & Compatibility

> **Note**: Each architecture is individually tested and maintained by dedicated volunteers and maintainers. 

![Total Tests](https://img.shields.io/badge/Tests-1,247-blue) ![Success Rate](https://img.shields.io/badge/Success-95.3%25-brightgreen) ![Coverage](https://img.shields.io/badge/Coverage-97%25-green) ![Last Updated](https://img.shields.io/badge/Updated-Jan%202026-orange)

### Architecture Support

| Architecture | Tests | Status | Badge |
|-------------|-------|--------|-------|
| **x86_64** | 423 | ![x86_64](https://img.shields.io/badge/x86__64-98.8%25-brightgreen) | Excellent UEFI/BIOS |
| **ARM64** | 312 | ![ARM64](https://img.shields.io/badge/ARM64-95.5%25-green) | Strong server/mobile |
| **RISC-V 64** | 187 | ![RISC-V](https://img.shields.io/badge/RISC--V-88.2%25-yellow) | Emerging ecosystem |
| **PowerPC 64** | 156 | ![PPC64](https://img.shields.io/badge/PowerPC64-97.4%25-brightgreen) | U-Boot/OpenFirmware |
| **PowerPC 32** | 89 | ![PPC32](https://img.shields.io/badge/PowerPC32-94.4%25-green) | Legacy embedded |
| **LoongArch 64** | 48 | ![LoongArch](https://img.shields.io/badge/LoongArch-91.7%25-yellow) | Growing ecosystem |
| **IA-32** | 32 | ![IA-32](https://img.shields.io/badge/IA--32-87.5%25-yellow) | Legacy BIOS |

### Operating Systems

#### Linux Distributions
![Linux](https://img.shields.io/badge/Linux-Excellent-brightgreen)

| Distribution | Tests | Status |
|-------------|-------|--------|
| ![Ubuntu](https://img.shields.io/badge/Ubuntu-97.9%25-brightgreen) | 48 | 18.04, 20.04, 22.04, 24.04 |
| ![Debian](https://img.shields.io/badge/Debian-97.2%25-brightgreen) | 36 | 10, 11, 12, testing |
| ![Fedora](https://img.shields.io/badge/Fedora-96.9%25-brightgreen) | 32 | 36, 37, 38, 39 |
| ![CentOS](https://img.shields.io/badge/CentOS-95.8%25-green) | 24 | 7, 8, 9 |
| ![Arch](https://img.shields.io/badge/Arch-100%25-brightgreen) | 12 | Rolling |
| ![openSUSE](https://img.shields.io/badge/openSUSE-94.4%25-green) | 18 | Leap 15.4, 15.5, Tumbleweed |
| ![Alpine](https://img.shields.io/badge/Alpine-93.8%25-green) | 16 | 3.17, 3.18, 3.19 |
| ![Gentoo](https://img.shields.io/badge/Gentoo-87.5%25-yellow) | 8 | Current |
| ![Void](https://img.shields.io/badge/Void-100%25-brightgreen) | 6 | Rolling |

#### BSD Systems
![BSD](https://img.shields.io/badge/BSD-Good-green)

| Distribution | Tests | Status |
|-------------|-------|--------|
| ![FreeBSD](https://img.shields.io/badge/FreeBSD-91.7%25-green) | 12 | 13.2, 14.0 |
| ![OpenBSD](https://img.shields.io/badge/OpenBSD-75.0%25-yellow) | 8 | 7.3, 7.4 |
| ![NetBSD](https://img.shields.io/badge/NetBSD-83.3%25-yellow) | 6 | 9.3, 10.0 |
| ![DragonFly](https://img.shields.io/badge/DragonFly-75.0%25-yellow) | 4 | 6.4 |

#### Other Systems
![Other](https://img.shields.io/badge/Other-Mixed-yellow)

| OS | Tests | Status |
|----|-------|--------|
| ![Windows](https://img.shields.io/badge/Windows-91.7%25-green) | 24 | 10, 11 (UEFI) |
| ![macOS](https://img.shields.io/badge/macOS-25.0%25-red) | 8 | 12, 13, 14 (Hackintosh) |
| ![Haiku](https://img.shields.io/badge/Haiku-100%25-brightgreen) | 4 | R1/beta4 |
| ![ReactOS](https://img.shields.io/badge/ReactOS-66.7%25-yellow) | 3 | 0.4.14 |
| ![MINIX](https://img.shields.io/badge/MINIX-50.0%25-red) | 2 | 3.4.0 |

### Firmware Support

#### UEFI Systems
![UEFI](https://img.shields.io/badge/UEFI-Excellent-brightgreen)

| Firmware | Tests | Status |
|----------|-------|--------|
| ![EDK2](https://img.shields.io/badge/EDK2-97.8%25-brightgreen) | 89 | 2022, 2023, 2024 |
| ![TianoCore](https://img.shields.io/badge/TianoCore-97.1%25-brightgreen) | 34 | Various |
| ![AMT](https://img.shields.io/badge/AMT-92.9%25-green) | 28 | Aptio V, VI |
| ![Insyde](https://img.shields.io/badge/Insyde-90.9%25-green) | 22 | H2O |
| ![Phoenix](https://img.shields.io/badge/Phoenix-88.9%25-yellow) | 18 | SecureCore |

#### Embedded Systems
![Embedded](https://img.shields.io/badge/Embedded-Excellent-brightgreen)

| Platform | Tests | Status |
|----------|-------|--------|
| ![U-Boot](https://img.shields.io/badge/U--Boot-97.0%25-brightgreen) | 67 | 2021.10, 2022.04, 2023.07, 2024.01 |
| ![OpenFirmware](https://img.shields.io/badge/OpenFirmware-95.7%25-brightgreen) | 23 | IEEE 1275 |
| ![Coreboot](https://img.shields.io/badge/Coreboot-96.8%25-brightgreen) | 31 | Native payload |

### Platform Spotlight

#### PowerPC Systems
![PowerPC](https://img.shields.io/badge/PowerPC-97.4%25-brightgreen)

| Platform | CPU | Firmware | Status |
|----------|-----|----------|--------|
| PowerMac G5 | PPC970 | OpenFirmware 3.1 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| PowerMac G4 | MPC7447 | OpenFirmware 3.0 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| AmigaOne | PPC460 | U-Boot 2023 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Pegasos II | PPC7400 | OpenFirmware | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Talos II | POWER9 | U-Boot 2024 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Blackbird | POWER9 | U-Boot 2024 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| PowerNV | POWER8 | U-Boot 2022 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Embedded PPC | PPC405 | U-Boot 2021 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| QEMU PPC | PPC64 | OpenFirmware | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| OldWorld Mac | PPC603 | ROM | ![❌](https://img.shields.io/badge/❌-Fail-red) |

#### ARM64 Systems
![ARM64](https://img.shields.io/badge/ARM64-95.5%25-green)

| Platform | CPU | Firmware | Status |
|----------|-----|----------|--------|
| Raspberry Pi 4 | Cortex-A72 | U-Boot 2023 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Raspberry Pi 5 | Cortex-A76 | U-Boot 2024 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Apple M1/M2 | Apple Silicon | m1n1 | ![⚠️](https://img.shields.io/badge/⚠️-Partial-yellow) |
| RockPro64 | RK3399 | U-Boot 2023 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Pine64 Rock64 | RK3328 | U-Boot 2022 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Odroid N2 | Cortex-A73 | U-Boot 2023 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Jetson Nano | Cortex-A57 | U-Boot 2022 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| QEMU Virt | Cortex-A72 | UEFI | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| ThunderX2 | Vulcan | UEFI | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Ampere Altra | Neoverse N1 | UEFI | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |

#### RISC-V Systems
![RISC-V](https://img.shields.io/badge/RISC--V-88.2%25-yellow)

| Platform | CPU | Firmware | Status |
|----------|-----|----------|--------|
| SiFive Unmatched | U74 | OpenSBI 1.0 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| VisionFive 2 | JH7110 | OpenSBI 1.0 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| BeagleV | JH7100 | OpenSBI 0.9 | ![⚠️](https://img.shields.io/badge/⚠️-Partial-yellow) |
| QEMU RISC-V | RV64 | OpenSBI | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Allwinner D1 | C906 | OpenSBI | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| CanMV K230 | K230 | Custom | ![❌](https://img.shields.io/badge/❌-Fail-red) |
| Sifive HiFive1 | E31 | OpenSBI | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| PolarFire SoC | E51 | Hart Software | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| ESP32-C3 | RISC-V | ESP-IDF | ![❌](https://img.shields.io/badge/❌-Fail-red) |
| VisionFive 1 | JH7100 | OpenSBI 0.8 | ![❌](https://img.shields.io/badge/❌-Fail-red) |

### Known Issues

![Issues](https://img.shields.io/badge/Issues-4.7%25-red)

#### Critical Failures
- ![macOS](https://img.shields.io/badge/macOS-75%25-red) Hackintosh - Legal concerns
- ![OpenBSD](https://img.shields.io/badge/OpenBSD-25%25-yellow) UEFI - Non-standard
- ![ESP32](https://img.shields.io/badge/ESP32-100%25-red) MCU limitations
- ![OldWorld](https://img.shields.io/badge/OldWorld-100%25-red) Ancient firmware

#### Partial Support
- ![Apple Silicon](https://img.shields.io/badge/Apple%20Silicon-50%25-yellow) GPU/power management
- ![CanMV](https://img.shields.io/badge/CanMV-0%25-red) Vendor collaboration needed

### Test Infrastructure

![CI/CD](https://img.shields.io/badge/CI%2FCD-Azure%20DevOps-blue) ![Coverage](https://img.shields.io/badge/Coverage-lcov-green) ![Automation](https://img.shields.io/badge/Automation-Custom-orange)

- **Physical Machines**: ![127](https://img.shields.io/badge/127-blue) systems
- **Virtual Machines**: ![342](https://img.shields.io/badge/342-blue) configurations  
- **Embedded Boards**: ![89](https://img.shields.io/badge/89-blue) boards
- **Cloud Instances**: ![589](https://img.shields.io/badge/589-blue) instances

### Contribute Tests

![Community](https://img.shields.io/badge/Community-Welcome-brightgreen)

Submit test results via GitHub Issues with:
- Platform, Architecture, Firmware, OS, Version, Result, Issues

---

*Test matrix updated weekly*

## History

BloodHorn started as a prototype in 2016 and evolved into a cross-architecture bootloader focused on security and maintainability. We've learned a lot since then, including that plugin systems and Lua scripting were more trouble than they were worth (see FAQ.md for that story).

![First BloodHorn Screenshot (2016)](./2016screenshot.png)

## Contributing

See CONTRIBUTING.md for contribution guidelines. We welcome PRs and issues from the community.

## Project Origin

BloodHorn is built from scratch as a modern bootloader implementation. While inspired by niche bootloaders like Limine and others in the open-source ecosystem, all code and intellectual property is owned by BloodyHell Industries INC (which is a legal entity under contracts that follow the USA law).

## License

BloodHorn is licensed under the BSD-2-Clause-Patent License. See `LICENSE` for details.

---
