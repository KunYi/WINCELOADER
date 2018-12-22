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

#undef dimof
#define dimof(x)        (sizeof(x)/sizeof(x[0]))

//------------------------------------------------------------------------------

//(db)
//#ifdef SHIP_BUILD
//
//uint32_t
//g_bootLogMask = 0;
//
//#else
//
//uint32_t
//g_bootLogMask = (1 << ZONE_ERROR_ID);
//
//#endif

//------------------------------------------------------------------------------

static
struct {
    WCHAR buffer[0x1000];
} g_log;

//------------------------------------------------------------------------------
VOID
BootLogA(
    LPCSTR formatA,
    va_list pArgList
    )
{
    wchar_t formatW[256];
    enum_t ix;

    ix = 0;
    //Convert an char* string into wchar* string by simply copying since ASCII
    //is fully backwards compatible with Unicode
    while ((formatA[ix] != '\0') && (ix < dimof(formatW) - 1))
        {
        formatW[ix++] = formatA[ix];
        }
    formatW[ix] = L'\0';

    BootLogVSPrintf(g_log.buffer, dimof(g_log.buffer), formatW, pArgList, FALSE);
    OEMBootLogWrite(g_log.buffer);
}
//------------------------------------------------------------------------------

VOID
BootLogW(
    LPCWSTR format,
    va_list pArgList
    )
{
    BootLogVSPrintf(g_log.buffer, dimof(g_log.buffer), format, pArgList, TRUE);
    OEMBootLogWrite(g_log.buffer);
}

//------------------------------------------------------------------------------

VOID
BootLog(
    LPCWSTR format,
    ...
    )
{
    va_list pArgList;
    
    va_start(pArgList, format);
    BootLogW(format, pArgList);
}

//------------------------------------------------------------------------------

