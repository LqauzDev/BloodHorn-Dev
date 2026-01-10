# BloodHorn Troubleshooting and Debugging Guide

This comprehensive guide helps diagnose and resolve common issues with BloodHorn bootloader.

## Table of Contents

1. [Quick Diagnostic Checklist](#quick-diagnostic-checklist)
2. [Boot Failure Issues](#boot-failure-issues)
3. [Graphics and Display Problems](#graphics-and-display-problems)
4. [Security and Verification Issues](#security-and-verification-issues)
5. [Network Boot Problems](#network-boot-problems)
6. [Configuration Issues](#configuration-issues)
7. [Memory and Performance Issues](#memory-and-performance-issues)
8. [Architecture-Specific Issues](#architecture-specific-issues)
9. [Debugging Techniques](#debugging-techniques)
10. [Common Error Messages](#common-error-messages)
11. [Recovery Procedures](#recovery-procedures)

## Quick Diagnostic Checklist

Before diving into detailed troubleshooting, run through this quick checklist:

### Basic System Check
- [ ] Verify EDK2 environment is properly set up
- [ ] Check BloodHorn binary is correctly built
- [ ] Ensure target system supports UEFI boot
- [ ] Verify boot media is properly formatted (FAT32)
- [ ] Check system firmware settings (Secure Boot, CSM, etc.)

### File System Check
- [ ] Verify all required files are present on boot media
- [ ] Check file permissions and attributes
- [ ] Validate configuration file syntax
- [ ] Ensure kernel and initrd files are accessible

### Hardware Compatibility
- [ ] Verify graphics adapter compatibility
- [ ] Check storage controller support
- [ ] Validate network adapter for PXE boot
- [ ] Ensure TPM hardware is present (if using TPM features)

## Boot Failure Issues

### Issue: BloodHorn Doesn't Start

**Symptoms:**
- System hangs immediately after selecting BloodHorn
- No output on screen
- System reboots automatically

**Diagnostic Steps:**

1. **Check Build Integrity:**
   ```bash
   # Verify BloodHorn was built correctly
   ls Build/BloodHorn/DEBUG_GCC5/X64/BloodHorn.efi
   # Should show: BloodHorn.efi file exists
   ```

2. **Verify UEFI Compatibility:**
   ```bash
   # Check if system supports UEFI (Linux)
   efibootmgr -v
   # Or check BIOS/UEFI setup for UEFI mode
   
   # Verify EDK2 environment is set up
   echo $EDK_TOOLS_PATH
   # Should show path to EDK2 tools
   ```

3. **Test Minimal Configuration:**
   - Create minimal `bloodhorn.ini` with basic settings
   - Disable advanced features (TPM, secure boot, GUI)
   ```ini
   [boot]
   default_entry=linux
   menu_timeout=5
   use_gui=false
   secure_boot=false
   tpm_enabled=false
   ```

**Common Solutions:**
- Rebuild BloodHorn with correct EDK2 environment:
  ```bash
  source edksetup.sh
  build -a X64 -p BloodHorn/BloodHorn.dsc -t GCC5
  ```
- Ensure boot media is formatted as FAT32
- Disable Secure Boot in firmware temporarily
- Check for corrupted bootloader files
- Verify all submodules are initialized:
  ```bash
  git submodule update --init --recursive
  ```

### Issue: Kernel Loading Fails

**Symptoms:**
- "Failed to load kernel" message
- "File not found" errors
- Kernel verification failures

**Diagnostic Steps:**

1. **Verify Kernel Path:**
   ```ini
   # Check bloodhorn.ini
   [linux]
   kernel=/boot/vmlinuz-5.15.0
   initrd=/boot/initrd.img-5.15.0
   ```

2. **Check File Accessibility:**
   - Use recovery shell to list files
   - Verify file permissions
   - Check for file corruption

3. **Validate Kernel Format:**
   ```bash
   file vmlinuz-5.15.0
   # Should show: Linux kernel x86 boot executable (bzImage)
   ```

4. **Check Hardcoded Paths:**
   BloodHorn uses hardcoded paths in boot wrappers:
   ```c
   // From BootLinuxKernelWrapper
   return linux_load_kernel("/boot/vmlinuz", "/boot/initrd.img", "root=/dev/sda1 ro");
   ```

**Common Solutions:**
- Place kernel files in `/boot/` directory on boot media
- Ensure kernel is supported format (bzImage, ELF)
- Disable kernel verification if signatures are missing
- Check for sufficient memory for kernel loading
- Verify kernel files are not corrupted

### Issue: ExitBootServices Fails

**Symptoms:**
- "ExitBootServices failed" message
- System hangs during boot transition
- Memory map errors

**Diagnostic Steps:**

1. **Check Memory Map Consistency:**
   - BloodHorn does not implement retry logic for ExitBootServices
   - Monitor debug output for memory map changes

2. **Verify Memory Allocation:**
   - Ensure all allocated memory is freed
   - Check for memory leaks in custom code

**Common Solutions:**
- Free all allocated memory before ExitBootServices
- Check for conflicting memory allocations
- Reduce memory usage in bootloader
- Ensure proper cleanup in error paths

## Graphics and Display Problems

### Issue: No GUI Display

**Symptoms:**
- Blank screen after BloodHorn starts
- Only text output visible
- "Graphics initialization failed" message

**Diagnostic Steps:**

1. **Check Graphics Protocol:**
   ```c
   // In recovery shell, check available graphics modes
   graphics mode list
   ```

2. **Verify Font Loading:**
   ```ini
   # Check font configuration
   [boot]
   font_path=DejaVuSans.ttf
   font_size=12
   ```

3. **Test Framebuffer Access:**
   - Verify framebuffer address is valid
   - Check pixel format compatibility

**Common Solutions:**
- Disable GUI mode and use text interface
- Use different font file or size
- Check graphics driver compatibility
- Verify monitor supports selected resolution

### Issue: Mouse Not Working

**Symptoms:**
- Mouse cursor doesn't move
- Click events not detected
- "Mouse initialization failed" message

**Diagnostic Steps:**

1. **Check USB Support:**
   - Verify USB controllers are initialized
   - Check for USB mouse device recognition

2. **Test Mouse Protocol:**
   - Ensure HID protocol is available
   - Check for absolute vs. relative addressing

**Common Solutions:**
- Use keyboard navigation instead
- Check USB port and mouse functionality
- Verify HID driver support in firmware

## Security and Verification Issues

### Issue: TPM Initialization Fails

**Symptoms:**
- "TPM 2.0 not available" message
- "TPM initialization failed" error
- Secure boot verification failures

**Diagnostic Steps:**

1. **Check TPM Hardware:**
   ```bash
   # Linux: Check TPM device
   ls /dev/tpm*
   dmesg | grep -i tpm
   ```

2. **Verify TPM 2.0 Support:**
   - Check firmware TPM settings
   - Ensure TPM is enabled and activated

3. **Test TPM Protocol:**
   - Verify TCG2 protocol availability
   - Check TPM response times

**Common Solutions:**
- Enable TPM in firmware settings
- Disable TPM features if not needed
- Update firmware to latest version
- Check for TPM hardware compatibility

### Issue: Kernel Hash Verification Fails

**Symptoms:**
- "Kernel hash verification failed" message
- "Security check failed" error
- Boot blocked due to verification failure

**Diagnostic Steps:**

1. **Verify Hash Configuration:**
   ```c
   // Check expected hash in source
   FILE_HASH g_known_hashes[1] = {
       {"kernel.efi", {0x12, 0x34, ...}}  // Expected SHA-512
   };
   ```

2. **Calculate Actual Hash:**
   ```bash
   sha512sum kernel.efi
   # Compare with expected hash
   ```

3. **Check File Integrity:**
   - Verify kernel file is not corrupted
   - Check for file tampering

**Common Solutions:**
- Update expected hash with correct value
- Replace corrupted kernel file
- Temporarily disable verification for testing
- Use secure distribution channels for kernels

## Network Boot Problems

### Issue: PXE Boot Fails

**Symptoms:**
- "PXE initialization failed" message
- "Network adapter not found" error
- Timeout during network boot

**Diagnostic Steps:**

1. **Check Network Hardware:**
   - Verify network adapter is recognized
   - Check for driver support in firmware

2. **Test Network Connectivity:**
   ```bash
   # Check DHCP server availability
   dhcping -s <server_ip>
   
   # Test TFTP server
   tftp <server_ip> -c get testfile
   ```

3. **Verify PXE Configuration:**
   - Check DHCP option 43 (PXE options)
   - Verify TFTP server accessibility

**Common Solutions:**
- Check network cable and switch port
- Verify DHCP server configuration
- Ensure TFTP server is running
- Check firewall settings

### Issue: HTTP Boot Fails

**Symptoms:**
- "HTTP boot failed" message
- "Server not reachable" error
- SSL/TLS certificate issues

**Diagnostic Steps:**

1. **Test HTTP Server:**
   ```bash
   curl -I http://server/boot/kernel.efi
   # Check server response and headers
   ```

2. **Verify SSL Configuration:**
   - Check certificate validity
   - Verify TLS protocol support

3. **Check Network Settings:**
   - Verify DNS resolution
   - Test proxy settings if applicable

**Common Solutions:**
- Use HTTP instead of HTTPS for testing
- Update server certificates
- Check network proxy configuration
- Verify server accessibility

## Configuration Issues

### Issue: Configuration File Not Found

**Symptoms:**
- "Configuration file not found" message
- Default settings applied unexpectedly
- Custom settings ignored

**Diagnostic Steps:**

1. **Check File Locations:**
   ```
   /EFI/BOOT/bloodhorn.ini
   /bloodhorn.ini
   /config/bloodhorn.ini
   ```

2. **Verify File Format:**
   - Check INI syntax (sections, key=value pairs)
   - Validate JSON format if using JSON config
   - Ensure proper encoding (UTF-8)

3. **Test Configuration Parsing:**
   - Use recovery shell to view configuration
   - Check for parsing error messages

**Common Solutions:**
- Place configuration file in correct location
- Fix syntax errors in configuration file
- Use UEFI variables as alternative configuration
- Check file permissions and accessibility

### Issue: Invalid Configuration Values

**Symptoms:**
- "Invalid parameter" messages
- Settings not applied correctly
- Default values used instead

**Diagnostic Steps:**

1. **Validate Numeric Values:**
   ```ini
   # Check ranges
   menu_timeout=30        # 0-300 seconds
   font_size=12           # 8-72 pixels
   ```

2. **Verify String Values:**
   - Check for special characters
   - Verify path formats
   - Ensure proper quoting if needed

3. **Check Boolean Values:**
   ```ini
   # Valid boolean formats
   secure_boot=true
   tpm_enabled=1
   use_gui=yes
   ```

**Common Solutions:**
- Correct invalid parameter values
- Use supported configuration options
- Check documentation for valid ranges
- Validate configuration before deployment

## Memory and Performance Issues

### Issue: Out of Memory Errors

**Symptoms:**
- "Out of resources" messages
- Memory allocation failures
- System hangs during boot

**Diagnostic Steps:**

1. **Check Available Memory:**
   - Verify system has sufficient RAM
   - Check memory usage during boot
   - Monitor memory allocation patterns

2. **Analyze Memory Leaks:**
   - Check for unfreed allocations
   - Verify proper cleanup in error paths
   - Monitor memory map changes

3. **Optimize Memory Usage:**
   - Reduce buffer sizes where possible
   - Free unused memory promptly
   - Use memory pools for frequent allocations

**Common Solutions:**
- Increase system RAM if possible
- Optimize memory allocation in custom code
- Reduce feature set to lower memory requirements
- Check for memory leaks in third-party libraries

### Issue: Slow Boot Performance

**Symptoms:**
- Boot takes unusually long time
- Long delays between menu options
- Slow file loading

**Diagnostic Steps:**

1. **Profile Boot Process:**
   - Time each boot stage
   - Identify bottlenecks
   - Check for unnecessary delays

2. **Optimize File Access:**
   - Use faster storage media
   - Optimize file locations
   - Reduce file sizes where possible

3. **Reduce Feature Overhead:**
   - Disable unused features
   - Optimize graphics operations
   - Minimize network operations

**Common Solutions:**
- Disable unnecessary features
- Use faster storage (SSD vs HDD)
- Optimize configuration for speed
- Update to latest BloodHorn version

## Architecture-Specific Issues

### x86/x86_64 Issues

**Common Problems:**
- Legacy BIOS compatibility
- 32-bit vs 64-bit mismatches
- PAE memory addressing

**Solutions:**
- Ensure correct architecture build
- Check CPU compatibility
- Verify firmware mode (UEFI vs Legacy)

### ARM64 Issues

**Common Problems:**
- Device Tree Blob (DTB) issues
- ARM-specific boot protocols
- Memory alignment requirements

**Solutions:**
- Verify DTB file integrity
- Check ARM boot loader protocol
- Ensure proper memory alignment

### RISC-V Issues

**Common Problems:**
- Limited firmware support
- RISC-V-specific boot requirements
- Toolchain compatibility

**Solutions:**
- Use compatible firmware version
- Verify RISC-V boot protocol
- Check toolchain compatibility

## Debugging Techniques

### Enable Debug Output

1. **Compile with Debug Symbols:**
   ```bash
   build -a X64 -p BloodHorn/BloodHorn.dsc -t GCC5 -D DEBUG
   ```

2. **Enable Verbose Logging:**
   ```ini
   [debug]
   verbose=true
   log_level=debug
   ```

3. **Use UEFI Shell for Debugging:**
   ```bash
   # Load BloodHorn in shell for testing
   fs0:\EFI\BOOT\BloodHorn.efi -debug
   ```

### Memory Debugging

1. **Enable Memory Tracking:**
   ```c
   #define MEMORY_DEBUG 1
   // Track allocations and deallocations
   ```

2. **Check Memory Map:**
   ```c
   // Print memory map before ExitBootServices
   PrintMemoryMap();
   ```

3. **Validate Pointers:**
   ```c
   // Add pointer validation
   if (!Buffer || Buffer == (VOID*)0xFFFFFFFF) {
       return EFI_INVALID_PARAMETER;
   }
   ```

### Network Debugging

1. **Enable Network Tracing:**
   ```ini
   [network]
   debug=true
   trace_packets=true
   ```

2. **Test Network Connectivity:**
   ```bash
   # Use UEFI shell network commands
   ifconfig -a
   ping <server_ip>
   ```

### Graphics Debugging

1. **Check Graphics Modes:**
   ```c
   // List available graphics modes
   ListGraphicsModes();
   ```

2. **Test Framebuffer Access:**
   ```c
   // Simple test pattern
   DrawTestPattern();
   ```

## Common Error Messages

### EFI Status Codes

| Error | Meaning | Solution |
|-------|---------|----------|
| EFI_NOT_FOUND | File/protocol not found | Check file paths, verify hardware |
| EFI_ACCESS_DENIED | Permission denied | Check file permissions, Secure Boot |
| EFI_OUT_OF_RESOURCES | Insufficient memory | Reduce memory usage, add RAM |
| EFI_DEVICE_ERROR | Hardware failure | Check hardware compatibility |
| EFI_TIMEOUT | Operation timed out | Increase timeouts, check network |

### BloodHorn-Specific Messages

| Message | Cause | Solution |
|---------|-------|----------|
| "Coreboot initialization failed" | Coreboot not detected | Check Coreboot installation |
| "TPM 2.0 not available" | TPM hardware missing | Disable TPM or add hardware |
| "Kernel hash verification failed" | Hash mismatch | Update hash or replace kernel |
| "Graphics initialization failed" | Graphics driver issue | Disable GUI or update drivers |
| "Network boot failed" | Network problems | Check network configuration |

## Recovery Procedures

### Emergency Recovery Shell

1. **Access Recovery Shell:**
   - Select "Recovery Shell" from boot menu
   - Or press 'R' during boot timeout

2. **Common Recovery Commands:**
   ```bash
   # List files
   ls fs0:\EFI\BOOT\
   
   # Check configuration
   cat fs0:\bloodhorn.ini
   
   # Test kernel loading
   load kernel fs0:\vmlinuz
   
   # Verify file integrity
   verify fs0:\kernel.efi
   ```

### Backup Bootloader

1. **Create Backup:**
   ```bash
   # Backup current bootloader
   cp /EFI/BOOT/BloodHorn.efi /EFI/BOOT/BloodHorn.efi.backup
   ```

2. **Restore from Backup:**
   ```bash
   # Restore if issues occur
   cp /EFI/BOOT/BloodHorn.efi.backup /EFI/BOOT/BloodHorn.efi
   ```

### Fallback Boot Options

1. **Chainload Other Bootloaders:**
   ```ini
   [boot]
   fallback=chainload
   chainload_target=/EFI/GRUB/grubx64.efi
   ```

2. **UEFI Shell Fallback:**
   ```ini
   [boot]
   fallback=uefi_shell
   shell_path=/EFI/TOOLS/shellx64.efi
   ```

### System Recovery

1. **Reinstall BloodHorn:**
   - Boot from installation media
   - Reinstall BloodHorn to EFI partition
   - Restore configuration from backup

2. **Reset to Defaults:**
   - Remove configuration file
   - BloodHorn will use built-in defaults
   - Reconfigure as needed

## Getting Help

### Community Support

- **GitHub Issues**: Report bugs and request features
- **Forums**: Community discussion and support
- **IRC/Chat**: Real-time assistance

### Reporting Bugs

When reporting issues, include:

1. **System Information:**
   - Hardware specifications
   - Firmware version and settings
   - BloodHorn version and build options

2. **Error Details:**
   - Complete error messages
   - Steps to reproduce
   - Debug output if available

3. **Configuration:**
   - BloodHorn configuration file
   - System boot configuration
   - Network settings (if applicable)

### Contributing Fixes

1. **Fork Repository:**
   - Create fork on GitHub
   - Create feature branch

2. **Test Thoroughly:**
   - Test on multiple systems
   - Verify no regressions
   - Update documentation

3. **Submit Pull Request:**
   - Include detailed description
   - Add test cases
   - Update changelog

This troubleshooting guide should help resolve most common BloodHorn issues. For additional support, consult the community resources and documentation.
