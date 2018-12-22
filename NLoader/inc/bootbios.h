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
#ifndef __BOOT_BIOS_H
#define __BOOT_BIOS_H

#include <bootTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

void*
BootBiosBuffer(
    );

//------------------------------------------------------------------------------

void
BootBiosInt10(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    );

//------------------------------------------------------------------------------

void
BootBiosInt13(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    );

//------------------------------------------------------------------------------

void
BootBiosInt14(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    );

//------------------------------------------------------------------------------

void
BootBiosInt15(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    );

//------------------------------------------------------------------------------

void
BootBiosInt16(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    );

//------------------------------------------------------------------------------

void
BootBiosInt1A(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    );

//------------------------------------------------------------------------------

__inline
void*
BootBiosPtr2Void(
    uint16_t ptr[2]
    )
{
    return (void*)((ptr[1] << 4) + ptr[0]);
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_BIOS_H
