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
#include <pehdr.h>
#include <romldr.h>

//------------------------------------------------------------------------------
// External variables

extern
ROMHDR*
volatile
const
pTOC;

extern
uint32_t
BootStackSize;

//------------------------------------------------------------------------------

#define BOOT_HEAP_COOKIE            'paeh'
#define BOOT_HEAP_ALLOC             (1 << 31)

typedef struct BootHeapBlock_t {
    uint32_t cookie;
    bool_t free;
    size_t prevSize;
    size_t size;
} BootHeapBlock_t;

//------------------------------------------------------------------------------

static
BootHeapBlock_t*
s_bootHeap = NULL;

//------------------------------------------------------------------------------

void
BootHeapInit(
    )
{
    size_t size;
    BootHeapBlock_t* pBlock;

    // Check if there is space for heap...
    if ((INT32)pTOC == -1) goto cleanUp;
    if (pTOC->ulRAMEnd <= pTOC->ulRAMFree) goto cleanUp;
    size = pTOC->ulRAMEnd - pTOC->ulRAMFree;
    if (size < BootStackSize) goto cleanUp;
    size -= BootStackSize;
    if (size < sizeof(BootHeapBlock_t)) goto cleanUp;

    pBlock = (BootHeapBlock_t*)pTOC->ulRAMFree;
    size -= sizeof(BootHeapBlock_t);

    pBlock->cookie = BOOT_HEAP_COOKIE;
    pBlock->free = TRUE;
    pBlock->prevSize = 0;
    pBlock->size = size;
    s_bootHeap = pBlock;
    pBlock = (BootHeapBlock_t*)((UCHAR*)pBlock + size);
    pBlock->cookie = BOOT_HEAP_COOKIE;
    pBlock->free = FALSE;
    pBlock->prevSize = size;
    pBlock->size = 0;

cleanUp:
    return;
}

//------------------------------------------------------------------------------

void*
BootAlloc(
    size_t size
    )
{
    VOID *pMemory = NULL;
    BootHeapBlock_t *pBlock = s_bootHeap;
    DWORD blockSize;
    

    // Check if heap is initialized and size isn't bigger than 512KB
    if ((pBlock == NULL) || (size >= 0x20000000)) goto cleanUp;

    // Include header and round to 32bit boundary
    size = (size + sizeof(BootHeapBlock_t) + 3) & ~0x03;

    // Look for free block which we can use
    for (;;)
        {
        // Check header cookie
        if (pBlock->cookie != BOOT_HEAP_COOKIE)
            {
//            BOOTMSG(ZONE_ERROR, (L"ERROR: BootAlloc: "
//                L"Heap memory layout destroyed!\r\n"
//                ));
            goto cleanUp;
            }

        // Last block has size zero...
        if (pBlock->size == 0) goto cleanUp;

        // If item has sufficient size we find it
        if (pBlock->free && (pBlock->size >= size)) break;

        // Move to next item
        pBlock = (BootHeapBlock_t*)((UCHAR*)pBlock + pBlock->size);
        }

    // Should it be divided?
    blockSize = pBlock->size - size;    
    if (blockSize >= (sizeof(BootHeapBlock_t) + 4))
        {
        BootHeapBlock_t *pAlloc, *pNext;

        // Get next block and update size in header
        pNext = (BootHeapBlock_t*)((UCHAR*)pBlock + pBlock->size);
        if (pNext->cookie != BOOT_HEAP_COOKIE)
            {
//            BOOTMSG(ZONE_ERROR, (L"ERROR: BootAlloc: "
//                L"Heap memory layout destroyed!\r\n"
//                ));
            goto cleanUp;
            }
        pNext->prevSize = size;

        // Create block with required size at top
        pAlloc = (BootHeapBlock_t*)((UCHAR*)pBlock + blockSize);
        pAlloc->cookie = BOOT_HEAP_COOKIE;
        pAlloc->free = FALSE;
        pAlloc->size = size;
        pAlloc->prevSize = blockSize;

        // Reduce original block size
        pBlock->size = blockSize;

        // Allocated memory starts after block header
        pMemory = &pAlloc[1];
        }
    else
        {
        // Block is now allocated
        pBlock->free = FALSE;

        // Allocated memory starts after block header
        pMemory = &pBlock[1];
        }

cleanUp:
    return pMemory;
}

//------------------------------------------------------------------------------

void
BootFree(
    void *pMemory
    )
{
    BootHeapBlock_t *pBlock, *pNext;


    // Ignore calls with NULL
    if (pMemory == NULL) goto cleanUp;

    // Get and check block header
    pBlock = (BootHeapBlock_t*)((UCHAR*)pMemory - sizeof(BootHeapBlock_t));
    if ((pBlock->cookie != BOOT_HEAP_COOKIE) || pBlock->free) goto cleanUp;

    // Mark block as free
    pBlock->free = TRUE;

    // Get next block header
    pNext = (BootHeapBlock_t*)((UCHAR*)pBlock + pBlock->size);
    if (pNext->cookie != BOOT_HEAP_COOKIE) goto cleanUp;

    // If this block is free merge them
    if (pNext->free)
        {
        BootHeapBlock_t* pNext2;

        pNext2 = (BootHeapBlock_t*)((UCHAR*)pNext + pNext->size);
        if (pNext2->cookie != BOOT_HEAP_COOKIE) goto cleanUp;
        pNext2->prevSize += pBlock->size;
        pBlock->size += pNext->size;
        pNext->cookie = 0;
        pNext = pNext2;
        }

    // If there is previous block
    if (pBlock->prevSize != 0)
        {
        BootHeapBlock_t *pPrev;

        pPrev = (BootHeapBlock_t*)((UCHAR*)pBlock - pBlock->prevSize);
        if (pPrev->cookie != BOOT_HEAP_COOKIE) goto cleanUp;

        // If previous block is free merge them
        if (pPrev->free)
            {
            pNext->prevSize += pPrev->size;
            pPrev->size += pBlock->size;
            pBlock->cookie = 0;
            }
        }

    // Done
    pMemory = NULL;

cleanUp:
//    BOOTMSG(ZONE_ERROR && (pMemory != NULL), (L"ERROR: BootFree: "
//        L"Heap memory layout destroyed!\r\n"
//        ));
    return;
}

//------------------------------------------------------------------------------

