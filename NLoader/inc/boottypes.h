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
#ifndef __BOOT_TYPES_H
#define __BOOT_TYPES_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define: dimof
//
#define dimof(x)                    _countof((x))

//------------------------------------------------------------------------------
//
//  Type: uintX_t/intX_t
//
//  Type which has 8/16/32 bits on target platform.
//
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;

typedef __nullterminated char *string_t;
typedef __nullterminated wchar_t *wstring_t;
typedef __nullterminated const char *cstring_t;
typedef __nullterminated const wchar_t *wcstring_t;

//------------------------------------------------------------------------------
//
//  Type: size_t
//
//  This type is used for variables which express size of some entity. In
//  future it can be extended from 32-bit to 64-bit unsigned integer.
//
typedef uint32_t size_t;

//------------------------------------------------------------------------------
//
//  Type: enum_t
//
//  Type used for variables which can store enumeration value. In difference
//  to size_t it is unlikely it will use 64-bit unsigned integer on 32-bit
//  platforms.
//
typedef uint32_t enum_t;

//------------------------------------------------------------------------------
//
//  Type: flags_t
//
//  Type used for flags parameters. Currently 32-bit unsigned integer
//  value is used. Using custom type should point out specific use pattern
//  and allow extend size if necessary.
//
typedef uint32_t flags_t;

//------------------------------------------------------------------------------
//
//  Type: handle_t
//
//  Type used for boot driver handlers. 
//
typedef void *handle_t;

//------------------------------------------------------------------------------
//
//  Type: bool_t
//
//  This type is used to make code consistent between C and C++.
//
#ifndef __cplusplus
#define false   (1 != 1)
#define true    (1 == 1)
typedef long bool_t;
#else
typedef bool bool_t;
#endif

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_TYPES_H
