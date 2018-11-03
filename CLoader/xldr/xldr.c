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
#include "xldr.h"
#include <bootLog.h>
#include <pehdr.h>
#include <romldr.h>

//------------------------------------------------------------------------------
//  Global variables

ROMHDR* 
volatile
const
pTOC = (ROMHDR *)-1; 

//------------------------------------------------------------------------------

#pragma pack(push, 1)

typedef struct BinImageHeader_t {
    uint8_t sign[7];
    uint32_t address;
    uint32_t length;
} BinImageHeader_t;

typedef struct BinImageRecord_t {
    uint32_t address;
    uint32_t length;
    uint32_t checkSum;
} BinImageRecord_t;

#pragma pack(pop)

typedef 
void 
(*PFN_LAUNCH)(
    );

//------------------------------------------------------------------------------
//  Local Functions

static
bool_t
SetupCopySection(
    );

//------------------------------------------------------------------------------

void
XldrMain(
    )
{
    handle_t hFile;
    BinImageHeader_t header;
    BinImageRecord_t record;
    uint32_t sum, start, ix;
    
    // Setup global variables
    if (!SetupCopySection()) goto powerOff;

    BootLog(
        L"Microsoft Windows CE XLDR Version %d.%d (Built %S %S)\r\n",
        VERSION_MAJOR, VERSION_MINOR, __DATE__, __TIME__
        );
    
    hFile = FileSystemInit();
    if (hFile == NULL) goto powerOff;

    // Read header...
    if (!FileSystemRead(hFile, &header, sizeof(header)))
        {
        BootLog(L"BootLoader Image Header Read Failed!\r\n");
        goto powerOff;
        }

    // We don't want downloaded image will modify running code
    if ((memcmp(header.sign, "B000FF\x0A", sizeof(header.sign)) != 0) ||
        (header.address < 0x10000) || 
        ((header.address + header.length) < 0x1000))
        {
        BootLog(L"BootLoader Image Header Is Invalid!\r\n");
        goto powerOff;
        }
    
    for (;;)
        {
        // Read the record header
        if (!FileSystemRead(hFile, &record, sizeof(record)))
            {
            BootLog(L"BootLoader Record Header Read Failed!\r\n");
            goto powerOff;
            }

        // Last Header?
        if ((record.address == 0) && (record.checkSum == 0))
            {
            // Save address & break while loop
            start = record.length;
            break;
            }

        // Verify header
        if ((record.address < header.address) ||
            ((record.address - header.address + record.length) > header.length))
            {
            BootLog(L"BootLoader Image Record Header Corrupted!\r\n");
            goto powerOff;
            }
        
        // Read record
        if (!FileSystemRead(hFile, (void*)record.address, record.length))
            {
            BootLog(L"BootLoader Record Data Read Failed!\r\n");
            goto powerOff;
            }
        
        // Verify checksum
        sum = 0;
        for (ix = 0; ix < record.length; ix++)
            {
            sum += ((uint8_t*)record.address)[ix];
            }
        if (sum != record.checkSum)
            {
            BootLog(L"BootLoader Record Checksum Verification Failed!\r\n");
            goto powerOff;
            }
        
        }

    FileSystemDeinit(hFile);

BootLog(L"XLDR: Ready to jump to main loader...\r\n");


    // Jump to image
    ((PFN_LAUNCH)(start))();

powerOff:
    BootLog(L"*** Spin For Ever ***\r\n");
    for (;;)
        ;
}

//------------------------------------------------------------------------------
//
//  Function:  SetupCopySection
//
//  Copies image's copy section data (initialized globals) to the correct
//  fix-up location.  Once completed, initialized globals are valid.
//
bool_t
SetupCopySection(
    )
{
    bool_t rc = false;
    uint32_t loop, count;
    COPYentry *pCopyEntry;
    const uint8_t *pSrc;
    uint8_t *pDst;


    if (pTOC == (ROMHDR *const) -1) goto cleanUp;

    pCopyEntry = (COPYentry *)pTOC->ulCopyOffset;
    for (loop = 0; loop < pTOC->ulCopyEntries; loop++)
        {
        count = pCopyEntry->ulCopyLen;
        pDst = (uint8_t*)pCopyEntry->ulDest;
        pSrc = (uint8_t*)pCopyEntry->ulSource; 
        memcpy(pDst, pSrc, count);
        pDst += count;
        count = pCopyEntry->ulDestLen - pCopyEntry->ulCopyLen;
        memset(pDst, 0, count);
        }

    rc = true;

cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------

