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
#include "include/bldr.h"
#include "include/oal_args.h"

/**
  BootLoaderPreLoad(), This function will try open ULDR partition if it exist or
  update mode in Boot Config is TRUE, set Boot atate is BOOT_STATE_LOAD_ULDR and return.
  Because ULDR is not implement at this time, so commend out relation code and set Boot State
  default to BOOT_STATE_LOAD_OS.

  @param  pLoader   A pointer to the BootLoader_t content.

  @return enum_t    A Boot State value.

**/
enum_t
BootLoaderPreLoad(
    BootLoader_t *pLoader
    )
{
    enum_t  rc = BOOT_STATE_LOAD_OS;
    bool_t* pUpdateMode;

//******************************************************************************
// To operate ULDR is not support yet, so comment out
#if 0
    handle_t hBlock = NULL, hPartition = NULL;
    uint8_t *pSector = NULL;
    size_t sectorSize;
#endif
//******************************************************************************

    // Look for update flag in memory
    pUpdateMode = BootArgsQuery(pLoader, OAL_ARGS_QUERY_UPDATEMODE);
    if ((pUpdateMode != NULL) && (*pUpdateMode != 0)) {
        rc = BOOT_STATE_LOAD_ULDR;
        goto cleanUp;
    }

//******************************************************************************
// To operate ULDR is not support yet, so comment out
#if 0
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
#endif
//******************************************************************************

cleanUp:

//******************************************************************************
// To operate ULDR is not support yet, so comment out
#if 0
    BootFree(pSector);
    BootBlockClose(hPartition);
    BootBlockDeinit(hBlock);
#endif
//******************************************************************************
    return rc;
}

//------------------------------------------------------------------------------

