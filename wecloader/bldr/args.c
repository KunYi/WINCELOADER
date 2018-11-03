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
#include <oal_args.h>

#undef ZONE_ERROR
#include <bootArg.h>
#include <ceddk.h>

//------------------------------------------------------------------------------

void
BootArgsInit(
    bool_t force
    )
{
    BOOT_ARGS **ppBootArgs = (BOOT_ARGS **)IMAGE_SHARE_BOOT_ARGS_PTR_PA;
    BOOT_ARGS *pBootArgs = (BOOT_ARGS *)IMAGE_SHARE_BOOT_ARGS_PA;

    if ((pBootArgs->dwSig != BOOTARG_SIG) ||
        (pBootArgs->dwLen < sizeof(BOOT_ARGS)) ||
        (*ppBootArgs != pBootArgs))
        {
        force = true;
        }

    if (force)
        {
        *ppBootArgs = pBootArgs;
        memset(pBootArgs, 0, sizeof(*pBootArgs));
        pBootArgs->dwSig = BOOTARG_SIG;
        pBootArgs->dwLen = sizeof(BOOT_ARGS);
        pBootArgs->dwVersionSig = BOOT_ARG_VERSION_SIG;
        pBootArgs->MajorVersion = BOOT_ARG_MAJOR_VER;
        pBootArgs->MinorVersion = BOOT_ARG_MINOR_VER;
        }
}

//------------------------------------------------------------------------------

void*
BootArgsQuery(
    BootLoader_t *pLoader,
    enum_t type
    )
{
    void *pData = NULL;
    BOOT_ARGS* pBootArgs = (BOOT_ARGS *)IMAGE_SHARE_BOOT_ARGS_PA;
    static BOOL updateMode = FALSE;

    // Check if there is expected signature
    if ((pBootArgs->dwSig != BOOTARG_SIG) ||
        (pBootArgs->dwLen < sizeof(BOOT_ARGS)))
        goto cleanUp;

    switch (type)
        {
        case OAL_ARGS_QUERY_UPDATEMODE:
            updateMode = (pLoader->imageUpdateFlags & OAL_ARGS_UPDATEMODE) ? TRUE : FALSE;
            pData = &updateMode;
            break;
        }
    
cleanUp:
    return pData;
}

//------------------------------------------------------------------------------


