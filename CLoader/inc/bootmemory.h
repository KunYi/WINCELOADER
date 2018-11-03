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
#ifndef __BOOT_MEMORY_H
#define __BOOT_MEMORY_H

#include <bootTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: BOOT_ADDRESS_TABLE    
//
//  Defines the address table entry type.
//
//  The table defines the mapping from the 4GB physical address space
//  to the kernel's 512MB "un-mapped" spaces.  The kernel will create
//  two ranges of virtual addresses from this table. One from virtual
//  address 0x80000000 to 0x9FFFFFFF which  has caching and buffering
//  enabled and one from 0xA0000000 to 0xBFFFFFFF which has the cache
//  and buffering disabled.
// 
//  Each entry in the table consists of the Cached Virtual Base Address
//  (CA), the  Physical Base Address (PA) to map from, and the number
//  of megabytes to map.
// 
//  The order of the entries is arbitrary, but RAM should be placed
//  first for optimal performance. The table is zero-terminated, so
//  the last entry MUST be all zeroes.
//
typedef struct {
    uint32_t  CA;                  // cached virtual address
    uint32_t  PA;                  // physical address
    uint32_t  size;                // size, in MB bytes
} BOOT_MAP_TABLE;

//------------------------------------------------------------------------------
//
//  Define:  BOOT_MEMORY_CACHE_BIT
//
//  Defines the address bit that determines if an address is cached
//  or uncached, according to the ranges below:
//
//      0x80000000 - 0x9FFFFFFF ==> CACHED   address
//      0xA0000000 - 0xBFFFFFFF ==> UNCACHED address
//
#define BOOT_MEMORY_CACHE_BIT   0x20000000

//------------------------------------------------------------------------------

#define BootCAtoUA(va)          (VOID*)(((uint32_t)(va))|BOOT_MEMORY_CACHE_BIT)
#define BootUAtoCA(va)          (VOID*)(((uint32_t)(va))&~BOOT_MEMORY_CACHE_BIT)

#if defined(MIPS) || defined(SHx)

#define BootPAtoCA(pa)          (VOID*)(((uint32_t)(pa))|0x80000000)
#define BootPAtoUA(pa)          (VOID*)(((uint32_t)(pa))|0xA0000000)
#define BootPAtoVA(pa, c)       (VOID*)((c)?(pa)|0x80000000:(pa)|0xA0000000)
#define BootVAtoPA(va)          (((uint32_t)(va))&~0xE0000000)
#define BootImageVAtoPA(va)     BootVAtoPA(va)

#else

#define BootPAtoCA(pa)          BootPAtoVA(pa, TRUE)
#define BootPAtoUA(pa)          BootPAtoVA(pa, FALSE)

void*
BootPAtoVA(
    uint32_t pa,
    bool_t cached
    );

uint32_t
BootVAtoPA(
    void *pAddress
    );

uint32_t
BootImageVAtoPA(
    void *pAddress
    );

#endif

//------------------------------------------------------------------------------
//
//  Function:  BootAlloc
//
void*
BootAlloc(
    uint32_t size
    );

//------------------------------------------------------------------------------
//
//  Function:  BootFree
//
void
BootFree(
    void* pMemory
    );

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_MEMORY_H
