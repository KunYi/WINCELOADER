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
#include <bootLog.h>

//------------------------------------------------------------------------------

//Debug zone constants are defined in \boot\inc\bootlog.h

DBGPARAM
dpCurSettings = {
    L"BootLoader", {
        L"Error",   L"Warning",   L"Info",   L"Function",
        L"Init",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0000
};

//------------------------------------------------------------------------------

void
__cdecl
NKDbgPrintfW(
    LPCWSTR lpszFmt, 
    ...
    )
{
    va_list pArgList;
    wchar_t buffer[128];
    
    va_start(pArgList, lpszFmt);
    BootLogVSPrintf(buffer, dimof(buffer), lpszFmt, pArgList, TRUE);
    OEMBootLogWrite(buffer);
    va_end(pArgList);
}

//------------------------------------------------------------------------------

void
OutputDebugStringW(
    LPCWSTR pOutputString 
    )
{
    OEMBootLogWrite(pOutputString);
}

//------------------------------------------------------------------------------
