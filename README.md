# BloodHorn

<p align="center">
  <img src="./Z.png" alt="BloodHorn Logo" width="420"/>
  <br>
  <em>Fast, secure, and reliable system bootloader</em>
</p>

[![Quality](quality_badge.svg)](./) [![Build Status](https://ci.codeberg.org/api/badges/PacHashs/BloodHorn/status.svg)](https://ci.codeberg.org/PacHashs/BloodHorn) [![Platform](https://img.shields.io/badge/platform-x86__64%20%7C%20ARM64%20%7C%20RISC--V%20%7C%20LoongArch-blue)](https://codeberg.org/PacHashs/BloodHorn) [![License](https://img.shields.io/badge/License-BSD--2--Clause--Patent-blue.svg)](LICENSE)
## About
BloodHorn is a compact, extensible bootloader built with EDK2 and optional Coreboot payload support. It targets UEFI and legacy workflows on multiple architectures and emphasizes secure module verification and maintainability.

## Safety & Audience
BloodHorn operates at firmware level and can affect device boot and data. Read `SECURITY.md` before use and back up any existing bootloaders.

Intended audience: firmware engineers, OS developers, and security researchers.

## CI
We run CI builds and tests on the public pipeline. See `.woodpecker.yml` for details and the CI dashboard for current status.

## Support
If you find BloodHorn useful, see `CONTRIBUTING.md` or support via the project's sponsorship links.

## Getting started

Prerequisites: Git, an EDK2 toolchain, and a C toolchain (GCC/Clang or MSVC).

## Installation

Install by copying the built `BloodHorn.efi` to your EFI partition; back up your current bootloader first.

## Layout
Top-level folders include `coreboot/`, `uefi/`, `security/`, `fs/`, `net/`, and `plugins/`.

## Features

- Multi-architecture support (x86_64, ARM64, RISC-V, LoongArch)
- Secure Boot and TPM 2.0 support
- Coreboot payload support and UEFI integration

## History

BloodHorn started as an early bootloader prototype in 2016 and has since evolved into a cross-architecture payload with a focus on security and maintainability.

![First BloodHorn Screenshot (2016)](./2016screenshot.png)

## Configuration
Create a minimal `bloodhorn.ini` next to `BloodHorn.efi` on the EFI partition:
## FAQ

- What architectures are supported? - x86_64, ARM64, RISC-V, LoongArch.
- Is Secure Boot supported? - Yes. BloodHorn includes Secure Boot support and optional TPM verification.
- How to contribute? - See `CONTRIBUTING.md` and open issues or PRs on the project repo.

## Contributors

See `maintainers.py` and the project contributors list.

---

![BSD License](bsd.svg)

BloodHorn is licensed under the BSD-2-Clause-Patent License. See `LICENSE` for details.
