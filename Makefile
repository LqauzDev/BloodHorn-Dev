# BloodHorn Build System
# Automated EDK2 build system for BloodHorn bootloader

.PHONY: all clean edk2-build x64 ia32 aarch64 riscv64 loongarch64 help install

# Default target
all: x64

# EDK2 configuration
EDK2_URL ?= https://github.com/tianocore/edk2.git
EDK2_BRANCH ?= master
EDK2_DIR ?= edk2
BUILD_DIR ?= Build
TOOLCHAIN ?= GCC5

# Architecture targets
x64: edk2-build TARGET=X64
ia32: edk2-build TARGET=IA32
aarch64: edk2-build TARGET=AARCH64
riscv64: edk2-build TARGET=RISCV64
loongarch64: edk2-build TARGET=LOONGARCH64

# EDK2 setup
edk2-setup:
	@if [ ! -d "$(EDK2_DIR)" ]; then \
		echo "Cloning EDK2 from $(EDK2_URL)..."; \
		git clone -b $(EDK2_BRANCH) $(EDK2_URL) $(EDK2_DIR); \
	else \
		echo "EDK2 already exists, updating..."; \
		cd $(EDK2_DIR) && git pull origin $(EDK2_BRANCH); \
	fi
	@echo "Setting up EDK2 build environment..."
	cd $(EDK2_DIR) && git submodule update --init --recursive

# Build BloodHorn with EDK2
edk2-build: edk2-setup
	@echo "Building BloodHorn for $(TARGET)..."
	cd $(EDK2_DIR) && \
		export WORKSPACE=$$PWD && \
		export PACKAGES_PATH=$$PWD/../ && \
		export EDK_TOOLS_PATH=$$PWD/BaseTools && \
		. edksetup.sh && \
		build -a $(TARGET) -p BloodHorn/BloodHorn.inf -t $(TOOLCHAIN) -b $(BUILD_DIR)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	rm -f BloodHorn.efi
	rm -f BloodHorn_*.efi

# Clean everything including EDK2
distclean: clean
	@echo "Cleaning everything including EDK2..."
	rm -rf $(EDK2_DIR)

# Install BloodHorn
install: x64
	@echo "Installing BloodHorn..."
	@if [ -d "$(BUILD_DIR)/BloodHorn/$(TOOLCHAIN)/X64" ]; then \
		cp $(BUILD_DIR)/BloodHorn/$(TOOLCHAIN)/X64/BloodHorn.efi ./BloodHorn.efi; \
		echo "BloodHorn.efi installed successfully"; \
	else \
		echo "Build not found. Run 'make x64' first."; \
		exit 1; \
	fi

# Help
help:
	@echo "BloodHorn Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build for X64 (default)"
	@echo "  x64              - Build for X64 architecture"
	@echo "  ia32             - Build for IA32 architecture"
	@echo "  aarch64           - Build for ARM64 architecture"
	@echo "  riscv64           - Build for RISC-V 64-bit architecture"
	@echo "  loongarch64       - Build for LoongArch 64-bit architecture"
	@echo "  edk2-build       - Build with EDK2 (internal target)"
	@echo "  edk2-setup       - Setup EDK2 environment"
	@echo "  clean            - Clean build artifacts"
	@echo "  distclean         - Clean everything including EDK2"
	@echo "  install          - Install BloodHorn.efi"
	@echo "  help             - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  EDK2_URL         - EDK2 repository URL (default: https://github.com/tianocore/edk2.git)"
	@echo "  EDK2_BRANCH      - EDK2 branch to use (default: master)"
	@echo "  EDK2_DIR         - EDK2 directory name (default: edk2)"
	@echo "  BUILD_DIR        - Build directory (default: Build)"
	@echo "  TOOLCHAIN        - EDK2 toolchain (default: GCC5)"
	@echo ""
	@echo "Examples:"
	@echo "  make x64                    # Build for X64"
	@echo "  make edk2-build TARGET=AARCH64 # Build for ARM64"
	@echo "  make clean                   # Clean build"
	@echo "  make install                 # Install BloodHorn.efi"
