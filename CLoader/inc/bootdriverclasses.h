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
#ifndef __BOOT_DRIVER_CLASSES_H
#define __BOOT_DRIVER_CLASSES_H

#include <BootTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define:  BOOT_IOCTL
//
#define BOOT_IOCTL(c, i)                    ((c << 16) | i)

//------------------------------------------------------------------------------

#define BOOT_DRIVER_CLASS_TERMINAL          0x0001

#define BOOT_DRIVER_CLASS_DOWNLOAD          0x0010
#define BOOT_DRIVER_CLASS_TRANSPORT         0x0011
#define BOOT_DRIVER_CLASS_BLOCK             0x0012
#define BOOT_DRIVER_CLASS_FILESYSTEM        0x0013

#define BOOT_DRIVER_CLASS_DISPLAY           0x0020
#define BOOT_DRIVER_CLASS_KEYPAD            0x0021
#define BOOT_DRIVER_CLASS_BATTERY           0x0022

#define BOOT_DRIVER_CLASS_OEM               0x8000

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_DRIVER_CLASSES_H
