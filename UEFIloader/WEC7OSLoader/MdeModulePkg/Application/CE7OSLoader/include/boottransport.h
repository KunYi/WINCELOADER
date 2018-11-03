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
#ifndef __BOOT_TRANSPORT_H
#define __BOOT_TRANSPORT_H

#include "include/bootDriver.h"
#include "include/bootFactory.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define BOOT_TRANSPORT_IOCTL(i)     BOOT_IOCTL(BOOT_DRIVER_CLASS_TRANSPORT, i)

enum BootTransportIoCtl_e {
    BOOT_TRANSPORT_IOCTL_READ   = BOOT_TRANSPORT_IOCTL(1),
    BOOT_TRANSPORT_IOCTL_GET_OS_CONFIG_INFO = BOOT_TRANSPORT_IOCTL(2)
};

typedef struct BootTransportReadParams_t {
    void *pBuffer;
    size_t size;
} BootTransportReadParams_t;

enum BootTransportEdbgReadOSConfigInfoType_e {
    BOOT_TRANSPORT_READ_OS_CONFIG_TYPE_PASSIVE_KITL = 0,   // bool_t returned
    BOOT_TRANSPORT_READ_OS_CONFIG_TYPE_KITL_TRANSPORT = 1  // uint32_t returned
};

typedef struct BootTransportGetOsConfigInfoParams_t {
      enum_t  type;
      void    *pBuffer;
      size_t  BufferSize;
} BootTransportGetOsConfigInfoParams_t;    


//------------------------------------------------------------------------------

#define BootTransportDeinit         BootDriverDeinit
#define BootTransportIoCtl          BootDriverIoCtl

//------------------------------------------------------------------------------
//
//  Function:  BootTransportRead
//
__inline
bool_t
BootTransportRead(
    handle_t hDriver,
    void *pBuffer,
    size_t size
    )
{
    BootTransportReadParams_t params;

    params.pBuffer = pBuffer;
    params.size = size;
    return BootDriverIoCtl(
        hDriver, BOOT_TRANSPORT_IOCTL_READ, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------

__inline
bool_t
BootTransportGetOSConfigInfo(
    handle_t hDriver,
    enum_t   type,
    void     *pBuffer,
    size_t   BufferSize
    )
{
    BootTransportGetOsConfigInfoParams_t params;

    params.type = type;
    params.pBuffer = pBuffer;
    params.BufferSize = BufferSize;
   
    
    return BootDriverIoCtl(
            hDriver, BOOT_TRANSPORT_IOCTL_GET_OS_CONFIG_INFO, &params, sizeof(params)
            );
    
}
    
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_TRANSPORT_H
