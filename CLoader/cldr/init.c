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
#include <bootBios.h>
#include <bootBlockBios.h>
#include <bootDisplayBios.h>
#include <pehdr.h>
#include <romldr.h>

#include <DisplayAPI.h>
#include <FileSysAPI.h>

#undef ZONE_ERROR
#include <bootArg.h>

//------------------------------------------------------------------------------

extern
ROMHDR* 
volatile
const
pTOC;

static
const 
uint32_t 
s_test[] = { 
    0x55aaaa55, 0xaa5555aa, 12345678, 0xf00f4466 
};

static
BootLoader_t
s_bootLoader;

#pragma pack(push, 1)
typedef struct {
    USHORT Size;
    PVOID Base;
} FWORDPtr;
#pragma pack(pop)


static const DWORD* pIVT = (DWORD*)0x001FEF00; // see also bldr.bib
static const DWORD MAX_INTR = 0x1b; // maximum interrupts we will save
static const BYTE UNSUPPORTED_FUNCTION = 0x86;

extern BOOT_ARGS *pBootArgs;

//------------------------------------------------------------------------------
//
//  Function:  OEMBootInit - Does initialization stuff common to all CEPCs
//
void* OEMBootInit(  )
{
    BootLoader_t *pLoader = &s_bootLoader;
    BootDisplayBiosVbeInfoBlock_t *pInfo;
    uint32_t eax, ebx, ecx, edx, esi, edi;
    FWORDPtr IVTPtr;
#pragma pack(push, 1)
    typedef struct e820_t {
        ULARGE_INTEGER addr;
        ULARGE_INTEGER size;
        uint32_t type; 
    } e820_t;
#pragma pack(pop)
    enum { usable = 1 };
    e820_t * e820buffer;
	

	//Get the COM port from the Bootargs
	pLoader->comPort = pBootArgs->ucComPort;
    InitSerialEcho(pLoader->comPort);

    // Find if there is PCI BIOS extension
    eax = 0xB101;
    edi = 0x0000;

    _asm { sidt [IVTPtr] };
    if (IVTPtr.Base == 0) {
        // IVT was setup by the BIOS, save it
        memcpy((DWORD*)pIVT, IVTPtr.Base, MAX_INTR*sizeof(DWORD));

		DEBUGMSG(ZONE_INFO, (L"OEMBootInit:: IVT saved by BIOS.\r\n"));
	
	} else {
        // IVT was overwritten by the kernel, restore the saved copy
        IVTPtr.Size = 0xffff;
        IVTPtr.Base = 0;
        memcpy(IVTPtr.Base, pIVT, MAX_INTR*sizeof(DWORD));
        _asm { lidt [IVTPtr] };

		DEBUGMSG(ZONE_INFO, (L"OEMBootInit:: IVT restored.\r\n"));
	
    }

    BootBiosInt1A(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if (((eax & 0xFF00) == 0) && (edx == ' ICP'))
        {
        pLoader->pciBiosMajor = (uint8_t)((ebx & 0xFF00) >> 8);
        pLoader->pciBiosMinor = (uint8_t)((ebx & 0x00FF) >> 0);
        pLoader->pciBusMode   = (uint8_t)((eax & 0x00FF) >> 0);
        pLoader->pciLastBus   = (uint8_t)((ecx & 0x00FF) >> 0);

		DEBUGMSG(ZONE_INFO, (L"OEMBootInit:: PCI BIOS extension found.\r\n"));

        }
    else
        {
        pLoader->pciBiosMajor = 0;
        pLoader->pciBiosMinor = 0;
        pLoader->pciBusMode   = 0;
        pLoader->pciLastBus   = 0;

		DEBUGMSG(ZONE_INFO, (L"OEMBootInit:: NO PCI BIOS extension.\r\n"));

        }

    // Find if there is BIOS Enhanced Disk Drive Services support
    eax = 0x4100;
    ebx = 0x55AA;
    edx = pLoader->driveId;
    BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if (ebx == 0xAA55)
        {
        pLoader->eddBiosMajor = (uint8_t)((eax & 0xF000) >> 12);
        pLoader->eddBiosMinor = (uint8_t)((eax & 0x0F00) >> 8);
        pLoader->eddBiosIfcs  = (uint16_t)ecx;

		DEBUGMSG(ZONE_INFO, (L"OEMBootInit:: BIOS Enhanced Disk Drive services support found.\r\n"));

        }
    else
        {
        pLoader->eddBiosMajor = 0;
        pLoader->eddBiosMinor = 0;
        pLoader->eddBiosIfcs  = 0;

		DEBUGMSG(ZONE_INFO, (L"OEMBootInit:: NO BIOS Enhanced Disk Drive services support.\r\n"));

        }

    // Find if there is APM support
    eax = 0x5300;
    ebx = 0x0000;
    BootBiosInt15(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if ((ebx & 0xFFFF) == 0x504D)
        {
        pLoader->apmBiosMajor = (uint8_t)((eax & 0xFF00) >> 8);
        pLoader->apmBiosMinor = (uint8_t)((eax & 0x00FF) >> 0);
        pLoader->apmBiosFlags = (uint8_t)((ecx & 0x00FF) >> 0);

		DEBUGMSG(ZONE_INFO, (L"OEMBootInit:: APM support found.\r\n"));

        }
    else
        {
        pLoader->apmBiosMajor = 0;
        pLoader->apmBiosMinor = 0;
        pLoader->apmBiosFlags = 0;

		DEBUGMSG(ZONE_INFO, (L"OEMBootInit:: NO APM support.\r\n"));

        }

    // Find if there is VBE support (VESA)
    pInfo = BootBiosBuffer();
    memset(pInfo, 0, sizeof(*pInfo));
    pInfo->signature = '2EBV';
    eax = 0x4F00;
    edi = (uint32_t)pInfo;
    BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if ((eax & 0xFFFF) == 0x004F)
        {
        pLoader->vbeBiosMajor = (pInfo->version >> 8);
        pLoader->vbeBiosMinor = (pInfo->version & 0xFF);
        pLoader->videoRam = pInfo->totalMemory * 64 * 1024; // convert from 64k blocks

		DEBUGMSG(ZONE_INFO, (L"OEMBootInit:: VBE (VISA) support found.\r\n"));

        }
    else
        {
        pLoader->vbeBiosMajor = 0;
        pLoader->vbeBiosMinor = 0;

		DEBUGMSG(ZONE_INFO, (L"OEMBootInit:: NO VBE (VISA) support.\r\n"));

        }

    e820buffer = BootBiosBuffer();
    ebx = 0x0000;
    pLoader->ramTop = 0;

    // Use int15/e820 to detect ram size
    do 
        {
        eax = 0xe820;
        ecx = sizeof(e820_t);
        edx = 'SMAP';
        edi = (uint32_t)e820buffer;
        memset(e820buffer, 0, sizeof(e820_t));
        BootBiosInt15(&eax, &ebx, &ecx, &edx, &esi, &edi);

        // we should verify the carry flag isn't set - indicating an error

        if (ecx != sizeof(e820_t))
            break;

        if (e820buffer->type == usable && e820buffer->addr.QuadPart > pLoader->ramTop) 
            {
            pLoader->ramestimated = false;
            // just use the last usable ram section
            if (e820buffer->addr.QuadPart + e820buffer->size.QuadPart > 0xffffffff)
                pLoader->ramTop = 0xffffffff; // 4096MB Max
            else
                pLoader->ramTop = e820buffer->addr.LowPart + e820buffer->size.LowPart;
            }
        } 
    while (ebx);

    // Use int15/e801 to detect ram size
    if (pLoader->ramTop == 0) {
        eax = 0xe801;
        ecx = edx = ebx = 0;
        BootBiosInt15(&eax, &ebx, &ecx, &edx, &esi, &edi);
        if (HIBYTE(eax) != UNSUPPORTED_FUNCTION) {
            // we assume 16MB+ RAM
            if (eax && eax <= 0x3c00 && ebx)
                pLoader->ramTop = ebx * 64*1024 + 0x3c00 * 1024;
            else if (ecx && ecx <= 0x3c00 && edx)
                pLoader->ramTop = edx * 64*1024 + 0x3c00 * 1024;
            pLoader->ramestimated = false;
        }
    }

    // Use memcpy/memcmp to try to detect where ram stops
    if (pLoader->ramTop == 0) {
        // Find top of memory
        eax = pTOC->ulRAMEnd + 0x00100000;
        while (eax < (DWORD)-1)
            {
            uint32_t buffer[4];
            memcpy(buffer, (void*)(eax - sizeof(buffer)), sizeof(buffer));
            memcpy((void*)(eax - sizeof(buffer)), s_test, sizeof(buffer));
            if (memcmp((void*)(eax - sizeof(buffer)), s_test, sizeof(buffer)) != 0)
                {
                break;
                }
            memcpy((void*)(eax - sizeof(buffer)), buffer, sizeof(buffer));
            if (eax + 0x00100000 > eax)
                eax += 0x00100000;
            else
                eax = (DWORD)-1;
            }

        pLoader->ramTop = eax - 0x00100000;
        pLoader->ramestimated = true;
    }

	//
	// Initialize file system so we can read files.
	//
	InitializeFileSys ();

    return pLoader;
}

//------------------------------------------------------------------------------

void
OEMBootStall(
    uint32_t delay
    )
{
    uint32_t eax, ebx, ecx, edx, esi, edi; 

    eax = 0x8600;
    ecx = (delay >> 16) & 0xFFFF;
    edx = (delay >>  0) & 0xFFFF;
    BootBiosInt15(&eax, &ebx, &ecx, &edx, &esi, &edi);
}


#if 0

//------------------------------------------------------------------------------
//
//  Using BIOS to get time is probably more portable method than read RTC
//  CMOS chip (and also typically more accurate). However because of slow
//  execution on VirtualPC platform we are stick back 

uint32_t
OEMBootGetTickCount(
    )
{
    uint32_t eax, ebx, ecx, edx, esi, edi;

    eax = 0;
    ecx = 0;
    edx = 0;
    BootBiosInt1A(&eax, &ebx, &ecx, &edx, &esi, &edi);
    return ((ecx << 16)|(edx & 0xFFFF)) * 55;
}

//------------------------------------------------------------------------------

#else

uint8_t
CmosRead(
    uint8_t offset 
    )
{
    // Remember, we only change the low order 5 bits in address register
    MASKPORT8((uint8_t*)0x0070, 0x7F, offset);
    return INPORT8((uint8_t*)0x0071);
}

//------------------------------------------------------------------------------

#define RTC_SECOND          0x00
#define RTC_MINUTE          0x02
#define RTC_STATUS_A        0x0A

#define DECODE_BCD(b)       ((b >> 4) * 10 + (b & 0xF))

uint32_t
OEMBootGetTickCount(
    )
{
    static uint32_t tick = 0;
    static uint16_t prevOffset = 0;
    uint8_t sec, min;
    uint16_t offset;

    do
        {
        while ((CmosRead(RTC_STATUS_A) & 0x80) != 0);
        sec = CmosRead(RTC_SECOND);
        min = CmosRead(RTC_MINUTE);
        }
    while (sec != CmosRead(RTC_SECOND));

    min = DECODE_BCD(min);
    sec = DECODE_BCD(sec);
    offset = min * 60 + sec;

    if (offset >= prevOffset)
        {
        tick += offset - prevOffset;
        }
    else
        {
        tick += 3600 + offset - prevOffset;
        }
    prevOffset = offset;
    
    return tick * 1000;
}    

//------------------------------------------------------------------------------

#endif
