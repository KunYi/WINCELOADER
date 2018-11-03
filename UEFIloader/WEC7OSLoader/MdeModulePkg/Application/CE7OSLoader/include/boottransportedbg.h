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
#ifndef __BOOT_TRANSPORT_EDBG_H
#define __BOOT_TRANSPORT_EDBG_H

#include "include/bootTransport.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

enum BootTransportEdbgIoCtl_e {
    BOOT_TRANSPORT_EDBG_IOCTL_OPTIONS = BOOT_TRANSPORT_IOCTL(0x8001)
};

//------------------------------------------------------------------------------

typedef 
bool_t 
(*pfnBootTransportEdbgDmaInit_t)(
    uint32_t buffer,
    uint32_t size
    );

typedef 
bool_t 
(*pfnBootTransportEdbgInit_t)(
    __in_opt uint8_t *pAddress, 
    uint32_t offset,
    __inout uint16_t mac[3]
    );

typedef
bool_t 
(*pfnBootTransportEdbgDeinit_t)(
    );

typedef 
uint32_t
(*pfnBootTransportEdbgSendFrame_t)(
    __in_bcount(length) uint8_t *pBuffer, 
    uint32_t length
    );

typedef
uint32_t
(*pfnBootTransportEdbgRecvFrame_t)(
    __out_bcount(*pLength) uint8_t *pBuffer, 
    __inout uint32_t *pLength
    );

typedef
void
(*pfnBootTransportEdbgFilter_t)(
    flags_t filter
    );

typedef struct BootEdbgDriver_t {
    pfnBootTransportEdbgDmaInit_t pfnDmaInit;
    pfnBootTransportEdbgInit_t pfnInit;
    pfnBootTransportEdbgDeinit_t pfnDeinit;
    pfnBootTransportEdbgSendFrame_t pfnSendFrame;
    pfnBootTransportEdbgRecvFrame_t pfnRecvFrame;
    pfnBootTransportEdbgFilter_t pfnFilter;
} BootEdbgDriver_t;

//------------------------------------------------------------------------------

enum BootTransportEdbgFlag_e {
    BOOT_TRANSPORT_EDBG_FLAG_ENABLED    = 0x0001,   // Kitl enable
    BOOT_TRANSPORT_EDBG_FLAG_PASSIVE    = 0x0002,   // Kitl passive mode
    BOOT_TRANSPORT_EDBG_FLAG_DHCP       = 0x0004,   // DHCP enable
    BOOT_TRANSPORT_EDBG_FLAG_VMINI      = 0x0008,   // VMINI enable
    BOOT_TRANSPORT_EDBG_FLAG_POLL       = 0x0010,   // Use polling (no interrupt)
    BOOT_TRANSPORT_EDBG_FLAG_EXTNAME    = 0x0020    // Extend device name
};

typedef 
bool_t
(*pfnBootTransportEdbgName_t)(
    __in void *pContext,
    __in uint16_t mac[3],
    __out_bcount(bufferSize) string_t buffer,
    size_t bufferSize
    );

handle_t
BootTransportEdbgInit(
    __in void *pContext,
    __in const BootEdbgDriver_t *pDriver,
    __in pfnBootTransportEdbgName_t pfnName,
    __in_opt uint8_t *pAddress,
    uint32_t offset,
    __inout uint16_t mac[3],
    uint32_t ip
    );

//------------------------------------------------------------------------------

typedef struct BootTransportEdbgOptionsParams_t {
    uint32_t bootMeRetry;
    uint32_t bootMeDelay;
    uint32_t tftpTimeout;
    uint32_t dhcpRetry;
    uint32_t dhcpTimeout;
    uint32_t arpTimeout;
} BootTransportEdbgOptionsParams_t;

//------------------------------------------------------------------------------

__inline
bool_t
BootTransportEdbgOptions(
    handle_t hDriver,
    uint32_t bootMeRetry,
    uint32_t bootMeDelay,
    uint32_t tftpTimeout,
    uint32_t dhcpRetry,
    uint32_t dhcpTimeout,
    uint32_t arpTimeout
    )
{
    BootDriverVTable_t **pVTable = hDriver;
    BootTransportEdbgOptionsParams_t params;

    params.bootMeRetry = bootMeRetry;
    params.bootMeDelay = bootMeDelay;
    params.tftpTimeout = tftpTimeout;
    params.dhcpRetry   = dhcpRetry;
    params.dhcpTimeout = dhcpTimeout;
    params.arpTimeout  = arpTimeout;
    return (*pVTable)->pfnIoCtl(
        hDriver, BOOT_TRANSPORT_EDBG_IOCTL_OPTIONS, &params, sizeof(params)
        );
}
    
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
