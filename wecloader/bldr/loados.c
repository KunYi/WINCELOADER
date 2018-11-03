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
#include "bldr.h"
#include <bootBlockUtils.h>
#include <bootFileSystemFat.h>
#include <bootFileSystemUtils.h>

//------------------------------------------------------------------------------

bool_t
BootLoaderLoadOs(
    BootLoader_t* pLoader
    )
{
    enum_t rc = (enum_t)BOOT_STATE_FAILURE;
    handle_t hBlock = NULL;
    handle_t hPartition = NULL;
    handle_t hFatFs = NULL;
    handle_t hFile = NULL;
    uint32_t address = 0;

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
        // And FAT file system on it...
        hFatFs = BootFileSystemFatInit(hBlock, 0);
        if (hFatFs == NULL) goto cleanUp;

        // Look for nk.bin file
        hFile = BootFileSystemOpen(
            hFatFs, L"nk.bin", BOOT_FILESYSTEM_ACCESS_READ, 
            BOOT_FILESYSTEM_ATTRIBUTE_NORMAL
            );
        if (hFile == NULL) goto cleanUp;

        // Try to load it to memory (with zero offset)
        if (!BootFileSystemReadBinFile(hFile, 0, &address)) goto cleanUp;

        // Save start address
        pLoader->runAddress = BootVAtoPA((VOID*)address);
        }
    
    // Done
    rc = BOOT_STATE_RUN;
    
cleanUp:
    BootFileSystemClose(hFile);
    BootFileSystemDeinit(hFatFs);
    BootBlockClose(hPartition);
    BootBlockDeinit(hBlock);
    if (rc == BOOT_STATE_FAILURE)
        {
        BOOTMSG(ZONE_ERROR, (
            L"ERROR: No bootable OS found!\r\n"
            ));
        }
    return rc;
}

//------------------------------------------------------------------------------

