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
  BootLoaderLoadUldr(), This function will do Update loader if Boot state is BOOT_STATE_LOAD_ULDR
  or update mode in Boot Config is TRUE.
  This function is not implement yet, so it will do nothing and set BOOT_STATE_LOAD_OS
  for next step.

  @param  pLoader   A pointer to the BootLoader_t content.

  @return bool_t    A BOOL value to indicate ULDR is executed successfully or not.

**/

bool_t
BootLoaderLoadUldr(
    BootLoader_t* pLoader
    )
{
    enum_t rc = (enum_t)BOOT_STATE_FAILURE;

//******************************************************************************
// To operate ULDR is not support yet, so comment out
#if 0
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
//******************************************************************************
#else
    rc = BOOT_STATE_LOAD_OS;
#endif

    return rc;
}

//------------------------------------------------------------------------------

