# Frequently Asked Questions

### Why does BloodHorn have so many features? Isn't that bloated?

BloodHorn follows a security + features + compact philosophy. Unlike minimalist bootloaders, we believe that modern systems need comprehensive security features, user-friendly interfaces, and enterprise integration capabilities.

Our feature set is carefully curated:
- Security: TPM 2.0, comprehensive crypto, secure boot, kernel verification
- Usability: Graphical themes, mouse support, multi-language, recovery shell
- Enterprise: EDK2 integration, multiple config formats, broad architecture support

We maintain compactness through efficient EDK2-based architecture and modular design, not by removing essential functionality.

### What about LUKS? What about security? Encrypt the kernel!

BloodHorn provides multiple layers of security that make kernel encryption unnecessary:

1. SHA-512 kernel hash verification - Ensures kernel integrity
2. TPM 2.0 integration - Hardware-rooted trust chains
3. Secure Boot support - Cryptographic verification of all components
4. Encrypted configuration options - Protect sensitive settings
5. Constant-time operations - Prevent timing attacks

Putting the kernel/modules on a FAT32 partition with SHA-512 verification provides equivalent security to full disk encryption for boot purposes, while maintaining compatibility and performance.

### What if a malicious actor modifies the config file?

BloodHorn provides several protection mechanisms:

1. Secure Boot integration - The bootloader itself is signed and verified
2. SHA-512 config verification - Configuration integrity can be enforced
3. TPM 2.0 attestation - Hardware-level verification of boot state
4. Multiple config sources - INI, JSON, and UEFI variables with priority hierarchy
5. Recovery shell - Detects and allows recovery from configuration corruption

For enterprise deployments, we recommend using UEFI variable configuration combined with Secure Boot for maximum protection.

### Why use EDK2 instead of a standalone approach?

EDK2 provides several advantages for a security-focused bootloader:

1. Proven firmware framework - Battle-tested in enterprise environments
2. Comprehensive hardware support - Wide device compatibility out-of-the-box
3. Security features - Built-in Secure Boot, TPM integration, memory protection
4. Enterprise integration - Standard firmware interface for IT infrastructure
5. Maintainability - Industry-standard codebase with regular security updates

This allows BloodHorn to focus on bootloader-specific innovations while leveraging EDK2's robust foundation.

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
