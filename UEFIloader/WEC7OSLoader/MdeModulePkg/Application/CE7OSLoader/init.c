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
#include <Protocol/LoadedImage.h>
#include <Protocol/GraphicsOutput.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiMemoryLib/MemLibInternals.h>

#include "include/bldr.h"
#include "include/bootdebug.h"
#include "include/RamTop.h"

//------------------------------------------------------------------------------
//  Global variables
EFI_LOADED_IMAGE                *g_LoadedImage;
EFI_GRAPHICS_OUTPUT_PROTOCOL    *g_GraphicsOutput = NULL;

extern EFI_HANDLE               gCEImageHandle;
extern EFI_SYSTEM_TABLE         *gCESystemTable;

//------------------------------------------------------------------------------
//******************************************************************************
// Because to compile below code need many c files and relative header files
// from Original WEC7, so now is command out
#if 0
static
const
BootEdbgDriver_t
s_Rtl8139 = {
    RTL8139InitDMABuffer,
    RTL8139Init,
    NULL,
    RTL8139SendFrame,
    RTL8139GetFrame,
    NULL
};

static
const
BootEdbgDriver_t
s_Dec21140 = {
    DEC21140InitDMABuffer,
    DEC21140Init,
    NULL,
    DEC21140SendFrame,
    DEC21140GetFrame,
    NULL
};

static
const
BootEdbgDriver_t
s_Ne2000 = {
    NULL,
    NE2000Init,
    NULL,
    NE2000SendFrame,
    NE2000GetFrame,
    NULL
};


static
const
Device_t
s_devices[] = {
    { L"Boot Disk", DeviceTypeStore, (enum_t)IfcTypeUndefined, 0,    NULL         },
    { L"RTL8139",   DeviceTypeEdbg,  IfcTypePci,                0x12111113, &s_Rtl8139   },
    { L"RTL8139",   DeviceTypeEdbg,  IfcTypePci,                0x13001186, &s_Rtl8139   },
    { L"RTL8139",   DeviceTypeEdbg,  IfcTypePci,                0x813910EC, &s_Rtl8139   },
    { L"DEC21140",  DeviceTypeEdbg,  IfcTypePci,                0x00091011, &s_Dec21140  },
    { L"NE2000",    DeviceTypeEdbg,  IfcTypePci,                0x09401050, &s_Ne2000    },
    { L"NE2000",    DeviceTypeEdbg,  IfcTypePci,                0x802910EC, &s_Ne2000    }
};

#endif
//******************************************************************************

//------------------------------------------------------------------------------

static
const
Device_t
s_devices[] = {
    { L"Boot Disk", DeviceTypeStore, (enum_t)IfcTypeUndefined,  0,          NULL    },
    { L"RTL8139",   DeviceTypeEdbg,  IfcTypePci,                0x12111113, NULL    },
    { L"RTL8139",   DeviceTypeEdbg,  IfcTypePci,                0x13001186, NULL    },
    { L"RTL8139",   DeviceTypeEdbg,  IfcTypePci,                0x813910EC, NULL    },
    { L"DEC21140",  DeviceTypeEdbg,  IfcTypePci,                0x00091011, NULL    },
    { L"NE2000",    DeviceTypeEdbg,  IfcTypePci,                0x09401050, NULL    },
    { L"NE2000",    DeviceTypeEdbg,  IfcTypePci,                0x802910EC, NULL    }
};


static
BootLoader_t
s_bootLoader;


