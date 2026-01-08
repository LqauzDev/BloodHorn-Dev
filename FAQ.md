# Frequently Asked Questions

### What are the system requirements for BloodHorn?

**Minimum Requirements:**
- **CPU**: x86_64, AArch64 (ARM64), or RISC-V 64-bit
- **RAM**: 128MB minimum, 1GB recommended
- **Storage**: 32MB free space for installation
- **Firmware**: UEFI 2.8+ or BIOS with CSM
- **Build System**:
  - GCC or Clang toolchain
  - Python 3.7+
  - EDK2 development environment
  - NASM assembler

### Why is EDK2 required for building BloodHorn?

BloodHorn is built on top of the EDK2 framework because it provides:

1. **Hardware Abstraction**: Consistent interface across different system architectures
2. **Security Features**: Built-in support for Secure Boot, TPM, and cryptographic operations
3. **Reliability**: Industry-standard framework used in production environments
4. **Maintainability**: Well-documented codebase with active development

### What about security features?

BloodHorn implements several security measures:

1. **Secure Boot**: Verifies cryptographic signatures of all components
2. **TPM 2.0**: For hardware-based security operations and attestation
3. **Kernel Verification**: Optional hash checking of loaded kernels
4. **Memory Protection**: W^X (Write XOR Execute) memory permissions
5. **ASLR**: Address Space Layout Randomization for runtime protection

### How does BloodHorn handle disk encryption?

BloodHorn supports booting from encrypted volumes through:

1. **LUKS Integration**: Can unlock LUKS-encrypted root partitions
2. **TPM 2.0**: Supports sealing encryption keys to TPM
3. **Secure Boot**: Ensures the bootloader itself hasn't been tampered with
4. **Kernel Command-line**: Can pass encryption parameters to the Linux kernel

For full-disk encryption, we recommend using LUKS2 with TPM 2.0 integration for the best balance of security and usability.

### What if a malicious actor modifies the config file?

BloodHorn provides several protection mechanisms:

1. Secure Boot integration - The bootloader itself is signed and verified
2. SHA-512 config verification - Configuration integrity can be enforced
3. TPM 2.0 attestation - Hardware-level verification of boot state
4. Multiple config sources - INI, JSON, and UEFI variables with priority hierarchy
5. Recovery shell - Detects and allows recovery from configuration corruption

For enterprise deployments, we recommend using UEFI variable configuration combined with Secure Boot for maximum protection.

### What architectures are supported?

Currently supported architectures:

1. **x86_64**: Standard 64-bit x86 systems (most modern PCs)
2. **AArch64 (ARM64)**: ARM-based servers and single-board computers
3. **RISC-V 64**: Emerging RISC-V platforms

Planned support:
- LoongArch 64-bit
- x86 (32-bit) legacy systems

### How do I report security vulnerabilities?

Please report security issues responsibly to:
- Email: security@bloodyhell.industries
- PGP Key: [Available on our website]

We appreciate responsible disclosure and will acknowledge all valid reports.

### Why support so many architectures?

Modern computing environments are diverse. BloodHorn supports:
- x86_64 - Desktop and server standard
- ARM64 - Mobile, embedded, and modern servers
- RISC-V - Open ISA for future-proofing
- LoongArch - Chinese processor ecosystem
- IA-32/x86 - Legacy system support

This broad support ensures BloodHorn can be deployed in any environment, from embedded systems to enterprise servers, while maintaining a consistent security posture and feature set.

### Why was the plugin system removed?

BloodHorn previously included a plugin system, but it was removed because it added unnecessary complexity without providing significant benefits. The bootloader's core functionality is comprehensive enough that plugins are not needed for most use cases.

This decision aligns with our philosophy of maintaining a secure, focused bootloader that does what's needed well, rather than trying to be extensible for every possible edge case.

### Why was Lua scripting removed?

BloodHorn previously included Lua scripting support, but it was removed to maintain focus on the bootloader's core mission. Scripting added complexity and potential security surfaces without providing essential functionality for most users.

The recovery shell provides sufficient automation capabilities for troubleshooting and maintenance, while keeping the attack surface minimal and the codebase focused on security and performance.

### I do not want to have a separate FAT boot partition! What can I do?

BloodHorn supports multiple deployment scenarios:

1. EFI System Partition sharing - Share the ESP with your OS's /boot directory
2. Network boot - PXE/HTTP boot eliminates local storage requirements
3. Coreboot integration - Direct firmware integration for embedded systems
4. Multiple filesystem support - While we recommend FAT32 for compatibility, we support reading from various filesystems

For UEFI systems, sharing the EFI System Partition is the recommended approach and works seamlessly with modern operating systems.

### Why doesn't BloodHorn have a decompressor?

BloodHorn doesn't need a decompressor because it uses the EDK2 framework, which provides efficient loading mechanisms. Unlike standalone bootloaders that need compression to fit into limited BIOS stages, EDK2 handles memory management and loading efficiently without requiring additional compression layers.

This approach provides several advantages:
- Simpler codebase with fewer potential failure points
- Faster boot times without decompression overhead
- Better security with less code to audit
- More reliable operation without compression-related issues

### Why not support filesystem X or feature Y? (Common questions)

BloodHorn focuses on features that provide tangible security and usability benefits:

1. Security-critical features - TPM, crypto, secure boot, verification
2. Essential filesystems - FAT32 for broad compatibility, sufficient for boot needs
3. User experience - Themes, mouse, localization, recovery tools
4. Enterprise features - EDK2 integration, multiple config formats, broad architecture support

We avoid feature creep that doesn't serve our core mission of providing a secure, user-friendly bootloader. The kernel should handle complex filesystem and storage management.

### How does BloodHorn compare to other bootloaders?

vs GRUB2: More secure, modern architecture, better performance
vs Limine: More features, better security, broader architecture support  
vs systemd-boot: More security features, better customization, cross-platform
vs rEFInd: More enterprise features, better security, recovery shell

we don't compare as in the end each bootloader has its own unique features and strengths. it's just bloodhorns stack compared to other bootloaders.
BloodHorn occupies a unique position as a security-focused, feature-rich bootloader that maintains compactness through efficient design rather than feature removal.
