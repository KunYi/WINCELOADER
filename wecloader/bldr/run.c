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
#include <bootTransportEdbg.h>
#include <bootDisplayBios.h>
#include <bootPci.h>

#undef ZONE_ERROR
#include <bootArg.h>
#include <ceddk.h>

#include <pehdr.h>
#include <romldr.h>

//------------------------------------------------------------------------------

#define IMAGE_SHARE_BOOT_ARGS_PA            0x001FF000
#define IMAGE_SHARE_BOOT_ARGS_PTR_PA        0x001FFFFC

//------------------------------------------------------------------------------

void
BootStart(
    );

//------------------------------------------------------------------------------

#if 0
void
DumpTOC(
    uint32_t runAddress
    )
{
    uint32_t *pInfo;  
    ROMHDR *pTOC;

    pInfo = BootPAtoUA(runAddress + ROM_SIGNATURE_OFFSET);
    if (*pInfo != ROM_SIGNATURE)
        {
        BootLog(L"\r\nNo image signature at 0x%08x!\r\n", pInfo);
        goto cleanUp;
        }
    pTOC = BootPAtoUA(runAddress + pInfo[2]);

    // Print out ROMHDR information
    BootLog(L"\r\n");
    BootLog(L"ROMHDR (pTOC = 0x%08x) ---------------------\r\n", pTOC);
    BootLog(L"  DLL First           : 0x%08x\r\n", pTOC->dllfirst);
    BootLog(L"  DLL Last            : 0x%08x\r\n", pTOC->dlllast);
    BootLog(L"  Physical First      : 0x%08x\r\n", pTOC->physfirst);
    BootLog(L"  Physical Last       : 0x%08x\r\n", pTOC->physlast);
    BootLog(L"  Num Modules         : %10d\r\n",   pTOC->nummods);
    BootLog(L"  RAM Start           : 0x%08x\r\n", pTOC->ulRAMStart);
    BootLog(L"  RAM Free            : 0x%08x\r\n", pTOC->ulRAMFree);
    BootLog(L"  RAM End             : 0x%08x\r\n", pTOC->ulRAMEnd);
    BootLog(L"  Num Copy Entries    : %10d\r\n",   pTOC->ulCopyEntries);
    BootLog(L"  Copy Entries Offset : 0x%08x\r\n", pTOC->ulCopyOffset);
    BootLog(L"  Prof Symbol Length  : 0x%08x\r\n", pTOC->ulProfileLen);
    BootLog(L"  Prof Symbol Offset  : 0x%08x\r\n", pTOC->ulProfileOffset);
    BootLog(L"  Num Files           : %10d\r\n",   pTOC->numfiles);
    BootLog(L"  Kernel Flags        : 0x%08x\r\n", pTOC->ulKernelFlags);
    BootLog(L"  FileSys RAM Percent : 0x%08x\r\n", pTOC->ulFSRamPercent);
    BootLog(L"  Driver Glob Start   : 0x%08x\r\n", pTOC->ulDrivglobStart);
    BootLog(L"  Driver Glob Length  : 0x%08x\r\n", pTOC->ulDrivglobLen);
    BootLog(L"  CPU                 :     0x%04x\r\n", pTOC->usCPUType);
    BootLog(L"  MiscFlags           :     0x%04x\r\n", pTOC->usMiscFlags);
    BootLog(L"  Extensions          : 0x%08x\r\n", pTOC->pExtensions);
    BootLog(L"  Tracking Mem Start  : 0x%08x\r\n", pTOC->ulTrackingStart);
    BootLog(L"  Tracking Mem Length : 0x%08x\r\n", pTOC->ulTrackingLen);
    BootLog(L"------------------------------------------------\r\n");
    BootLog(L"\r\n");

cleanUp:    
    return;
}
#endif

