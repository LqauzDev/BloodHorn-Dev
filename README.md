# BloodHorn

<p align="center">
  <img src="Z.png" alt="BloodHorn Logo" width="500"/>
  <br>
  <em>"The bootloader that boots so fast, it's already judging your life choices"</em>
</p>

> **Temporary Hiatus Notice**  
> Currently on a digital sabbatical until 2026.  
> Think of it as a really long coffee break.  
> The code isn't going anywhere - it's just taking a nap.  
> PRs are in read-only mode (like your grandma's Facebook account).

## BloodHorn: The Bootloader That Doesn't Ask Stupid Questions

BloodHorn is the Chuck Norris of bootloaders - it doesn't boot your system, it allows your system to boot. While other bootloaders are still asking if you want to start in safe mode, BloodHorn has already booted your OS, made coffee, and is now judging your choice of Linux distribution.

## Support Me

If you like this project and want to support me, you can donate through [Liberapay](https://liberapay.com/Listedroot/donate).

[![Build Status](https://ci.codeberg.org/api/badges/PacHashs/BloodHorn/status.svg)](https://ci.codeberg.org/PacHashs/BloodHorn)

[![Donate using Liberapay](https://liberapay.com/assets/widgets/donate.svg)](https://liberapay.com/Listedroot/donate)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-x86__64%20%7C%20ARM64%20%7C%20RISC--V%20%7C%20LoongArch-blue)](https://codeberg.org/PacHashs/BloodHorn)

## Quick Start

1. Build with EDK2: `build -p BloodHorn/BloodHorn.dsc`
2. Copy `BloodHorn.efi` to your EFI system partition.
3. Add a minimal config at the partition root as `bloodhorn.ini`:

```ini
[boot]
default = linux
menu_timeout = 5
language = en
font_path = /fonts/ter-16n.psf

[linux]
kernel = /boot/vmlinuz
initrd = /boot/initrd.img
cmdline = root=/dev/sda1 ro quiet
```

4. Optional assets:
   - Localization file: `\\locales\\en.ini` (key=value strings)
   - PSF1 font: `/fonts/ter-16n.psf` (referenced by `boot.font_path`)
5. Boot BloodHorn from your firmware menu. Press any key to open the menu during the countdown.
## Features That Make GRUB Look Like a Dial-Up Connection

- **Architectures**: x86_64, ARM64, RISC-V, LoongArch - Like a UN meeting, but for your computer's soul
- **Protocols**: Linux, Multiboot 1/2, Limine, Chainload, PXE, BloodChain - More protocols than your ex has excuses
- **Security**: Secure Boot, TPM 2.0, file verification - We've got more layers than an onion (and might make you cry less)
- **Config**: INI/JSON/UEFI vars - Because one way to do things is boring, and three is a party
- **UI**: Text/graphical with localization - Now with 100% fewer "grub>" prompts that make you question your life choices
- **Firmware**: Coreboot + UEFI hybrid - Like peanut butter and chocolate, but for your firmware
- **Recovery Mode**: For when you thought "rm -rf /" was a good idea at 3 AM
- **Boot Speed**: Faster than you can say "I'll just check Reddit while I wait"

## Building
- Install EDK2 and required toolchain
- Build: `build -p BloodHorn/BloodHorn.dsc`
- Output: `Build/BloodHorn/`

## Usage
- Copy `BloodHorn.efi` to your EFI partition and select it in firmware.
- Put `bloodhorn.ini` or `bloodhorn.json` at the partition root (UEFI vars `BLOODHORN_*` override files).
- Optional: `\\locales\\<lang>.ini` and PSF1 font under `/fonts/` referenced by `boot.font_path`.

**Coreboot Integration:** BloodHorn automatically detects and adapts to Coreboot firmware when running as a payload, providing enhanced hardware control and performance while maintaining UEFI compatibility.

## Documentation
- `docs/Overview.md` – high-level overview
- `docs/Configuration.md` – all config options (themes, fonts, locales, entries)
- `docs/Boot-Protocols.md` – supported protocols
- `docs/COREBOOT_INTEGRATION.md` – hybrid mode details

**Coreboot Integration:** For detailed information about BloodHorn's Coreboot firmware integration, see `docs/COREBOOT_INTEGRATION.md`.

## Why BloodHorn? (Asking for a Friend)

### You'll Love BloodHorn If You've Ever:
- Wanted to high-five your bootloader (metaphorically, please don't touch the screen)
- Thought "I wish this boot process had more personality"
- Wasted hours configuring GRUB only to get a blinking cursor
- Wanted to feel like a hacker without actually having to hack anything

### Choose BloodHorn When:
- **You value your time** - Boots faster than you can say "I'll just check my phone"
- **You like options** - More configuration choices than a hipster coffee shop
- **You make mistakes** - Recovery mode that doesn't judge (much)
- **You're not a morning person** - Because neither are we

### Stick With GRUB If You:
- Enjoy typing commands that look like they're in Klingon
- Think booting should be a social activity (with all that waiting around)
- Believe "minimalist" means "I like staring at a command prompt"
- Have a poster of Linus Torvalds on your wall (we see you)

### Real Developers (and Their Computers) Love BloodHorn
- **For the rebels** who think "standards are just suggestions"
- **For the optimists** who think "it worked once, ship it!"
- **For the realists** who know they'll be debugging this at 2 AM
- **For the dreamers** who believe one day their code will be as cool as BloodHorn

> *"I used to have a life. Then I found BloodHorn. Now I spend all my time booting things just for fun."*  
> — Probably someone, somewhere, hopefully

## FAQ

### What is the size of BloodHorn?
BloodHorn source code is approximately 500 KB before compilation. After building, the bootloader binary ranges from 1.5 to 2 MB depending on enabled features and architecture.

### How do I contribute to BloodHorn?
Contributions are welcome! Please see our contribution guidelines in the docs/ directory and feel free to open issues or submit pull requests on our Codeberg repository.

### Does BloodHorn support Secure Boot?
Yes, BloodHorn includes comprehensive Secure Boot support with TPM 2.0 integration and cryptographic verification of all loaded modules.

### Does BloodHorn support Coreboot firmware?
Yes, BloodHorn features comprehensive Coreboot firmware integration with automatic detection and hybrid initialization. When running as a Coreboot payload, it leverages Coreboot's direct hardware control for enhanced performance while maintaining UEFI compatibility for broader system support.

## Contributors

- **[PacHash](https://github.com/PacHashs)** - Lead Developer
- **[BillNyeTheScienceGuy](https://codeberg.org/BillNyeTheSienceGuy)** - Main Team

---

*BloodHorn was inspired by modern bootloaders, but all code is original and written from scratch for educational purposes and for use in future operating systems.*

## License
MIT.
