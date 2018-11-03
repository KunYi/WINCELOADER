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
#include <Protocol/GraphicsOutput.h>
#include <Library/UefiMemoryLib/MemLibInternals.h>
#include <Library/BaseMemoryLib.h>

#include "include/bldr.h"

//------------------------------------------------------------------------------
//  Global variables
extern EFI_GRAPHICS_OUTPUT_PROTOCOL    *g_GraphicsOutput;

static
const
uint32_t
s_test[] = {
    0x55aaaa55, 0xaa5555aa, 12345678, 0xf00f4466
};

//------------------------------------------------------------------------------
//

//

/**
  In OEMBootRun() we need to prepare information passed to Intel WEC7 CrownBay
  in way it will understand. So until we modify ARGs we have to use little
  complicated and inconsistent way how to pass required information, if this
  function is succes, it will return a address that UefiMain() use to jump to
  Kernel.

  @param  pContext      A pointer to point a contant in BootLoader_t structure.

  @return An address that loader should to jump to booting image.

**/
uint32_t
OEMBootRun(
    void *pContext
    )
{
    BOOT_ARGS                               *pBootArgs;
    EFI_STATUS                              Status;
    UINTN                                   uSizeOfInfo;
    enum_t                                  uMode, uMaxMode;
    BootLoader_t                            *pLoader = pContext;
    BOOL                                    bFindMpde = FALSE;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION    *pInfo;

    DBGMSG(DBGZONE_INFO, (L"OEMBootRun()+\n"));

    pBootArgs = (BOOT_ARGS *)IMAGE_SHARE_BOOT_ARGS_PA;

    if (pLoader->runAddress == 0)
        pLoader->runAddress = pBootArgs->dwLaunchAddr;

    // Debug serial port
    if(pLoader->comPort > 4)
        pLoader->comPort = 1;

    pBootArgs->ucComPort = pLoader->comPort;
    pBootArgs->ucBaudDivisor = pLoader->baudDivisor;

       // PCI bus
    pBootArgs->ucPCIConfigType = pLoader->pciBusMode;

    if (pLoader->formatUserStore)
        pBootArgs->ucLoaderFlags |= LDRFL_FORMAT_STORE;
    else
        pBootArgs->ucLoaderFlags &= ~LDRFL_FORMAT_STORE;

    // KITL
    if ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_DHCP) == 0) {
        pBootArgs->EdbgFlags |= 0x0001;
    }
    pBootArgs->EdbgAddr.dwIP = pLoader->ipAddress;

    if ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_ENABLED) == 0) {
        pBootArgs->KitlTransport = KTS_NONE;
    }
    else {
        // Save the download media type
        pBootArgs->KitlTransport = (WORD)pLoader->KitlTransportMediaType;

        if ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_PASSIVE) != 0) {
            pBootArgs->KitlTransport |= KTS_PASSIVE_MODE;
        }
    }

    if ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_VMINI) == 0) {
        pBootArgs->ucLoaderFlags |= LDRFL_KITL_DISABLE_VMINI;
    }
    else {
        pBootArgs->ucLoaderFlags &= ~LDRFL_KITL_DISABLE_VMINI;
    }

//******************************************************************************
// Because Pci function not acquire from EFI yet, so comment below code
 #if 0
    switch (pLoader->kitlDevice.ifc)
        {
        case IfcTypePci:
            {
            BootPciLocation_t pciLoc;

            pciLoc.logicalLoc = pLoader->kitlDevice.location;
            pBootArgs->dwEdbgBaseAddr = BootPciGetMbar(
                pciLoc, 0
                ) & 0xFFFFFFFC; // mask out the least two significant bits
            pBootArgs->ucEdbgIRQ = 0;
            }
            break;
        }
