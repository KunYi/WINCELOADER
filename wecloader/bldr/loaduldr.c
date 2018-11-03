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
#include <oal_args.h>

//------------------------------------------------------------------------------

bool_t
BootLoaderLoadUldr(
    BootLoader_t* pLoader
    )
{
    enum_t rc = (enum_t)BOOT_STATE_FAILURE;
    handle_t hBlock = NULL;
    handle_t hPartition = NULL;
    uint32_t address = 0;


    hBlock = OEMBootCreateDriver(pLoader, BOOT_DRIVER_CLASS_BLOCK, 0);
    if (hBlock == NULL) goto cleanUp;

    // Open partition with ULDR
    hPartition = BootBlockOpenPartition(hBlock, 0x20, 0);
    if (hPartition == NULL)
        {
        BOOTMSG(ZONE_ERROR, (
            L"ERROR: Can't open ULDR partition (0x20)\r\n"
            ));
        goto cleanUp;
        }

    // Load BinFs image to memory (with zero offset)
    if (!BootBlockLoadBinFsImage(hBlock, hPartition, 0, &address))
        {
        BOOTMSG(ZONE_ERROR, (
            L"ERROR: Failed load BINFS image from ULDR partition!\r\n"
            ));
        goto cleanUp;
        }

    // Get BINFS start address
    pLoader->runAddress = BootVAtoPA((VOID*)address);

    // Done
    rc = BOOT_STATE_RUN;
    
cleanUp:
    BootBlockDeinit(hBlock);
    return rc;
}

//------------------------------------------------------------------------------

