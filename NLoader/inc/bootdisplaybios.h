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
#ifndef __BOOT_DISPLAY_BIOS_H
#define __BOOT_DISPLAY_BIOS_H

#include <bootDisplay.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#pragma pack(push, 1)

typedef struct BootDisplayBiosVbeInfoBlock_t {
    uint32_t signature;
    uint16_t version;
    uint16_t oemStringPtr[2];
    uint8_t  capabilities[4];
    uint16_t videoModesPtr[2];
    uint16_t totalMemory;
    uint16_t oemSoftwareRev;
    uint16_t oemVendorNamePtr[2];
    uint16_t oemProductNamePtr[2];
    uint16_t oemProductRevPtr[2];
    uint8_t  reserved[222];
    uint8_t  oemData[256];
} BootDisplayBiosVbeInfoBlock_t;

#pragma pack(pop)

//------------------------------------------------------------------------------

enum BootDisplayBiosIoCtl_e {
    BOOT_DISPLAY_IOCTL_MODE_QUERY = BOOT_DISPLAY_IOCTL(0x8001),
    BOOT_DISPLAY_IOCTL_MODE_INFO = BOOT_DISPLAY_IOCTL(0x8002),
    BOOT_DISPLAY_IOCTL_MODE_SET = BOOT_DISPLAY_IOCTL(0x8003)
};

typedef struct BootDisplayModeQueryParams_t {
    enum_t mode;
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t vesaMode;
    uint32_t phFrame;
    uint32_t stride;
    uint32_t redSize;
    uint32_t redPos;
    uint32_t greenSize;
    uint32_t greenPos;
    uint32_t blueSize;
    uint32_t bluePos;
} BootDisplayModeQueryParams_t;

typedef struct BootDisplayModeSetParams_t {
    enum_t mode;
} BootDisplayModeSetParams_t;

//------------------------------------------------------------------------------

handle_t
BootDisplayBiosInit(
    );

//------------------------------------------------------------------------------

__inline
bool_t
BootDisplayModeQuery(
    handle_t hDriver,
    enum_t *pMode,
    uint32_t *pWidth,
    uint32_t *pHeight,
    uint32_t *pBpp
    )
{
    bool_t rc = false;
    BootDisplayModeQueryParams_t params;

    memset(&params, 0, sizeof(params));
    params.mode = *pMode;
    params.width = *pWidth;
    params.height = *pHeight;
    params.bpp = *pBpp;
    if (BootDriverIoCtl(
            hDriver, BOOT_DISPLAY_IOCTL_MODE_QUERY, &params, sizeof(params)
            ))
        {
        *pMode = params.mode;
        *pWidth = params.width;
        *pHeight = params.height;
        *pBpp = params.bpp;
        rc = true;
        }
    return rc;          
}

//------------------------------------------------------------------------------

__inline
bool_t
BootDisplayModeQueryFull(
    handle_t hDriver,
    enum_t *pMode,
    uint32_t *pWidth,
    uint32_t *pHeight,
    uint32_t *pBpp,
    uint32_t *pVesaMode,
    uint32_t *pPhFrame,
    uint32_t *pStride,
    uint32_t *pRedSize,
    uint32_t *pRedPos,
    uint32_t *pGreenSize,
    uint32_t *pGreenPos,
    uint32_t *pBlueSize,
    uint32_t *pBluePos
    )
{
    bool_t rc = false;
    BootDisplayModeQueryParams_t params;

    memset(&params, 0, sizeof(params));
    params.mode = *pMode;
    params.width = *pWidth;
    params.height = *pHeight;
    params.bpp = *pBpp;
    if (BootDriverIoCtl(
            hDriver, BOOT_DISPLAY_IOCTL_MODE_QUERY, &params, sizeof(params)
            ))
        {
        *pMode = params.mode;
        *pWidth = params.width;
        *pHeight = params.height;
        *pBpp = params.bpp;
        *pVesaMode = params.vesaMode;
        *pPhFrame = params.phFrame;
        *pStride = params.stride;
        *pRedSize = params.redSize;
        *pRedPos = params.redPos;
        *pGreenSize = params.greenSize;
        *pGreenPos = params.greenPos;
        *pBlueSize = params.blueSize;
        *pBluePos = params.bluePos;
        rc = true;
        }
    return rc;          
}
//------------------------------------------------------------------------------

__inline
bool_t
BootDisplayModeSet(
    handle_t hDriver,
    enum_t mode
    )
{
    BootDisplayModeSetParams_t params;

    params.mode = mode;
    return BootDriverIoCtl(
        hDriver, BOOT_DISPLAY_IOCTL_MODE_SET, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_DISPLAY_BIOS_H
