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
#include <wtypes.h>
#include <oal_pci.h>
#include <bootPci.h>

//------------------------------------------------------------------------------

LPVOID NKCreateStaticMapping(DWORD dwPhysBase, DWORD dwSize)
{
    UNREFERENCED_PARAMETER(dwSize);
    return (LPVOID)dwPhysBase;
}

//------------------------------------------------------------------------------

UINT32
OALPCICfgRead(
   UINT32 busId, 
   OAL_PCI_LOCATION pciLoc, 
   UINT32 offset, 
   UINT32 size,
   __out_bcount(size) VOID *pData
)
{
    BootPciLocation_t bootPciLoc;
    UNREFERENCED_PARAMETER(busId);
    bootPciLoc.logicalLoc = pciLoc.logicalLoc;
    return BootPciConfigRead(bootPciLoc, (uint8_t)offset, pData, size);
}