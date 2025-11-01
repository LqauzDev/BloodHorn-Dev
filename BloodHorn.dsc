[Defines]
  PLATFORM_NAME                  = BloodHorn
  PLATFORM_GUID                  = 12345678-1234-1234-1234-1234567890AB
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/BloodHorn
  SUPPORTED_ARCHITECTURES        = X64|AARCH64|RISCV64|LOONGARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = BloodHorn.fdf
  VPD_TOOL_GUID                  = 8C3D856A-9BE6-468E-850A-24D7EEDEED21

[LibraryClasses]
  # Basic UEFI Libraries
  UefiLib                 = MdePkg/Library/UefiLib/UefiLib.inf
  UefiApplicationEntryPoint = MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib = MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib = MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  
  # Core Libraries
  BaseLib                 = MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib           = MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  DebugLib                = MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  DebugPrintErrorLevelLib  = MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  
  # Memory Management
  MemoryAllocationLib     = MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DxeServicesTableLib     = MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  
  # PCD and Configuration
  PcdLib                  = MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  
  # String and Print
  PrintLib                = MdePkg/Library/BasePrintLib/BasePrintLib.inf
  
  # Platform Specific
  DebugAgentLib           = MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  
  # For Coreboot integration
  CorebootLib             = BloodHorn/Coreboot/Library/CorebootLib/CorebootLib.inf
  
  # Security Libraries
  BaseCryptLib            = CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
  Tpm2DeviceLib           = SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2DeviceLibDTpm.inf

[Packages]
  # Core EDK2 Packages
  MdePkg/MdePkg.dec
  MdeModulePkg/MdeModulePkg.dec
  
  # Security Packages
  CryptoPkg/CryptoPkg.dec
  SecurityPkg/SecurityPkg.dec
  
  # Platform Specific
  BloodHorn/BloodHorn.dec

[Components]
  # Main Application
  BloodHorn/BloodHorn.inf
  
  # Coreboot Support
  BloodHorn/Coreboot/Library/CorebootLib/CorebootLib.inf
  
  # Required UEFI Drivers
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf
  MdeModulePkg/Core/Dxe/DxeMain.inf
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
  
  # Console Support
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  
  # File System Support
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  
  # FAT File System
  FatPkg/EnhancedFatDxe/Fat.inf
  
  # BDS (Boot Device Selection)
  MdeModulePkg/Universal/BdsDxe/BdsDxe.inf

[PcdsFixedAtBuild]
  # Coreboot platform configuration
  gBloodHornTokenSpaceGuid.PcdCorebootEnabled|TRUE
  gBloodHornTokenSpaceGuid.PcdCorebootTableAddress|0x500
  gBloodHornTokenSpaceGuid.PcdCorebootFramebufferEnabled|TRUE

  # Bootloader configuration
  gBloodHornTokenSpaceGuid.PcdDefaultBootEntry|"linux"
  gBloodHornTokenSpaceGuid.PcdMenuTimeout|10
  gBloodHornTokenSpaceGuid.PcdTpmEnabled|TRUE
  gBloodHornTokenSpaceGuid.PcdSecureBootEnabled|FALSE
  gBloodHornTokenSpaceGuid.PcdGuiEnabled|TRUE

[BuildOptions]
  # Global build options
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES
  
  # Coreboot-specific build options
  GCC:*_*_*_CC_FLAGS = -DCOREBOOT_PLATFORM -I$(WORKSPACE)/BloodHorn/coreboot
  
  # Debug options
  *_DEBUG_*_CC_FLAGS = -DDEBUG_ENABLE=1
  *_RELEASE_*_CC_FLAGS = -DNDEBUG
  
  # Security flags
  *_*_*_CC_FLAGS = -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks
  
  # Warning flags
  GCC:*_*_*_CC_FLAGS = -Wall -Werror -Wno-array-bounds -Wno-unused-parameter

[UserExtensions.TianoCore."ExtraFiles"]
  # Coreboot platform files
  BloodHorn/coreboot/coreboot_platform.h
  BloodHorn/coreboot/coreboot_payload.h
