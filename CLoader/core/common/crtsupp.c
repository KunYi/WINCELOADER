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
#include <bootCore.h>
#include <coreCrtStorage.h>

//------------------------------------------------------------------------------

#define _CVTBUFSIZE             (309 + 40)

//------------------------------------------------------------------------------

void
__cdecl
__crt_unrecoverable_error(
    const wchar_t *pszExpression,
    const wchar_t *pszFunction,
    const wchar_t *pszFile,
    unsigned int nLine,
    uintptr_t pReserved
    )
{
    UNREFERENCED_PARAMETER(pszExpression);
    UNREFERENCED_PARAMETER(pszFunction);
    UNREFERENCED_PARAMETER(pszFile);
    UNREFERENCED_PARAMETER(nLine);
    UNREFERENCED_PARAMETER(pReserved);
    OEMBootPowerOff(NULL);
}

//------------------------------------------------------------------------------
//
//  Return a pointer to a buffer of size _CVTBUFSIZE for use by _fcvt and _ecvt.
//
char*
__crt_get_storage__fcvt(
    )
{
    static char data[_CVTBUFSIZE];
    return data;
}

//------------------------------------------------------------------------------
//
//  Return a pointer to a long for use by rand and srand.
//
long*
__crt_get_storage_rand(
    )
{
    static long data;
    return &data;
}

//------------------------------------------------------------------------------
//
//  Return a pointer to a char* for use by strtok.
//
char**
__crt_get_storage_strtok(
    )
{
    static char* dataPtr;
    return &dataPtr;
}

//------------------------------------------------------------------------------
//
//  Return a pointer to a wchar_t* for use by wcstok
//
wchar_t**
__crt_get_storage_wcstok(
    )
{
    static wchar_t* dataPtr;
    return &dataPtr;
}

//------------------------------------------------------------------------------

