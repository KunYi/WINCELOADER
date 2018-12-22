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
#include <bootMemory.h>

//------------------------------------------------------------------------------
//
//  Function:  LocalAlloc
//
//  This is simple wrapper which allows use standard C++ library new/delete
//  operators.
//
#ifdef DEBUG
#undef LocalAlloc
#endif

HLOCAL 
LocalAlloc(
    UINT flags, 
    UINT size
    )
{
    UNREFERENCED_PARAMETER(flags);
    return BootAlloc(size);
}    

//------------------------------------------------------------------------------
//
//  Function:  LocalFree
//
//  This is simple wrapper which allows use standard C++ library new/delete
//  operators.
//
HLOCAL 
LocalFree(
    HLOCAL hMemory 
    )
{
    BootFree(hMemory);
    return NULL;
}

//------------------------------------------------------------------------------

