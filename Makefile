# BloodHorn Build System
# Automated EDK2 build system for BloodHorn bootloader

.PHONY: all clean distclean preflight edk2-setup edk2-build x64 ia32 aarch64 riscv64 loongarch64 help install

# Default target
all: x64

PROJECT_ROOT := $(abspath .)
PACKAGE_DSC  := BloodHorn.dsc
MODULE_INF   := BloodHorn.inf

# EDK2 configuration
EDK2_URL ?= https://github.com/tianocore/edk2.git
EDK2_BRANCH ?= master
EDK2_DIR ?= edk2
TOOLCHAIN ?= GCC5
BUILD_TARGET ?= DEBUG
TARGET ?= X64

x64: TARGET=X64
x64: edk2-build

ia32: TARGET=IA32
ia32: edk2-build

aarch64: TARGET=AARCH64
aarch64: edk2-build

riscv64: TARGET=RISCV64
riscv64: edk2-build

loongarch64: TARGET=LOONGARCH64
loongarch64: edk2-build

preflight:
	@./scripts/verify_edk2_layout.sh

# EDK2 setup
edk2-setup:
	@if [ ! -d "$(EDK2_DIR)" ]; then \
		echo "Cloning EDK2 from $(EDK2_URL)..."; \
		if ! git clone -b "$(EDK2_BRANCH)" "$(EDK2_URL)" "$(EDK2_DIR)"; then \
			echo "ERROR: failed to clone EDK2."; \
			echo "Hint: pass EDK2_DIR=<existing local checkout> when running without internet access."; \
			exit 1; \
		fi; \
	else \
		echo "EDK2 already exists at $(EDK2_DIR), skipping clone/update."; \
	fi
	@echo "Setting up EDK2 submodules..."
	cd "$(EDK2_DIR)" && git submodule update --init --recursive

# Build BloodHorn with EDK2
edk2-build: preflight edk2-setup
	@if [ ! -f "$(PROJECT_ROOT)/$(PACKAGE_DSC)" ]; then \
		echo "ERROR: missing platform DSC at $(PROJECT_ROOT)/$(PACKAGE_DSC)"; \
		exit 1; \
	fi
	@if [ ! -f "$(PROJECT_ROOT)/$(MODULE_INF)" ]; then \
		echo "ERROR: missing module INF at $(PROJECT_ROOT)/$(MODULE_INF)"; \
		exit 1; \
	fi
	@echo "Building BloodHorn for $(TARGET) (target=$(BUILD_TARGET), toolchain=$(TOOLCHAIN))..."
	cd "$(EDK2_DIR)" && \
		export WORKSPACE=$$PWD && \
		export PACKAGES_PATH="$$PWD:$(PROJECT_ROOT)" && \
		export EDK_TOOLS_PATH=$$PWD/BaseTools && \
		. edksetup.sh && \
		build -a "$(TARGET)" -p "$(PACKAGE_DSC)" -m "$(MODULE_INF)" -t "$(TOOLCHAIN)" -b "$(BUILD_TARGET)"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf Build
	rm -f BloodHorn.efi BloodHorn_*.efi

# Clean everything including EDK2
distclean: clean
	@echo "Cleaning everything including EDK2..."
	rm -rf "$(EDK2_DIR)"

# Install BloodHorn
install: x64
	@echo "Installing BloodHorn..."
	@build_output="$(EDK2_DIR)/Build/BloodHorn/$(BUILD_TARGET)_$(TOOLCHAIN)/X64/BloodHorn.efi"; \
	if [ -f "$$build_output" ]; then \
		cp "$$build_output" ./BloodHorn.efi; \
		echo "BloodHorn.efi installed successfully"; \
	else \
		echo "Build output not found at: $$build_output"; \
		echo "Run 'make x64' first and check build errors."; \
		exit 1; \
	fi

help:
	@echo "BloodHorn Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all               - Build for X64 (default)"
	@echo "  x64               - Build for X64 architecture"
	@echo "  ia32              - Build for IA32 architecture"
	@echo "  aarch64           - Build for ARM64 architecture"
	@echo "  riscv64           - Build for RISC-V 64-bit architecture"
	@echo "  loongarch64       - Build for LoongArch 64-bit architecture"
	@echo "  preflight         - Validate project layout before EDK2 build"
	@echo "  edk2-build        - Build with EDK2 (internal target)"
	@echo "  edk2-setup        - Setup EDK2 environment"
	@echo "  clean             - Clean build artifacts"
	@echo "  distclean         - Clean everything including EDK2"
	@echo "  install           - Copy built BloodHorn.efi into repository root"
	@echo "  help              - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  EDK2_URL          - EDK2 repository URL"
	@echo "  EDK2_BRANCH       - EDK2 branch to use"
	@echo "  EDK2_DIR          - EDK2 directory name or path"
	@echo "  TOOLCHAIN         - EDK2 toolchain tag (default: GCC5)"
	@echo "  BUILD_TARGET      - EDK2 build target (default: DEBUG)"
