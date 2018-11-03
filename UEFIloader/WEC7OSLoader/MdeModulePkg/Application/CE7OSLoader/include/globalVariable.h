//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//------------------------------------------------------------------------------

#ifndef _GLOBALVARIABLE_H
#define _GLOBALVARIABLE_H

#include "include/boottypes.h"

typedef VOID* GLOBALVAR_HANDLE;

bool_t UpdateGlobalVariables(BootConfig_t *pBootConfig);
bool_t CheckIsGlobalVarVaild(BootConfig_t *pBootConfig);
bool_t InitializeGlobalVariables(void);

#endif // _GLOBALVARIABLE_H
