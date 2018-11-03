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
#ifndef __BOOT_TERMINAL_UTILS_H
#define __BOOT_TERMINAL_UTILS_H

#include "include/boot.h"
#include "include/bootdebug.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

typedef void (*BootTerminalMenuPFnAction_t)(
    void *pContext,
    void *pActionContext
    );

typedef struct BootTerminalMenuEntry_t {
    wchar_t const key;
    wstring_t text;
    BootTerminalMenuPFnAction_t pfnAction;
    void *pActionContext;
} BootTerminalMenuEntry_t;

#pragma warning(push)
#pragma warning(disable:4200) //nonstandard extension used : zero-sized array in struct/union
typedef struct BootTerminalMenu_t {
    wstring_t const title;
    BootTerminalMenuEntry_t entries[];
} BootTerminalMenu_t;
#pragma warning(pop)

void
BootTerminalMenu(
    void *pContext,
    void *pActionContext
    );

//------------------------------------------------------------------------------

size_t
BootTerminalReadLine(
    wchar_t *pBuffer,
    size_t bufferChars
    );

bool_t
BootTerminalReadIp4(
    uint32_t *pIp4,
    wcstring_t prompt
    );

void
BootTerminalReadEnable(
    bool_t *pEnable,
    wcstring_t prompt
    );

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_TERMINAL_UTILS_H