//------------------------------------------------------------------------------
//
//  In OEMBootRun we need to prepare information passed to Windows CE CEPC
//  OAL in way it will understand. So until we modify CEPC OAL we have to
//  use little complicated and inconsistent way how to pass required
//  information.
//
uint32_t
OEMBootRun(
    void *pContext
    )
{
    BootLoader_t *pLoader = pContext;
    BOOT_ARGS *pBootArgs = (BOOT_ARGS *)IMAGE_SHARE_BOOT_ARGS_PA;
    handle_t hDisplay;

    if (pLoader->runAddress == 0)
        {
        pLoader->runAddress = pBootArgs->dwLaunchAddr;
        }

#if 0
    DumpTOC(pLoader->runAddress);
#endif
    // Debug serial port
    pBootArgs->ucComPort = pLoader->comPort;
    pBootArgs->ucBaudDivisor = pLoader->baudDivisor;

    // PCI bus
    pBootArgs->ucPCIConfigType = pLoader->pciBusMode;

    if (pLoader->formatUserStore)
        pBootArgs->ucLoaderFlags |= LDRFL_FORMAT_STORE;
    else
        pBootArgs->ucLoaderFlags &= ~LDRFL_FORMAT_STORE;


    // KITL
    if ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_DHCP) == 0)
        {
        pBootArgs->EdbgFlags |= 0x0001;
        }
    pBootArgs->EdbgAddr.dwIP = pLoader->ipAddress;

    if ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_ENABLED) == 0)
        {
        pBootArgs->KitlTransport = KTS_NONE;
        }
    else 
        {
        // Save the download media type
        pBootArgs->KitlTransport = (WORD)pLoader->KitlTransportMediaType; 

        if ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_PASSIVE) != 0)
            {
            pBootArgs->KitlTransport |= KTS_PASSIVE_MODE;
            }
        }

        if ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_VMINI) == 0)
            {
            pBootArgs->ucLoaderFlags |= LDRFL_KITL_DISABLE_VMINI;
            }
        else
            {
            pBootArgs->ucLoaderFlags &= ~LDRFL_KITL_DISABLE_VMINI;
            }
    
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

    if ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_POLL) != 0)
        {
        pBootArgs->ucEdbgIRQ = BOOTARG_IRQ_INVALID;
        }

    // Reboot & jump addresses
    pBootArgs->dwEBootAddr = (DWORD)BootStart;
    pBootArgs->dwEBootFlag = BOOTARG_SIG;
    pBootArgs->dwLaunchAddr = (DWORD)pLoader->runAddress;
    pBootArgs->RamTop.QuadPart = (pLoader->ramestimated?0:pLoader->ramTop);
    
    // Display
    hDisplay = OEMBootCreateDriver(pContext, BOOT_DRIVER_CLASS_DISPLAY, 0);
    if (hDisplay != NULL)
        {
        uint32_t width, height, bpp, vesaMode, phFrame, stride;
        uint32_t redSize, redPos, greenSize, greenPos, blueSize, bluePos;
        enum_t mode;

        mode = (enum_t)-1;
        width = pLoader->displayWidth;
        height = pLoader->displayHeight;
        bpp = pLoader->displayBpp;
        BootDisplayModeQueryFull(
            hDisplay, &mode, &width, &height, &bpp, &vesaMode,
            &phFrame, &stride, &redSize, &redPos, &greenSize, &greenPos,
            &blueSize, &bluePos
            );

        pBootArgs->pvFlatFrameBuffer = phFrame;
        pBootArgs->vesaMode = (WORD)vesaMode;
        pBootArgs->cxDisplayScreen = (WORD)width;
        pBootArgs->cyDisplayScreen = (WORD)height;
        pBootArgs->cxPhysicalScreen = (WORD)width;
        pBootArgs->cyPhysicalScreen = (WORD)height;
        pBootArgs->cbScanLineLength = (WORD)stride;
        pBootArgs->bppScreen = (WORD)bpp;
        pBootArgs->RedMaskSize = (UCHAR)redSize;
        pBootArgs->RedMaskPosition = (UCHAR)redPos;
        pBootArgs->GreenMaskSize = (UCHAR)greenSize;
        pBootArgs->GreenMaskPosition = (UCHAR)greenPos;
        pBootArgs->BlueMaskSize = (UCHAR)blueSize;
        pBootArgs->BlueMaskPosition = (UCHAR)bluePos;
        pBootArgs->VideoRam.QuadPart = pLoader->videoRam;
        
        if (pLoader->displayLogicalWidth == 0)
            {
            pBootArgs->cxDisplayScreen = (WORD)width;
            pBootArgs->cyDisplayScreen = (WORD)height;
            }
        else
            {
            pBootArgs->cxDisplayScreen = (WORD)pLoader->displayLogicalWidth;
            pBootArgs->cyDisplayScreen = (WORD)pLoader->displayLogicalHeight;
            }
        BootDisplayModeSet(hDisplay, mode);

        BootDisplayDeinit(hDisplay);
        }

    return pLoader->runAddress;
}

//------------------------------------------------------------------------------

