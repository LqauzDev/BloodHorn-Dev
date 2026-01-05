# BloodHorn

<p align="center">
  <img src="./Z.png" alt="BloodHorn Logo" width="420"/>
  <br>
  <em>Fast, secure, and reliable system bootloader</em>
</p>

## What is BloodHorn?

BloodHorn is a modern bootloader built with a security + features + compact philosophy. We believe bootloaders should be secure without sacrificing usability, and feature-rich without being bloated. Built on EDK2 with optional Coreboot support, BloodHorn runs on pretty much anything with a CPU.

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

## Project Structure

- coreboot/ - Coreboot payload integration
- uefi/ - UEFI-specific code
- security/ - Crypto, TPM, secure boot implementation
- fs/ - Filesystem support (FAT32 primarily)
- net/ - Network boot capabilities
- rust/ - Internal no_std crates for safe memory management
- boot/ - Main bootloader logic and UI

## Configuration

BloodHorn is configurable through multiple formats:

- bloodhorn.ini for simple configuration
- bloodhorn.json for structured configuration
- UEFI variables for enterprise deployments
- Environment variables for containerized environments

Place config files in the boot partition root. Check CONFIG.md for all options.

## History

BloodHorn started as a prototype in 2016 and evolved into a cross-architecture bootloader focused on security and maintainability. We've learned a lot since then, including that plugin systems and Lua scripting were more trouble than they were worth (see FAQ.md for that story).

![First BloodHorn Screenshot (2016)](./2016screenshot.png)

## Contributing

See CONTRIBUTING.md for contribution guidelines. We welcome PRs and issues from the community.

## CI/CD

We run CI builds and tests on Azure DevOps. Check the CI dashboard for current status.

## Project Origin

BloodHorn is built from scratch as a modern bootloader implementation. While inspired by niche bootloaders like Limine and others in the open-source ecosystem, all code and intellectual property is owned by BloodyHell Industries INC (which is a legal entity under contracts that follow the USA law).

This project was developed exclusively by human developers through manual coding processes. No artificial intelligence tools, automated code generation systems, or third-party code synthesis services were utilized in any phase of development. All source code represents original intellectual work created by the development team at BloodyHell Industries INC. We maintain complete authorship and copyright ownership of all codebase components.

BloodHorn Industries INC hereby affirms under penalty of perjury that this project constitutes entirely original software development. All source code, algorithms, implementations, and architectural designs are the product of human intellectual effort by our development team. The project contains no copied, derived, or synthesized code from any external sources beyond standard protocol implementations and legally incorporated dependencies.

Under 17 U.S.C. § 102 and applicable international copyright law, we maintain exclusive rights to all original works contained herein. Any allegations of plagiarism, code theft, or AI-generated content are unequivocally false and without merit. We maintain full legal title and copyright to all proprietary components of this codebase and will vigorously defend our intellectual property rights against any false claims.

This statement is made in good faith and with full knowledge of its legal consequences under applicable state and federal laws.

## License

BloodHorn is licensed under the BSD-2-Clause-Patent License. See `LICENSE` for details.

---
