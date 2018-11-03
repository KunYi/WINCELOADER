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
//------------------------------------------------------------------------------
//
//  Header:  oal_args.h
//
//  This header file defines OAL boot arguments module interface. The module
//  is internal and it doesn't export any function or variable to kernel.
//  It is used for passing boot arguments from boot loader to HAL/kernel on
//  devices using boot loader.
//
#ifndef __OAL_ARGS_H
#define __OAL_ARGS_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define:  OAL_ARGS_xxx
//
//  This constant are used to identify/verify argument structure and its
//  version in memory.
//
#define OAL_ARGS_SIGNATURE      'SGRA'
#define OAL_ARGS_VERSION        1

//------------------------------------------------------------------------------
//
//  Type:  OAL_ARG_HEADER
//
//  This type define arguments header. It should be used at start of argument
//  structure to identify it and verify version.
//
typedef struct {
    UINT32  signature;
    UINT16  oalVersion;
    UINT16  bspVersion;
} OAL_ARGS_HEADER;

//------------------------------------------------------------------------------
//
//  Define:  OAL_ARG_QUERY_xxx
//
//  This constant are used to identify argument items in structure. Values
//  smaller than 64 are reserved for OAL library. The platform implementation
//  can use values beginning from BSP_ARGS_QUERY.
//
#define OAL_ARGS_QUERY_DEVID        1
#define OAL_ARGS_QUERY_KITL         2
#define OAL_ARGS_QUERY_UUID         3
#define OAL_ARGS_QUERY_RTC          4
#define OAL_ARGS_QUERY_UPDATEMODE   5

#define BSP_ARGS_QUERY              64

//------------------------------------------------------------------------------
//
//  Function:  OALArgsQuery
//
//  This function is called by other OAL modules to obtain value from argument
//  structure. It should return NULL when given argument type wasn't found.
//  Function should also solve issues related to different argument structure
//  versions.
//
VOID* OALArgsQuery(UINT32 type);

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif
