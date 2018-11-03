//
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
//
// -- Intel Copyright Notice -- 
//  
// @par 
// Copyright (c) 2002-2011 Intel Corporation All Rights Reserved. 
//  
// @par 
// The source code contained or described herein and all documents 
// related to the source code ("Material") are owned by Intel Corporation 
// or its suppliers or licensors.  Title to the Material remains with 
// Intel Corporation or its suppliers and licensors. 
//  
// @par 
// The Material is protected by worldwide copyright and trade secret laws 
// and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, 
// distributed, or disclosed in any way except in accordance with the 
// applicable license agreement . 
//  
// @par 
// No license under any patent, copyright, trade secret or other 
// intellectual property right is granted to or conferred upon you by 
// disclosure or delivery of the Materials, either expressly, by 
// implication, inducement, estoppel, except in accordance with the 
// applicable license agreement. 
//  
// @par 
// Unless otherwise agreed by Intel in writing, you may not remove or 
// alter this notice or any other notice embedded in Materials by Intel 
// or Intel's suppliers or licensors in any way. 
//  
// @par 
// For further details, please see the file README.TXT distributed with 
// this software. 
//  
// @par 
// -- End Intel Copyright Notice -- 
//  
#include <Library/UefiLib.h>
#include <Protocol/Runtime.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "include/bldr.h"
#include "include/boot.h"
#include "include/bootdebug.h"

//------------------------------------------------------------------------------
//  Global variables

/**
  BootLoaderLoadOs(), This function is main function to load OS, it will call:
    1.BootFileSystemInit(), to initial Boot device Path and File Buffer for cache.
    2.BootFileSystemOpen(), to open nk image , get file handle and file lenght.
    3.BootFileSystemReadBinFile(), to read NK.bin to RAM.

  @param  pLoader               A pointer to the BootLoader_t content..

  @return BOOT_STATE_FAILURE    Indicate above three functions has error to executed.
  @return EFI_SUCCESS           This function is executed successfully.

**/
enum_t
BootLoaderLoadOs(
    BootLoader_t    *pLoader
    )
{
    EFI_FILE_HANDLE hFileHandle = NULL;
    CHAR16          ImageFileName[64];
    UINT32          nFileLengh = 0;
    enum_t          rc = (enum_t)BOOT_STATE_FAILURE;
    uint32_t        address = 0;
    EFI_STATUS      Status;
    EFI_TIME        Time;

//******************************************************************************
// To operate ULDR is not support yet, so Block BinFs need comment out
#if 0
    handle_t hBlock = NULL;
    handle_t hPartition = NULL;
    handle_t hFatFs = NULL;
    handle_t hFile = NULL;

    hBlock = OEMBootCreateDriver(pLoader, BOOT_DRIVER_CLASS_BLOCK, 0);
    if (hBlock == NULL) goto cleanUp;

    // First try open partition with RAMIMG
    hPartition = BootBlockOpenPartition(hBlock, 0x23, 0);
    if (hPartition != NULL)
    {
        // Load BinFs image to memory (with zero offset)
        if (!BootBlockLoadBinFsImage(hBlock, hPartition, 0, &address))
            goto cleanUp;

        // Get BINFS start address
        pLoader->runAddress = BootVAtoPA((VOID*)address);
    }
    else
    {
#endif
//******************************************************************************

    AsciiStrToUnicodeStr(WINCE_OSIMAGE_NAME, ImageFileName);

    gRT->GetTime(&Time, NULL);
    INFOMSG(PRINT_INFO, (L"%s - Time = 0x%x:0x%x\n", L"BootLoaderLoadOs:Start", Time.Minute, Time.Second));

    Status = BootFileSystemInit();
    if(EFI_ERROR(Status)) {
        ERRMSG(TRUE, (L"BootLoaderLoadOs(): Failed to find root location, "
            L"status is 0x%X\n", Status));
        goto CleanUp;
    }

    Status = BootFileSystemOpen(
                                ImageFileName,
                                &hFileHandle,
                                &nFileLengh
                                );
    if(EFI_ERROR(Status) || hFileHandle == NULL || nFileLengh == 0) {
        ERRMSG(TRUE, (L"BootLoaderLoadOs(): Failed to open NK.bin, "
            L"status is 0x%X\n", Status));
        goto CleanUp;
    }
    else
        INFOMSG(TRUE, (L"BootLoaderLoadOs(): Success to open NK.bin\n"));

    if(BootFileSystemReadBinFile(hFileHandle, 0, &address)) {
        // Save start address
        pLoader->runAddress = BootVAtoPA((VOID*)address);
        rc = BOOT_STATE_RUN;
    }

//******************************************************************************
// Comment out matched brace with ULDR BinFS
#if 0
    }
#endif
//******************************************************************************

CleanUp:
    BootFileSystemClose(hFileHandle);

    gRT->GetTime(&Time, NULL);
    INFOMSG(PRINT_INFO, (L"%s - Time = 0x%x:0x%x\n", L"BootLoaderLoadOs:End", Time.Minute, Time.Second));

    if (rc == BOOT_STATE_FAILURE) {
        ERRMSG(TRUE, (L"ERROR: No bootable OS found!\r\n"));
    }

    return rc;
}

//------------------------------------------------------------------------------