#endif
//******************************************************************************

    if ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_POLL) != 0) {
        pBootArgs->ucEdbgIRQ = BOOTARG_IRQ_INVALID;
    }

    // Reboot & jump addresses
    pBootArgs->dwEBootAddr = 0;
    pBootArgs->dwEBootFlag = BOOTARG_SIG;
    pBootArgs->dwLaunchAddr = (DWORD)pLoader->runAddress;

    pBootArgs->RamTop.QuadPart = pLoader->ramestimated ?pLoader->ramTop : 0;
    INFOMSG(TRUE, (L"pBootArgs->RamTop.QuadPart = 0x%08x!\r\n",
        pBootArgs->RamTop.QuadPart));
    
    // Sets fallback video mode (320x200x8) if we can not get video mode match 
    // or closed to Display setting in Boot Argment, this codes reintialize the 
    // Display settitng of Boot Argment to minima available resolution for successfully boot up.
    if(!g_GraphicsOutput)
    {
        pBootArgs->cxDisplayScreen     = 320;
        pBootArgs->cyDisplayScreen     = 200;
        pBootArgs->cxPhysicalScreen    = 320;
        pBootArgs->cyPhysicalScreen    = 200;
        pBootArgs->bppScreen           = 8;
        pBootArgs->cbScanLineLength    = 320;
        pBootArgs->pvFlatFrameBuffer   = 0x800A0000;
    }
    else
    {
        pInfo = g_GraphicsOutput->Mode->Info;
        
    pBootArgs->pvFlatFrameBuffer = (DWORD)g_GraphicsOutput->Mode->FrameBufferBase;

    uMaxMode = g_GraphicsOutput->Mode->MaxMode;

    uMode = 0;
    while (uMode < uMaxMode) {
        Status = g_GraphicsOutput->QueryMode(
                                            g_GraphicsOutput,
                                            uMode,
                                            &uSizeOfInfo,
                                            &pInfo
                                            );

        if (EFI_ERROR(Status)) {
            WARNMSG(TRUE, (L"Cannot get mode info (status=0x%x)\n", Status));
            break;
        }
        if((pInfo->HorizontalResolution == pLoader->displayWidth) &&
           (pInfo->VerticalResolution == pLoader->displayHeight)) {
                pBootArgs->vesaMode = (WORD)uMode;
                pBootArgs->cxDisplayScreen = (WORD)pInfo->HorizontalResolution;
                pBootArgs->cyDisplayScreen = (WORD)pInfo->VerticalResolution;
                pBootArgs->cbScanLineLength = (WORD)pInfo->PixelsPerScanLine;
                pBootArgs->RedMaskSize = (UCHAR)pInfo->PixelInformation.RedMask;
                pBootArgs->GreenMaskSize = (UCHAR)pInfo->PixelInformation.GreenMask;
                pBootArgs->BlueMaskSize = (UCHAR)pInfo->PixelInformation.BlueMask;

                if ((pInfo->PixelFormat == PixelRedGreenBlueReserved8BitPerColor) ||
                    (pInfo->PixelFormat == PixelBlueGreenRedReserved8BitPerColor)) {
                        pBootArgs->RedMaskPosition = (UCHAR)16;
                        pBootArgs->GreenMaskPosition = (UCHAR)8;
                        pBootArgs->BlueMaskPosition = (UCHAR)0;
                }
                pBootArgs->bppScreen = (WORD)pLoader->displayBpp;
                bFindMpde = TRUE;
                break;
        }
        uMode++;
    }

    if(bFindMpde) {
        Status = g_GraphicsOutput->SetMode(
                                          g_GraphicsOutput,
                                          pBootArgs->vesaMode
                                          );
        if (EFI_ERROR(Status))
            WARNMSG(TRUE, (L"SeOEMBootRun():Cannot Set VESA mode (status=0x%x)\n", Status));
     }

    if(pLoader->displayLogicalWidth >= pLoader->displayWidth)
        pBootArgs->cxPhysicalScreen = (WORD)pLoader->displayLogicalWidth;
    if(pLoader->displayLogicalHeight >= pLoader->displayHeight)
        pBootArgs->cyPhysicalScreen = (WORD)pLoader->displayLogicalHeight;
        
    }

    DBGMSG(DBGZONE_INFO, (L"OEMBootRun()-, dwLaunchAddr 0x%X\n", pLoader->runAddress));

    return pLoader->runAddress;
}

//------------------------------------------------------------------------------

