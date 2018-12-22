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
#ifndef __BOOT_DRIVER_H
#define __BOOT_DRIVER_H

#include <BootTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Function:  pfnBootDriverDeinit_t
//
typedef
bool_t
(*pfnBootDriverDeinit_t)(
    void *pContext
    );

//------------------------------------------------------------------------------
//
//  Function:  pfnBootDriverIoCtl_t
//
typedef
bool_t
(*pfnBootDriverIoCtl_t)(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    );

//------------------------------------------------------------------------------
//
//  Structure:  BootDriverVTable_t
//
typedef struct BootDriverVTable_t {
    pfnBootDriverDeinit_t pfnDeinit;
    pfnBootDriverIoCtl_t pfnIoCtl;
} BootDriverVTable_t;

//------------------------------------------------------------------------------
//
//  Function:  BootDriverDeinit
//
__inline
bool_t
BootDriverDeinit(
    handle_t hDriver
    )
{
    BootDriverVTable_t **pVTable = (BootDriverVTable_t **)hDriver;
    return (pVTable != NULL) ? (*pVTable)->pfnDeinit(hDriver) : FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  BootDriverIOCtl
//
__inline
bool_t
BootDriverIoCtl(
    handle_t hDriver,
    enum_t code,
    void *pBuffer,
    size_t size
    )
{
    BootDriverVTable_t **pVTable = (BootDriverVTable_t **)hDriver;
    return (*pVTable)->pfnIoCtl(hDriver, code, pBuffer, size);
}

//------------------------------------------------------------------------------
//
//  Function:  OEMBootCreateDriver
//
handle_t
OEMBootCreateDriver(
    void *pContext,
    enum_t classId,
    enum_t index
    );

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_DRIVER_H
