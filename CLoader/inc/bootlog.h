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
#ifndef __BOOT_LOG_H
#define __BOOT_LOG_H

#include <bootTypes.h>
#include <stdarg.h>

//(db)
#include <dbgapi.h>
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Function:  OEMBootLogWriteByte
//
void
OEMBootLogWrite(
    wcstring_t string
    );

//------------------------------------------------------------------------------
//
//  Function:  BootLog
//
void
BootLog(
    wcstring_t format,
    ...
    );

void
BootLogW(
    wcstring_t format,
    va_list pArgList
    );

void
BootLogA(
    cstring_t format,
    va_list pArgList
    );


//------------------------------------------------------------------------------

VOID
BootLogDumpMemory(
    const void *pBuffer,
    uint32_t base,
    size_t size
    );

//------------------------------------------------------------------------------
//
//  Function:  BootLogSPrintf/BootLogVSPrintf
//
//  This function formats string to buffer. It uses standard format string
//  same as wsprintf function (which is identical to printf format without
//  float point support).
//
size_t
BootLogSPrintf(
    wstring_t buffer,
    size_t maxChars,
    wcstring_t format,
    ...
    );

size_t
BootLogVSPrintf(
    wstring_t buffer,
    size_t maxChars,
    wcstring_t format,
    va_list pArgList,
    bool_t emulateWPrintf
    );

//------------------------------------------------------------------------------

#ifdef SHIP_BUILD

#define BOOTMSG(cond, exp)      ((void)FALSE)

#define BOOTLOGSET(n)           (void)
#define BOOTLOGCLR(n)           (void)

#else

extern DBGPARAM dpCurSettings;

//extern
//uint32_t
//g_bootLogMask;

//#define BOOTMSG(cond, exp)      ((void)((cond)?(BootLog exp), TRUE : FALSE))
//#define BOOTMSGZONE(n)          ((g_bootLogMask & (1 << n)) != 0)

//#define BOOTMSGSET(n)           (g_bootLogMask |= (1 << n))
//#define BOOTMSGCLR(n)           (g_bootLogMask &= ~(1 << n))

//#define ZONE_ERROR_ID           (0)
//#define ZONE_WARN_ID            (1)
//#define ZONE_INFO_ID            (2)
//#define ZONE_FUNC_ID            (3)
//
//#define ZONE_ERROR              BOOTMSGZONE(ZONE_ERROR_ID)
//#define ZONE_WARN               BOOTMSGZONE(ZONE_WARN_ID)
//#define ZONE_INFO               BOOTMSGZONE(ZONE_INFO_ID)
//#define ZONE_FUNC               BOOTMSGZONE(ZONE_FUNC_ID)


#define ZONE_ERROR                DEBUGZONE(0)
#define ZONE_WARN                 DEBUGZONE(1)
#define ZONE_INFO                 DEBUGZONE(2)
#define ZONE_FUNC                 DEBUGZONE(3)
#define ZONE_INIT                 DEBUGZONE(4)
#define ZONE_DOWNLOAD             DEBUGZONE(5)


#endif

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_LOG_H
