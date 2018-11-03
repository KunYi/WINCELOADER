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
#ifndef __XLDR_H
#define __XLDR_H

#include <bootTypes.h>

//------------------------------------------------------------------------------

#define VERSION_MAJOR               1
#define VERSION_MINOR               0

//------------------------------------------------------------------------------

#define IMAGE_XLDR_BOOTSEC_PA       0xFA00
#define IMAGE_XLDR_BUFFER_PA        0xFC00

//------------------------------------------------------------------------------

handle_t
FileSystemInit(
    );

bool_t
FileSystemRead(
    handle_t hFileSystem,
    void *pBuffer,
    size_t size
    );

void
FileSystemDeinit(
    handle_t hFileSystem
    );

//------------------------------------------------------------------------------

#endif
