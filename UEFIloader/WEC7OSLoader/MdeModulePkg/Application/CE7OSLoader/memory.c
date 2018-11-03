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
//
// -- Intel Copyright Notice -- 
//  
// @par 
// Copyright (c) 2002-2011 Intel Corporation All Rights Reserved. 
//  
// @par 
// The source code contained or described herein and all documents 
// related to the source code ("Material") are owned by Intel Corporation 
// or its suppliers or licensors.  Title to the Material remains with 
// Intel Corporation or its suppliers and licensors. 
//  
// @par 
// The Material is protected by worldwide copyright and trade secret laws 
// and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, 
// distributed, or disclosed in any way except in accordance with the 
// applicable license agreement . 
//  
// @par 
// No license under any patent, copyright, trade secret or other 
// intellectual property right is granted to or conferred upon you by 
// disclosure or delivery of the Materials, either expressly, by 
// implication, inducement, estoppel, except in accordance with the 
// applicable license agreement. 
//  
// @par 
// Unless otherwise agreed by Intel in writing, you may not remove or 
// alter this notice or any other notice embedded in Materials by Intel 
// or Intel's suppliers or licensors in any way. 
//  
// @par 
// For further details, please see the file README.TXT distributed with 
// this software. 
//  
// @par 
// -- End Intel Copyright Notice -- 
//  
#include "include/bootMemory.h"

/**
  BootPAtoVA(), Simply return a input address.

  @param  pAddress      A pointer to a address

  @return uint32_t      A converted physical address.

**/
void *
BootPAtoVA(
    uint32_t    pa,
    bool_t      cache
    )
{
    UNREFERENCED_PARAMETER(cache);
    return (void *)pa;
}

/**
  BootVAtoPA(), Simply return a UINT32 type physical address.

  @param  pAddress      A pointer to a address

  @return uint32_t      A converted physical address.

**/
uint32_t
BootVAtoPA(
    void *pAddress
    )
{
    return (uint32_t)pAddress;
}

/**
  BootVAtoPA(), return a UINT32 type physical address.

  @param  pAddress      A pointer to a address

  @return uint32_t      A converted physical address.

**/
uint32_t
BootImageVAtoPA(
    void *pAddress
    )
{
    return (uint32_t)pAddress & ~0xA0000000;
}

//------------------------------------------------------------------------------

