// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
#ifndef __BOOT_FILESYSTEM_H
#define __BOOT_FILESYSTEM_H

#include <Library/UefiLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Library/UefiBootServicesTableLib.h>

#include "include/boottypes.h"


#ifdef __cplusplus
extern "C" {
#endif


EFI_STATUS
EFIAPI
BootFileSystemInit(
    );

EFI_STATUS
EFIAPI
BootFileSystemOpen(
    CHAR16          *FileName,
    EFI_FILE_HANDLE *pFileHandle,
    UINT32          *pFileLength
    );

UINT32
BootFileSystemRead(
    EFI_FILE_HANDLE hFileHandle,
    void *pAddress,
    UINT32 Length
    );

BOOLEAN
BootFileSystemRewind(
    EFI_FILE_HANDLE hFileHandle,
    UINT32 offset
    );

void
BootFileSystemClose(
    EFI_FILE_HANDLE hFileHandle
    );


#ifdef __cplusplus
}
#endif

#endif // __BOOT_FILESYSTEM_H