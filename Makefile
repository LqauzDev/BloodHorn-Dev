# BloodHorn Build System
# Automated EDK2 build system for BloodHorn bootloader

.PHONY: all clean distclean edk2-setup edk2-build x64 ia32 aarch64 riscv64 loongarch64 help install

# Default target
all: x64

# Project / package layout
PROJECT_ROOT := $(abspath .)
PROJECT_NAME := $(notdir $(PROJECT_ROOT))
PACKAGE_PATH := $(PROJECT_ROOT)/BloodHorn.dsc
MODULE_PATH := $(PROJECT_ROOT)/BloodHorn.inf

# EDK2 configuration
EDK2_URL ?= https://github.com/tianocore/edk2.git
EDK2_BRANCH ?= master
EDK2_DIR ?= edk2
TOOLCHAIN ?= GCC5
BUILD_TARGET ?= DEBUG
TARGET ?= X64

# Architecture targets
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

# EDK2 setup
edk2-setup:
	@if [ ! -d "$(EDK2_DIR)" ]; then \
		echo "Cloning EDK2 from $(EDK2_URL)..."; \
		if ! git clone -b "$(EDK2_BRANCH)" "$(EDK2_URL)" "$(EDK2_DIR)"; then \
			echo "ERROR: failed to clone EDK2."; \
			echo "Hint: if network access is restricted, provide an existing checkout with EDK2_DIR=<path>."; \
			exit 1; \
		fi; \
	else \
		echo "EDK2 already exists at $(EDK2_DIR), skipping clone/update."; \
	fi
	@echo "Setting up EDK2 submodules..."
	cd "$(EDK2_DIR)" && git submodule update --init --recursive

# Build BloodHorn with EDK2
edk2-build: edk2-setup
	@if [ ! -f "$(PACKAGE_PATH)" ]; then \
		echo "ERROR: missing platform DSC at $(PACKAGE_PATH)"; \
		exit 1; \
	fi
	@if [ ! -f "$(MODULE_PATH)" ]; then \
		echo "ERROR: missing module INF at $(MODULE_PATH)"; \
		exit 1; \
	fi
	@echo "Building BloodHorn for $(TARGET) (target=$(BUILD_TARGET), toolchain=$(TOOLCHAIN))..."
	cd "$(EDK2_DIR)" && \
		export WORKSPACE=$$PWD && \
		export PACKAGES_PATH="$(PROJECT_ROOT)" && \
		export EDK_TOOLS_PATH=$$PWD/BaseTools && \
		. edksetup.sh && \
		build -a "$(TARGET)" \
		      -p "$(PACKAGE_PATH)" \
		      -m "$(MODULE_PATH)" \
		      -t "$(TOOLCHAIN)" \
		      -b "$(BUILD_TARGET)"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf Build
	rm -f BloodHorn.efi
	rm -f BloodHorn_*.efi

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

# Help
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
	@echo "  edk2-build        - Build with EDK2 (internal target)"
	@echo "  edk2-setup        - Setup EDK2 environment"
	@echo "  clean             - Clean build artifacts"
	@echo "  distclean         - Clean everything including EDK2"
	@echo "  install           - Copy built BloodHorn.efi into repository root"
	@echo "  help              - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  EDK2_URL          - EDK2 repository URL (default: https://github.com/tianocore/edk2.git)"
	@echo "  EDK2_BRANCH       - EDK2 branch to use (default: master)"
	@echo "  EDK2_DIR          - EDK2 directory name or path (default: edk2)"
	@echo "  TOOLCHAIN         - EDK2 toolchain tag (default: GCC5)"
	@echo "  BUILD_TARGET      - EDK2 build target (default: DEBUG)"
	@echo ""
	@echo "Examples:"
	@echo "  make x64"
	@echo "  make aarch64 TOOLCHAIN=CLANG38 BUILD_TARGET=RELEASE"
	@echo "  make edk2-build TARGET=RISCV64"
