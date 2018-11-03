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
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiMemoryLib/MemLibInternals.h>

#include "include/bldr.h"
#include "include/bootdebug.h"
#include "include/oal_args.h"

//------------------------------------------------------------------------------
//  Global variables

// dwZoneMask if define the DBGMSG ZONE enable/disable, now it has belwo
// definition, if develope want to add more ZONE, they can modifiy in Bootdebug.h
// and enable/disable in here
//      DBGZONE_ERROR      DBGZONE(0)
//      DBGZONE_WARNING    DBGZONE(1)
//      DBGZONE_INFO       DBGZONE(2)
DWORD   dwZoneMask = 0x00000000;

// Default debug zones
#define DBGZONE_ERROR      DBGZONE(0)
#define DBGZONE_WARNING    DBGZONE(1)
#define DBGZONE_INFO       DBGZONE(2)
UINT32  gBootArgAddress;
VOID*   pAvailMem = NULL;


/**
  In BootArgsInit(), it's used to initial Boot Argments to default value.

  @param  force     Indicate Boot Argments need initial or not, for CrownBay,
                    this area is always flushed after reboot.

**/
void
BootArgsInit(
    bool_t force
    )
{
    BOOT_ARGS **ppBootArgs = (BOOT_ARGS **)IMAGE_SHARE_BOOT_ARGS_PTR_PA;
    BOOT_ARGS *pBootArgs = (BOOT_ARGS *)IMAGE_SHARE_BOOT_ARGS_PA;
    
    gBootArgAddress  = IMAGE_SHARE_BOOT_ARGS_PA;

    if ((pBootArgs->dwSig != BOOTARG_SIG) ||
        (pBootArgs->dwLen < sizeof(BOOT_ARGS)) ||
        (*ppBootArgs != pBootArgs)) {
        force = true;
    }

    if (force) {
        // Allocate buffer from UEFI.
        if (AllocateImageBuffer(&gBootArgAddress, EFI_PAGE_SIZE) == TRUE) {
            *ppBootArgs = pBootArgs;
            InternalMemSetMem(pBootArgs, sizeof(*pBootArgs), 0);
            pBootArgs->dwSig = BOOTARG_SIG;
            pBootArgs->dwLen = sizeof(BOOT_ARGS);
            pBootArgs->dwVersionSig = BOOT_ARG_VERSION_SIG;
            pBootArgs->MajorVersion = BOOT_ARG_MAJOR_VER;
            pBootArgs->MinorVersion = BOOT_ARG_MINOR_VER;

            // Debug
            pBootArgs->ucEdbgAdapterType   = EDBG_ADAPTER_DEFAULT;
            pBootArgs->ucComPort           = DEFAULT_COMPORT;
            pBootArgs->ucBaudDivisor       = DEFAULT_BAUDRATE;

            // Video
            pBootArgs->cxDisplayScreen     = DEFAULT_DISPLAY_WIDTH;
            pBootArgs->cyDisplayScreen     = DEFAULT_DISPLAY_HEIGHT;
            pBootArgs->bppScreen           = DEFAULT_DISPLAY_DEPTH;

            pBootArgs->cxPhysicalScreen     = DEFAULT_DISPLAY_WIDTH;
            pBootArgs->cyPhysicalScreen     = DEFAULT_DISPLAY_HEIGHT;

            // Boot
            CopyMem(pBootArgs->szDeviceNameRoot, DEFAULT_DEVICE_NAME_ROOT,
                MAX_DEV_NAMELEN);

            pBootArgs->ucLoaderFlags       = 0;         // Flags set by loader
            pBootArgs->ucPCIConfigType     = 1;
            pAvailMem = (VOID *)gBootArgAddress;
        }
    }

    DBGMSG(DBGZONE_INFO, (L"dwSig             = 0x%X \n", pBootArgs->dwSig));
    DBGMSG(DBGZONE_INFO, (L"dwVersionSig      = 0x%X \n", pBootArgs->dwVersionSig));
    DBGMSG(DBGZONE_INFO, (L"dwLen             = 0x%X \n", pBootArgs->dwLen));
    DBGMSG(DBGZONE_INFO, (L"ucEdbgAdapterType = %d \n",   pBootArgs->ucEdbgAdapterType));
    DBGMSG(DBGZONE_INFO, (L"ucComPort         = COM%d \n",pBootArgs->ucComPort));
    DBGMSG(DBGZONE_INFO, (L"ucBaudDivisor     = %d \n",   pBootArgs->ucBaudDivisor));
    DBGMSG(DBGZONE_INFO, (L"dwZoneMask        = 0x%X \n", dwZoneMask));
    DBGMSG(DBGZONE_INFO, (L"cxDisplayScreen   = %d \n",   pBootArgs->cxDisplayScreen));
    DBGMSG(DBGZONE_INFO, (L"cyDisplayScreen   = %d \n",   pBootArgs->cyDisplayScreen));
    DBGMSG(DBGZONE_INFO, (L"bppScreen         = %d \n",   pBootArgs->bppScreen));
    DBGMSG(DBGZONE_INFO, (L"szDeviceNameRoot  = '%c%c%c%c%c%c%c%c%c'\n",
        pBootArgs->szDeviceNameRoot[0], pBootArgs->szDeviceNameRoot[1],
        pBootArgs->szDeviceNameRoot[2], pBootArgs->szDeviceNameRoot[3],
        pBootArgs->szDeviceNameRoot[4], pBootArgs->szDeviceNameRoot[5],
        pBootArgs->szDeviceNameRoot[6], pBootArgs->szDeviceNameRoot[7],
        pBootArgs->szDeviceNameRoot[8]));
    DBGMSG(DBGZONE_INFO, (L"ucLoaderFlags    = 0x%X \n", pBootArgs->ucLoaderFlags));
    DBGMSG(DBGZONE_INFO, (L"ucPCIConfigType  = 0x%X \n", pBootArgs->ucPCIConfigType));

}

/**
  In BootArgsQuery(), it's used to query update mode state for decide run ULDR
  or not before into Boot Menu.

  @param  pLoader   A pointer to point a contant in BootLoader_t structure.

  @param  type      Indicate what type request will be queried.

**/
void*
BootArgsQuery(
    BootLoader_t    *pLoader,
    enum_t          type
    )
{
    void *pData = NULL;
    BOOT_ARGS* pBootArgs = (BOOT_ARGS *)IMAGE_SHARE_BOOT_ARGS_PA;
    static BOOLEAN updateMode = FALSE;

    // Check if there is expected signature
    if ((pBootArgs->dwSig != BOOTARG_SIG) ||
        (pBootArgs->dwLen < sizeof(BOOT_ARGS)))
        goto cleanUp;

    switch (type)
    {
        case OAL_ARGS_QUERY_UPDATEMODE:
            updateMode =
            (pLoader->imageUpdateFlags & OAL_ARGS_UPDATEMODE) ? TRUE : FALSE;
            pData = &updateMode;
            break;
    }

cleanUp:
    return pData;
}

//------------------------------------------------------------------------------