/**
  OEMBootInit(), This function will do:
  1. Initial BOOTARGS.
  2. Create local Boot config.
  3. Get UEFI_LOAD_IMAGE handle for boot device.
  4. Get GraphicOutput handle for get video information.
  5. Get available RAM size for pass to OS.

  @return pLoader    A pointer to a BootLoader_t for local Boot Config.

**/
void*
OEMBootInit(
    )
{
    EFI_STATUS                      Status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput = NULL;
    BootLoader_t                    *pLoader = &s_bootLoader;

    InternalMemSetMem(pLoader, sizeof(s_bootLoader), 0);

    // Initialize boot arguments
    BootArgsInit(false);

    // Initalize supported drivers info
    pLoader->devices = dimof(s_devices);
    pLoader->pDevices = s_devices;

    if ((gCEImageHandle == NULL) ||
        (gCESystemTable == NULL)) {
        ERRMSG(TRUE, (L"OEMBootInit(): Invalid ImageHandle or SystemHandle.\n"));
        return pLoader = NULL;
    }

    //Get Boot device info
    Status = gBS->HandleProtocol(
                    gCEImageHandle,
                    &gEfiLoadedImageProtocolGuid,
                    (VOID**)&g_LoadedImage
                    );

    if (EFI_ERROR(Status)) {
         ERRMSG(TRUE, (L"OEMBootInit(): Can not retrieve a "
            L"LoadedImageProtocol handle,(Status = 0x%x)\n", Status));
        return pLoader = NULL;
    }

     // Find if there is VBE support (VESA)
    Status = gBS->LocateProtocol(
                    &gEfiGraphicsOutputProtocolGuid,
                    NULL,
                    (VOID**)&GraphicsOutput
                    );


    if (EFI_ERROR(Status)) {
        ERRMSG(TRUE, (L"OEMBootInit(): Can not acquire "
            L"GraphicsOutputProtoco handle,(Status = 0x%x)\n", Status));
        return pLoader = NULL;
    }
    else
        g_GraphicsOutput = GraphicsOutput;

//******************************************************************************
// Reference BIOS loader reported RAM size to make WCE7 booting well
// Finally, we need use UEFI function to get correct RAM size.
    {
        UINT64    ramTop = 0;
        pLoader->ramTop = EFI_ERROR(GetRamTop(&ramTop)) ? 0x3f500000 : ramTop;
    }
//******************************************************************************

    pLoader->ramestimated = true;

    pLoader->videoRam = 0;

    // Done
    return pLoader;
}

/**
  AllocateImageBuffer(), This function will allocate numbers page im RAM that count by
  ImageLength parameter.

  @param  pImageStartingAddr    A Pointer to a address that Image header indicate image
                                start address in RAM.

  @param  ImageLength           From Image header to indicate how mnamy bytes the NK is be.

  @return BOOLEAN               A BOOL value to indicate memory is allocate or not.

**/
BOOLEAN
AllocateImageBuffer(
    UINT32 *pImageStartingAddr,
    UINT32 ImageLength
    )
{
    EFI_STATUS            Status;
    UINT32                NumberOfPages = EFI_SIZE_TO_PAGES(ImageLength);
    EFI_PHYSICAL_ADDRESS  LoadStartAddress;
    UINT32                imageOffset;

    LoadStartAddress = (EFI_PHYSICAL_ADDRESS)*pImageStartingAddr;
    LoadStartAddress &= ~EFI_PAGE_MASK;
    imageOffset = *pImageStartingAddr - (UINT32)LoadStartAddress;

    ImageLength += imageOffset;
    NumberOfPages = EFI_SIZE_TO_PAGES(ImageLength);

    INFOMSG(TRUE, (L"NumberOfPages = 0x%X \n", NumberOfPages));
    INFOMSG(TRUE, (L"LoadStartAddress 0x%X \n", LoadStartAddress));

    Status = gBS->AllocatePages(
                               AllocateAddress,
                               EfiLoaderData,
                               NumberOfPages,
                               &LoadStartAddress
                               );

    if (EFI_ERROR(Status) || (LoadStartAddress == 0)) {
        ERRMSG(TRUE,
            (L"Unable to allocate image buffer, "
            L"Status 0x%X, LoadStartAddress = 0x%X\n",
            Status, LoadStartAddress));
    }
    else {
        *pImageStartingAddr = (UINT32)LoadStartAddress;
        INFOMSG(TRUE, (L"Allocate image buffer OK, Status 0x%X, "
            L"LoadStartAddress = 0x%X\n",
            Status, LoadStartAddress));
    }

    return EFI_ERROR(Status) ? FALSE : TRUE;
}

//------------------------------------------------------------------------------