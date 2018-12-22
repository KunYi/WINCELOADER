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
#ifndef __BOOT_STRING_H
#define __BOOT_STRING_H

#include <bootTypes.h>

//------------------------------------------------------------------------------

__inline
size_t
BootStringCchCopyA(
    __out_ecount(cchDest) string_t dest,
    __in size_t cchDest,
    __in cstring_t src
    )
{
    size_t cch = 0;
    if (cchDest == 0) goto cleanUp;

    while ((cchDest > 0) && (*src != '\0'))
        {
        *dest++ = *src++;
        cchDest--;
        cch++;
        }
    if (cchDest == 0) dest--;
    *dest = '\0';

cleanUp:
    return cch;
}

//------------------------------------------------------------------------------

__inline
size_t
BootStringCchCopy(
    __out_ecount(cchDest) wstring_t dest,
    __in size_t cchDest,
    __in wcstring_t src
    )
{
    size_t cch = 0;

    if (cchDest == 0) goto cleanUp;
    while ((cchDest > 0) && (*src != L'\0'))
        {
        *dest++ = *src++;
        cchDest--;
        cch++;
        }
    if (cchDest == 0) dest--;
    *dest = L'\0';

cleanUp:
    return cch;
}

//------------------------------------------------------------------------------

__inline
size_t
BootStringCchCatA(
    __inout_ecount(cchDest) string_t dest,
    __in size_t cchDest,
    __in cstring_t src
    )
{
    size_t cch = 0;

    if (cchDest == 0) goto cleanUp;
    while ((cchDest > 0) && (*dest != '\0'))
        {
        dest++;
        cchDest--;
        cch++;
        }
    while ((cchDest > 0) && (*src != '\0'))
        {
        *dest++ = *src++;
        cchDest--;
        cch++;
        }
    if (cchDest == 0) dest--;
    *dest = '\0';
   
cleanUp:
    return cch;
}

//------------------------------------------------------------------------------

__inline
size_t
BootStringUtf8toUnicode(
    __inout_ecount(cchDest) wstring_t dest,
    __in size_t cchDest,
    __in cstring_t src
    )
{
    size_t cch = 0;

    if (cchDest == 0) goto cleanUp;
    while ((cchDest > 0) && (*src != '\0'))
        {
        *dest++ = *src++;
        cchDest--;
        cch++;
        }
    if (cchDest == 0) dest--;
    *dest = L'\0';

cleanUp:
    return cch;
}

//------------------------------------------------------------------------------

__inline
size_t
BootStringUnicodetoUtf8(
    __inout_ecount(cchDest) string_t dest,
    __in size_t cchDest,
    __in wcstring_t src
    )
{
    size_t cch = 0;

    if (cchDest == 0) goto cleanUp;
    while ((cchDest > 0) && (*src != L'\0'))
        {
        *dest++ = (char)(0xff & *src++);
        cchDest--;
        cch++;
        }
    if (cchDest == 0) dest--;
    *dest = '\0';

cleanUp:
    return cch;
}

//------------------------------------------------------------------------------

__inline
bool_t
BootStringEqual(
    __in wstring_t strA,
    __in wstring_t strB
    )
{
    bool_t rc = true;

    if (strA == strB) goto cleanUp;
    
    rc = (strA != NULL) && (strB != NULL);
    while (rc)
        {
        rc = (*strA == *strB);
        if (*strA == L'\0' || *strB == L'\0') break;
        strA++;
        strB++;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

#endif
