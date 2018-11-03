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
#include "bldr.h"
#include <bootBlockUtils.h>
#include <oal_args.h>

//------------------------------------------------------------------------------

enum_t
OEMBootLoad(
    void *pContext,
    enum_t state
    )
{
    enum_t rc = (enum_t)BOOT_STATE_FAILURE;
    BootLoader_t* pLoader = pContext;

    BOOTMSG(ZONE_FUNC, (L"+OEMBootLoad(0x%08x, 0x%08x)\r\n", pContext, state));

    // Depending on state
    switch (state)
        {
        case BOOT_STATE_POWERON:
            rc = BootLoaderPowerOn(pLoader);
            break;
        case BOOT_STATE_CONFIG:
            rc = BootLoaderConfig(pLoader);
            break;
        case BOOT_STATE_DOWNLOAD:
            rc = BootLoaderDownload(pLoader);
            break;
        case BOOT_STATE_PRELOAD:
            rc = BootLoaderPreLoad(pLoader);
            break;
        case BOOT_STATE_LOAD_ULDR:
            rc = BootLoaderLoadUldr(pLoader);
            break;
        case BOOT_STATE_LOAD_OS:
            rc = BootLoaderLoadOs(pLoader);
            break;
        }

    BOOTMSG(ZONE_FUNC, (L"-OEMBootLoad(rc = 0x%08x)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
