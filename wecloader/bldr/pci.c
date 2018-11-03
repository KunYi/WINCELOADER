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
#include <bootPci.h>
#include <bootBios.h>

//------------------------------------------------------------------------------

#pragma pack(push, 1)

typedef struct BiosPciRoutingBuffer {
    uint16_t size;
    uint16_t infoPtr[2];
} BiosPciRoutingBuffer;

typedef struct BiosPciRoutingInfo {
    uint8_t  bus;
    uint8_t  device;
    uint8_t  intALink;
    uint16_t intABitMap;
    uint8_t  intBLink;
    uint16_t intBBitMap;
    uint8_t  intCLink;
    uint16_t intCBitMap;
    uint8_t  intDLink;
    uint16_t intDBitMap;
    uint8_t  slotNumber;
    uint8_t  reserved;
} BiosPciRoutingInfo;

#pragma pack(pop)

//------------------------------------------------------------------------------

bool_t
BootPciConfigRead(
    BootPciLocation_t pciLoc,
    uint8_t offset,
    void *pData,
    size_t size
   )
{
    bool_t rc = false;
    uint32_t eax, ebx, ecx, edx, esi, edi;
    uint32_t location;


    if ((pciLoc.dev >= 32) || (pciLoc.fnc >= 8)) goto cleanUp;
    location  = ((uint32_t)pciLoc.bus << 8);
    location |= ((uint32_t)pciLoc.dev << 3) | pciLoc.fnc;
    while (size > 0)
        {
        eax = 0xB10A;
        ebx = location;
        edi = offset & ~0x03;
        BootBiosInt1A(&eax, &ebx, &ecx, &edx, &esi, &edi);
        if ((eax & 0xFF00) != 0) ecx = 0xFFFFFFFF;
        if (((offset & 0x03) != 0) || (size < 4))
            {
            uint8_t *pEcx = (uint8_t*)&ecx;
            size_t count = 4 - (offset & 0x03);
            if (size < count) count = size;
            memcpy(pData, &pEcx[offset & 0x03], count);
            pData = ((uint8_t*)pData) + count;
            offset += (uint8_t)count;
            size -= count;            
            }
        else
            {
            memcpy(pData, &ecx, sizeof(ecx));
            pData = ((uint8_t*)pData) + sizeof(ecx);
            offset += sizeof(ecx);
            size -= sizeof(ecx);            
            }
        }

    rc = true;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

