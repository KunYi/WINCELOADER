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
#ifndef __BOOT_PCI_H
#define __BOOT_PCI_H

#include <bootTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//  
enum PciClassCodes_e {
   CLASS_CODE_ETHERNET = 0x020000
};

//------------------------------------------------------------------------------
//
//  Type:  BOOT_PCI_LOCATION
//
//  This type defines the PCI Bus/Device/Function device location.
//
#pragma warning(push)
//Under Microsoft extensions (/Ze), you can specify a structure without 
//a declarator as members of another structure or union. These structures 
//generate W4 warning C4201. 
#pragma warning(disable:4201) 
typedef union BootPciLocation_t {
    struct {
        uint8_t fnc;                    // function
        uint8_t dev;                    // device
        uint8_t bus;                    // bus
        };
    uint32_t logicalLoc;
} BootPciLocation_t;
#pragma warning(pop)

//------------------------------------------------------------------------------

bool_t
BootPciConfigRead(
    BootPciLocation_t pciLoc,
    uint8_t offset,
    __inout_bcount(size) void *pData,
    size_t size
   );

//------------------------------------------------------------------------------

void*
BootPciMbarToVA(
    BootPciLocation_t pciLoc,
    enum_t index,
    bool_t cached
    );

//------------------------------------------------------------------------------

__inline
uint32_t
BootPciGetId(
    BootPciLocation_t pciLoc
    )
{
    uint32_t id;

    if (!BootPciConfigRead(pciLoc, 0, &id, sizeof(id)))
        {
        id = 0xFFFFFFFF;
        }
    return id;
}

//------------------------------------------------------------------------------

__inline
uint32_t
BootPciGetClassCodeAndRevId(
    BootPciLocation_t pciLoc
    )
{
    uint32_t id;

    if (!BootPciConfigRead(pciLoc, 8, &id, sizeof(id)))
    {
        id = 0xFFFFFFFF;
    }
    return id;
}

//------------------------------------------------------------------------------

__inline
uint32_t
BootPciGetMbar(
    BootPciLocation_t pciLoc,
    enum_t index
    )
{
    uint32_t mbar;
    
    if (!BootPciConfigRead(
            pciLoc, 0x10 + ((uint8_t)index << 2), &mbar, sizeof(mbar)
            ))
        {
        mbar = 0xFFFFFFFF;
        }
    return mbar;
}

//------------------------------------------------------------------------------

__inline
void
BootPciNextLocation(
    __inout BootPciLocation_t *pPciLoc
    )
{
    if (pPciLoc->fnc == 0)
        {
        uint8_t type;
        if (!BootPciConfigRead(*pPciLoc, 14, &type, sizeof(type)) ||
            ((type & 0x80) == 0))
            {
            pPciLoc->fnc = 7;
            }
        }
    if (++pPciLoc->fnc > 7)
        {
        pPciLoc->fnc = 0;
        if (++pPciLoc->dev > 31)
            {
            pPciLoc->dev = 0;
            pPciLoc->bus++;
            }
        }
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_PCI_H
