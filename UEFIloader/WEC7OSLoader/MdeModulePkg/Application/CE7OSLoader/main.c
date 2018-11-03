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

#include "include/bldr.h"

/**
  OEMBootLoad(), This function will call a booting step by Boot State, and return
  a Boot State for UefiMain() to decide to run next booting step or not..

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  BootState     A Boot State type to decide what step should to run at this time.

  @return enum_t        A Boot State value.

**/
enum_t
OEMBootLoad(
    void *pContext,
    enum_t  BootState
    )
{
    enum_t          Status = (enum_t) BOOT_STATE_FAILURE;
    BootLoader_t*   pLoader = pContext;

    DBGMSG(DBGZONE_INFO, (L"+OEMBootLoad(0x%08x, 0x%08x)\r\n", pContext, Status));

    // Depending on state
    switch (BootState)
        {
        case BOOT_STATE_POWERON:
            Status = BootLoaderPowerOn(pLoader);
            break;
        case BOOT_STATE_CONFIG:
            Status = BootLoaderConfig(pLoader);
            break;
//******************************************************************************
// Not NIC Support yet, so comment out
#if 0
        case BOOT_STATE_DOWNLOAD:
            Status = BootLoaderDownload(pLoader);
            break;
#endif
//******************************************************************************

//******************************************************************************
// Preload and Uldr is not support yet, so just present but nothing will do and
// just goto next BootState
        case BOOT_STATE_PRELOAD:
            Status = BootLoaderPreLoad(pLoader);
            break;
        case BOOT_STATE_LOAD_ULDR:
            Status = BootLoaderLoadUldr(pLoader);
            break;
//******************************************************************************
        case BOOT_STATE_LOAD_OS:
            Status = BootLoaderLoadOs(pLoader);
            break;
        }

    DBGMSG(DBGZONE_INFO, (L"-OEMBootLoad(rc = 0x%08x)\r\n", Status));
    return Status;
}

//------------------------------------------------------------------------------
