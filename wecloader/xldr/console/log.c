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
#include "xldr.h"
#include <bootLog.h>

//------------------------------------------------------------------------------

void
BiosInt10(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    );

//------------------------------------------------------------------------------

void
OEMBootLogWrite(
    wcstring_t string
    )
{
    uint32_t eax, ebx, ecx, edx, esi, edi;

    while (string[0] != L'\0')
        {
        eax = 0x0E00 | (uint8_t)string[0];
        ebx = 0x0007;
        BiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
        string++;
        }
}

//------------------------------------------------------------------------------

void
BootLog(
    wcstring_t format,
    ...
    )
{
    va_list pArgList;
    wchar_t buffer[128];
    
    va_start(pArgList, format);
    BootLogVSPrintf(buffer, dimof(buffer), format, pArgList, TRUE);
    OEMBootLogWrite(buffer);
}

//------------------------------------------------------------------------------

