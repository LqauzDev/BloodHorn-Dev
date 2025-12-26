# Contributing to BloodHorn

Thanks for being interested in hacking on BloodHorn. This file explains how to
work with the codebase, what kind of changes are welcome, and how to avoid
breaking things in subtle ways.

This is not a legal document or a wall of process – it’s a practical guide so
you can get useful patches in without fighting the project.

---

## 1. What kind of contributions make sense?

BloodHorn is a **serious bootloader / payload**, not a toy project. Useful
contributions include:

- Core boot logic fixes and cleanups (UEFI, Coreboot, early platform bring‑up).
- Better Linux / Windows / network boot paths and protocols.
- Security and robustness improvements (TPM, Secure Boot, verification).
- UI and UX improvements for the boot menu that don’t wreck simplicity.
- Rust integration work that keeps the Rust surface tight and focused.
- Documentation and examples that make it easier to actually use this thing.

Changes that are generally *not* interesting:

- Cosmetic refactors that don’t fix bugs or make the code easier to reason
  about.
- Huge feature dumps with no docs or clear use‑case.
- Turning BloodHorn into a kitchen‑sink "do everything" project.

If you’re unsure, open an issue or draft PR and describe what you want to do.

---

## 2. Getting the code and building

Prereqs (high‑level):

- A reasonably recent C toolchain (GCC/Clang) suitable for building EDK2.
- Python + `pip` if you use EDK2’s BaseTools as shipped.
- `git`.
- Optional but recommended: `rustup` and modern Rust if you are touching the
  Rust integration.

Typical steps (simplified):

1. Clone the repo:
   ```bash
   git clone https://example.com/BloodHorn.git
   cd BloodHorn
   ```
2. Follow the instructions in `README.md` and `USAGE.md` to set up the EDK2
   environment and build targets for your platform.
3. For Rust‑enabled builds, install the required Rust targets (see
   `docs/Rust-Integration.md`).

If the build instructions are missing something or unclear, that’s a good
candidate for your first documentation PR.

---

## 3. Coding style and guidelines

### C / C++

BloodHorn is mostly C.

- Stick to existing formatting and naming conventions in the file you’re
  editing.
- Prefer explicit, small helpers over clever macros.
- Don’t introduce new dependencies without a strong reason.
- Avoid "smart" but fragile tricks, especially around boot protocols and
  firmware interfaces.

When dealing with low‑level code (UEFI calls, Coreboot tables, memory maps):

- Be conservative with assumptions about firmware behaviour.
- Check return values; don’t silently ignore errors on critical paths.
- If you must rely on a subtle invariant, document it next to the code.

### Rust

Rust is used in **no_std, staticlib** crates under `rust/`. It is there to make
certain pieces **safer and easier to reason about**, not to rewrite the whole
bootloader.

- Keep crates focused and small: `bhshim`, `bhcore`, `bhlog`, `bhcfg`, `bhnet`,
  `bhutil` are examples.
- Use `#![no_std]` and the existing panic strategy (panic = abort).
- Prefer explicit FFI surfaces over magic globals.
- When exposing Rust to C:
  - Define a C header in `rust/` that clearly specifies the ABI.
  - Use `extern "C"` and simple, FFI‑friendly types.
  - Document any lifetime or ownership assumptions.

Avoid adding Rust just for aesthetics. If C already does the job reliably,
leave it alone unless you have a strong correctness or maintainability reason.

---

## 4. Rust integration specifics

BloodHorn currently uses Rust primarily for a **safe UEFI memory‑map shim** and
some internal helper crates.

- The shim lives in `rust/bhshim` and is wired into the EDK2 build via
  `bhshim_glue.c`, `bhshim_bootstrap.c`, and `.dsc/.inf` changes.
- Additional crates (`bhcore`, `bhlog`, `bhcfg`, `bhnet`, `bhutil`) live under
  `rust/` and are compiled as `no_std` staticlibs.
- Only crates that are explicitly linked and exposed through C/EDK2 actually
  affect the final firmware image.

If you add or change Rust code:

- Update `docs/Rust-Integration.md` if the behaviour or layout changes.
- Make sure the bootstrapper builds your crate for all supported architectures
  if it’s meant to be generic.
- Keep the public surface area small and well‑documented in the C header.

---

## 5. Linux and OS boot support

BloodHorn is intended to boot real OSes, not just test binaries.

- Linux boot support is configured via INI/JSON and environment variables
  (`bloodhorn.ini`, `bloodhorn.json`, `BLOODHORN_LINUX_*`). See
  `docs/Configuration.md`.
- When improving Linux (or other OS) support:
  - Preserve existing config keys and semantics.
  - Don’t introduce breaking changes without a clear migration path.
  - Always update docs and examples when you add new options.

If you are adding autodetection or distro‑specific helpers, keep them
optional and clearly separated from the core loader logic.

---

## 6. Security and trust

BloodHorn cares about security features (TPM, Secure Boot, integrity checks).
When you touch anything in this space:

- Be explicit about the threat model you are addressing.
- Do not weaken verification or measurement paths for convenience.
- Document any new assumptions (e.g., about keys, firmware, TPM presence).

If you’re adding a feature that could impact security (network boot flows,
recovery shell, etc.), include a short rationale in the PR description and
consider adding a note in `SECURITY.md` if it’s significant.

---

## 7. Documentation

Good docs are part of the project, not an afterthought.

- Update or create relevant files under `docs/` when you change behaviour.
- Keep examples in `USAGE.md`, `CONFIG.md`, and `docs/Configuration.md`
  accurate.
- For bigger features (new protocols, major Rust additions), add a dedicated
  markdown file in `docs/` and link it from `README.md` or the relevant
  overview doc.

If you find outdated or confusing documentation, feel free to fix it even if
you’re not touching the associated code.

---

## 8. Submitting changes

The rough flow:

1. Fork the repo and create a feature branch.
2. Make small, focused commits with clear messages.
3. Run the build(s) that are relevant to your change (e.g., at least one
   architecture, or all, if you’re modifying shared code).
4. Update or add tests if applicable (unit‑style tests, validation helpers,
   or at least a reproducible description).
5. Open a pull request with:
   - A short summary of what you changed.
   - Why you changed it (bugfix, feature, cleanup).
   - Any caveats or potential impact.

Review style is pragmatic: the main concern is "does this break existing
setups or make the code harder to reason about?".

---

## 9. Code of conduct

Use common sense:

- Be respectful in issues and PRs.
- Assume good faith, but don’t be afraid to push back on changes that
  complicate the project for little gain.

If there’s ever a formal code of conduct, it will be documented here, but the
baseline is simple: don’t be a jerk.

---

## 10. Getting help

If you’re stuck or unsure about an internal detail:

- Check `README.md`, `USAGE.md`, `CONFIG.md`, and `docs/` first.
- Look at existing implementations (for example, how UEFI / Coreboot / Linux
  boot paths are handled).
- Open an issue with a concrete question.

Contributions that respect the constraints of bootloader work (tight
environments, security implications, weird firmware) are welcome.
