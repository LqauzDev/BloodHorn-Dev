/*
 * secure.c
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#include <Uefi.h>
#include "compat.h"
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseCryptLib.h>
#include <Protocol/ImageAuthentication.h>
#include <Guid/ImageAuthentication.h>
#include <Guid/GlobalVariable.h>

EFI_STATUS EFIAPI VerifyImageSignature(
    IN CONST VOID    *ImageBuffer,
    IN UINTN         ImageSize,
    IN CONST UINT8   *PublicKey,
    IN UINTN         PublicKeySize
) {
    if (!ImageBuffer || ImageSize == 0 || !PublicKey || PublicKeySize == 0) {
        return EFI_INVALID_PARAMETER;
    }
    
    // Validate signature size - don't assume 256 bytes
    if (ImageSize < 64) return EFI_SECURITY_VIOLATION; // Minimum signature size
    
    // Try different signature formats
    UINTN SignatureSize = 256; // Default to RSA-2048
    if (PublicKeySize >= 512) {
        // Check if it's RSA-4096
        uint32_t key_bits = (PublicKey[0] << 24) | (PublicKey[1] << 16) | (PublicKey[2] << 8) | PublicKey[3];
        if (key_bits >= 4096) {
            SignatureSize = 512;
        }
    }
    
    if (ImageSize < SignatureSize) {
        return EFI_SECURITY_VIOLATION;
    }
    
    UINTN DataSize = ImageSize - SignatureSize;
    CONST UINT8* Data = (CONST UINT8*)ImageBuffer;
    CONST UINT8* Signature = Data + DataSize;
    UINT8 Hash[32];
    
    if (!Sha256HashAll(Data, DataSize, Hash)) {
        return EFI_SECURITY_VIOLATION;
    }
    
    // Use our improved crypto verification
    int crypto_result = verify_signature(Data, (uint32_t)DataSize, Signature, PublicKey);
    if (crypto_result != CRYPTO_SUCCESS) {
        return EFI_SECURITY_VIOLATION;
    }
    
    return EFI_SUCCESS;
}

BOOLEAN EFIAPI IsSecureBootEnabled(VOID) {
    EFI_STATUS Status;
    UINT8 SecureBoot = 0;
    UINTN DataSize = sizeof(SecureBoot);
    Status = gRT->GetVariable(L"SecureBoot", &gEfiGlobalVariableGuid, NULL, &DataSize, &SecureBoot);
    return !EFI_ERROR(Status) && (SecureBoot == 1);
}

EFI_STATUS EFIAPI LoadAndVerifyKernel(
    IN  CHAR16  *FileName,
    OUT VOID    **ImageBuffer,
    OUT UINTN   *ImageSize
) {
    if (!FileName || !ImageBuffer || !ImageSize) {
        return EFI_INVALID_PARAMETER;
    }
    
    EFI_STATUS Status;
    VOID *Buffer = NULL;
    UINTN Size = 0;
    
    Status = ReadFile(FileName, &Buffer, &Size);
    if (EFI_ERROR(Status)) return Status;
    
    if (IsSecureBootEnabled()) {
        // Load public key from a secure variable
        UINT8 PublicKey[512]; // Support up to RSA-4096
        UINTN PublicKeySize = sizeof(PublicKey);
        Status = gRT->GetVariable(L"PK", &gEfiGlobalVariableGuid, NULL, &PublicKeySize, PublicKey);
        if (EFI_ERROR(Status)) {
            FreePool(Buffer);
            return Status;
        }
        
        Status = VerifyImageSignature(Buffer, Size, PublicKey, PublicKeySize);
        if (EFI_ERROR(Status)) {
            FreePool(Buffer);
            // Zero out sensitive data
            crypto_zeroize(PublicKey, sizeof(PublicKey));
            return Status;
        }
        
        // Zero out public key after use
        crypto_zeroize(PublicKey, sizeof(PublicKey));
    }
    
    *ImageBuffer = Buffer;
    *ImageSize = Size;
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI ExecuteKernel(
    IN VOID     *ImageBuffer,
    IN UINTN    ImageSize,
    IN CHAR16   *CmdLine OPTIONAL
) {
    EFI_STATUS Status;
    EFI_HANDLE ImageHandle = NULL;
    Status = gBS->LoadImage(FALSE, gImageHandle, NULL, ImageBuffer, ImageSize, &ImageHandle);
    if (EFI_ERROR(Status)) return Status;
    if (CmdLine != NULL) {
        EFI_LOADED_IMAGE_PROTOCOL* LoadedImage = NULL;
        gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage);
        if (LoadedImage != NULL) {
            LoadedImage->LoadOptions = CmdLine;
            LoadedImage->LoadOptionsSize = (StrLen(CmdLine) + 1) * sizeof(CHAR16);
        }
    }
    Status = gBS->StartImage(ImageHandle, NULL, NULL);
    if (EFI_ERROR(Status)) gBS->UnloadImage(ImageHandle);
    return Status;
}