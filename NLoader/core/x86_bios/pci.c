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
#include <bootMemory.h>

//------------------------------------------------------------------------------

void*
BootPciMbarToVA(
    BootPciLocation_t pciLoc,
    enum_t index,
    bool_t cached
    )
{
    void *pAddress = NULL;
    uint32_t mbar;

    // Read MBAR
    if (!BootPciConfigRead(
            pciLoc, (uint8_t)(0x10 + (index << 2)), &mbar, sizeof(mbar)
            )) goto cleanUp;
    if (mbar == 0xFFFFFFFF) goto cleanUp;

    // Map MBAR to CPU address space
    if ((mbar & 0x1) != 0)
        {
        pAddress = (void*)(mbar & ~0x1);
        }
    else
        {
        pAddress = BootPAtoVA(mbar, cached);
        }
    
cleanUp:    
    return pAddress;
}

//------------------------------------------------------------------------------

