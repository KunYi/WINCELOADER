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
//------------------------------------------------------------------------------
//
//  File:  oal_memory.h
//
//  This header file defines MEMORY OAL module.
//
#ifndef __OAL_MEMORY_H
#define __OAL_MEMORY_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: OAL_ADDRESS_TABLE    
//
//  Defines the address table entry type.
//
//  The table defines the mapping from the 4GB physical address space
//  to the kernel's 512MB "un-mapped" spaces.  The kernel will create
//  two ranges of virtual addresses from this table. One from 
//  virtual address 0x80000000 to 0x9FFFFFFF which  has caching and buffering
//  enabled and one from 0xA0000000 to 0xBFFFFFFF which has the cache and
//  buffering disabled.
// 
//  Each entry in the table consists of the Cached Virtual Base Address (CA),
//  the  Physical Base Address (PA) to map from, and the number of megabytes
//  to map.
// 
//  The order of the entries is arbitrary, but RAM should be placed first for
//  optimal performance. The table is zero-terminated, so the last entry MUST
//  be all zeroes.
//

typedef struct {
    UINT32  CA;                         // cached virtual address
    UINT32  PA;                         // physical address
    UINT32  size;                       // size, in MB bytes
} OAL_ADDRESS_TABLE, *POAL_ADDRESS_TABLE;

//------------------------------------------------------------------------------
//
//  Define:  OAL_MEMORY_CACHE_BIT
//
//  Defines the address bit that determines if an address is cached
//  or uncached, according to the ranges below:
//
//      0x80000000 - 0x9FFFFFFF ==> CACHED   address
//      0xA0000000 - 0xBFFFFFFF ==> UNCACHED address
//
#define OAL_MEMORY_CACHE_BIT            0x20000000

//------------------------------------------------------------------------------

#if defined(MIPS) || defined(SHx)

#define OALPAtoCA(pa)       (VOID*)(((UINT32)(pa))|0x80000000)
#define OALPAtoUA(pa)       (VOID*)(((UINT32)(pa))|0xA0000000)
#define OALPAtoVA(pa, c)    (VOID*)((c)?(pa)|0x80000000:(pa)|0xA0000000)
#define OALVAtoPA(va)       (((UINT32)(va))&~0xE0000000)
#define OALCAtoUA(va)       (VOID*)(((UINT32)(va))|OAL_MEMORY_CACHE_BIT)
#define OALUAtoCA(va)       (VOID*)(((UINT32)(va))&~OAL_MEMORY_CACHE_BIT)

#else

#define OALPAtoCA(pa)       OALPAtoVA(pa, TRUE)
#define OALPAtoUA(pa)       OALPAtoVA(pa, FALSE)
#define OALCAtoUA(va)       OALPAtoUA(OALVAtoPA((VOID*)(va)))
#define OALUAtoCA(va)       OALPAtoCA(OALVAtoPA((VOID*)(va)))

VOID*
OALPAtoVA(
    UINT32 pa, 
    BOOL cached
    );

UINT32 
OALVAtoPA(
    __in VOID *va
    );

#endif

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif
