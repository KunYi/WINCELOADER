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
#ifndef __BOOT_FACTORY_H
#define __BOOT_FACTORY_H

#include <bootTypes.h>
#include <bootDriverClasses.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#endif // __BOOT_FACTORY_H
