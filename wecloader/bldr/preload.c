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

enum_t
BootLoaderPreLoad(
    BootLoader_t *pLoader
    )
{
    enum_t rc = BOOT_STATE_LOAD_OS;
    BOOL* pUpdateMode;
    handle_t hBlock = NULL, hPartition = NULL;
    uint8_t *pSector = NULL;
    size_t sectorSize;


    // Look for update flag in memory
    pUpdateMode = BootArgsQuery(pLoader, OAL_ARGS_QUERY_UPDATEMODE);
    if ((pUpdateMode != NULL) && (*pUpdateMode != 0))
        {
        rc = BOOT_STATE_LOAD_ULDR;
        goto cleanUp;
        }

    // Open ULDR partitions (if it doesn't exist it is ok)...
    hBlock = OEMBootCreateDriver(pLoader, BOOT_DRIVER_CLASS_BLOCK, 0);
    if (hBlock == NULL) goto cleanUp;
    
    hPartition = BootBlockOpenPartition(hBlock, 0x20, 0);
    if (hPartition != NULL)
        {
        // Get sector size & allocate memory for it
        sectorSize = BootBlockSectorSize(hBlock);
        pSector = BootAlloc(sectorSize);
        if ((sectorSize > 0) && (pSector != NULL))
            {
            // Read first sector from partition
            if (BootBlockRead(hPartition, 0, 1, pSector))
                {
                // Information about ULDR mode is on byte 56
                if (pSector[56] != 0) rc = BOOT_STATE_LOAD_ULDR;
                }
            }
        }
    
cleanUp:
    BootFree(pSector);
    BootBlockClose(hPartition);
    BootBlockDeinit(hBlock);
    return rc;
}

//------------------------------------------------------------------------------

