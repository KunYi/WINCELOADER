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
#include <bootBios.h>

//------------------------------------------------------------------------------
//  Table used to call BIOS proxy (see assembly code)
//
extern 
uint32_t
BiosCallTable[];

//------------------------------------------------------------------------------

typedef
void
(*PFN_BIOS_INT)(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    );

//------------------------------------------------------------------------------

static
bool_t
s_biosInitDone = false;

//------------------------------------------------------------------------------

static
void
BiosInit(
    )
{
    // Copy BIOS call code to address space bellow 64K
    memcpy((void*)BiosCallTable[0], BiosCallTable, BiosCallTable[2]);
    s_biosInitDone = true;
}

//------------------------------------------------------------------------------

void*
BootBiosBuffer(
    )
{
    if (!s_biosInitDone) BiosInit();
    return (void*)BiosCallTable[1];
}

//------------------------------------------------------------------------------

void
BootBiosInt10(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    )
{
    if (!s_biosInitDone) BiosInit();
    ((PFN_BIOS_INT)BiosCallTable[3])(pEAX, pEBX, pECX, pEDX, pESI, pEDI);
}

//------------------------------------------------------------------------------

void
BootBiosInt13(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    )
{
    if (!s_biosInitDone) BiosInit();
    ((PFN_BIOS_INT)BiosCallTable[4])(pEAX, pEBX, pECX, pEDX, pESI, pEDI);
}

//------------------------------------------------------------------------------

void
BootBiosInt14(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    )
{
    if (!s_biosInitDone) BiosInit();
    ((PFN_BIOS_INT)BiosCallTable[5])(pEAX, pEBX, pECX, pEDX, pESI, pEDI);
}

//------------------------------------------------------------------------------

void
BootBiosInt15(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    )
{
    if (!s_biosInitDone) BiosInit();
    ((PFN_BIOS_INT)BiosCallTable[6])(pEAX, pEBX, pECX, pEDX, pESI, pEDI);
}

//------------------------------------------------------------------------------

void
BootBiosInt16(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    )
{
    if (!s_biosInitDone) BiosInit();
    ((PFN_BIOS_INT)BiosCallTable[7])(pEAX, pEBX, pECX, pEDX, pESI, pEDI);
}

//------------------------------------------------------------------------------

void
BootBiosInt1A(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    )
{
    if (!s_biosInitDone) BiosInit();
    ((PFN_BIOS_INT)BiosCallTable[8])(pEAX, pEBX, pECX, pEDX, pESI, pEDI);
}

//------------------------------------------------------------------------------

