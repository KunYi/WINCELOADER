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
#ifndef __BOOT_DISPLAY_H
#define __BOOT_DISPLAY_H

#include <bootDriver.h>
#include <bootFactory.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define BOOT_DISPLAY_IOCTL(i)       BOOT_IOCTL(BOOT_DRIVER_CLASS_DISPLAY, i)
#define BOOT_DISPLAY_IOCTL_OEM(i)   BOOT_IOCTL(BOOT_DRIVER_CLASS_DISPLAY, 0x8000 + i)

enum BootDisplayIoCtl_e {
    BOOT_DISPLAY_IOCTL_FILLRECT = BOOT_DISPLAY_IOCTL(0x0001),
    BOOT_DISPLAY_IOCTL_BLTRECT  = BOOT_DISPLAY_IOCTL(0x0002),
    BOOT_DISPLAY_IOCTL_SLEEP    = BOOT_DISPLAY_IOCTL(0x0003),
    BOOT_DISPLAY_IOCTL_AWAKE    = BOOT_DISPLAY_IOCTL(0x0004)
};

typedef struct BootDisplayFillRectParams_t {
    RECT *pRect;
    uint32_t color;
} BootDisplayFillRectParams_t;

typedef struct BootDisplayBltRectParams_t {
    RECT *pRect;
    void *pBuffer;
} BootDisplayBltRectParams_t;

//------------------------------------------------------------------------------

#define BootDisplayDeinit   BootDriverDeinit
#define BootDisplayIoCtl    BootDriverIoCtl

//------------------------------------------------------------------------------

__inline
bool_t
BootDisplayFillRect(
    handle_t hDriver,
    RECT *pRect,
    uint32_t color
    )
{
    BootDisplayFillRectParams_t params;
    
    params.pRect = pRect;
    params.color = color;
    return BootDriverIoCtl(
        hDriver, BOOT_DISPLAY_IOCTL_FILLRECT, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------

__inline
bool_t
BootDisplayBltRect(
    handle_t hDriver,
    RECT *pRect,
    void *pBuffer
    )
{
    BootDisplayBltRectParams_t params;

    params.pRect = pRect;
    params.pBuffer = pBuffer;
    return BootDriverIoCtl(
        hDriver, BOOT_DISPLAY_IOCTL_BLTRECT, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------

__inline
bool_t
BootDisplaySleep(
    handle_t hDriver
    )
{
    return BootDriverIoCtl(hDriver, BOOT_DISPLAY_IOCTL_SLEEP, NULL, 0);
}

//------------------------------------------------------------------------------

__inline
bool_t
BootDisplayAwake(
    handle_t hDriver
    )
{
    return BootDriverIoCtl(hDriver, BOOT_DISPLAY_IOCTL_AWAKE, NULL, 0);
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_DISPLAY_H
