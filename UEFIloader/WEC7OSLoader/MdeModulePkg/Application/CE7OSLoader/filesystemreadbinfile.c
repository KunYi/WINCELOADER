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
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiMemoryLib/MemLibInternals.h>

#include "include/bldr.h"
#include "include/boot.h"
#include "include/bootdebug.h"

/**
  BootFileSystemReadBinFile(), This function called by BootLoaderLoadOs() to
  used EFI_FILE_HANDLE of read() function to read nk.bin from boot deviec
  to RAM.
  It will check opened file has signature or not to decide whether this file is CE
  image format, if so, it will read image header and the read record headre one by one
  to put image data in correct address.

  @param  hFile     A EFI_FILE_HANDLE handle for opend image file.

  @param  offset    If need to read on special address, this offest will be the
                    start address to read.

  @param  pAddress  A Pointer to get jump address from last record header of length.

  @return bool_t    A BOOL value to indicate read NK to RAM is success or not.

**/
bool_t
BootFileSystemReadBinFile(
    EFI_FILE_HANDLE hFile,
    size_t offset,
    uint32_t *pAddress
    )
{
    bool_t                      rc = false;
    BootBinFormatRamHeader_t    imageHeader;
    uint8_t                     sign[7];
    uint32_t                    imageBase, imageTop;
    DWORD                       *pRamaddress;

    if (!BootFileSystemRewind(hFile, 0)) {
        ERRMSG(TRUE, (L"BootFileSystemReadBinFile: "
            L"Failed seek to file start!\r\n"
            ));
        goto cleanUp;
    }

    // Read signature
    if ((BootFileSystemRead(hFile, sign, sizeof(sign)) != sizeof(sign)) ||
        (memcmp(sign, BOOT_BIN_SIGNATURE_RAM, sizeof(sign) != 0))) {
        ERRMSG(TRUE, (L"BootFileSystemReadBinFile: "
            L"Invalid file signature!\r\n"
            ));
        goto cleanUp;
    }

    // Read image header
    if (BootFileSystemRead(
            hFile, &imageHeader, sizeof(imageHeader)
            ) != sizeof(imageHeader)) {
        ERRMSG(TRUE, (L"BootFileSystemReadBinFile: "
            L"Failed read image header from file!\r\n"
            ));
        goto cleanUp;
    }

    DBGMSG(DBGZONE_INFO, (L"\r\nImage Header:\r\n"));
    DBGMSG(DBGZONE_INFO, (L"------------------------------------------------\r\n"));
    DBGMSG(DBGZONE_INFO, (L"Sync Bytes ................. '%c%c%c%c%c%c'\r\n",
        sign[0], sign[1], sign[2], sign[3], sign[4], sign[5]));
    DBGMSG(DBGZONE_INFO, (L"Image Address .............. 0x%x\r\n", imageHeader.start));
    DBGMSG(DBGZONE_INFO, (L"Image Length ............... 0x%x\r\n", imageHeader.size));

    // Check image location
    imageBase = imageHeader.start + offset;
    imageTop = imageBase + imageHeader.size;
    if (imageTop < imageBase) {
        ERRMSG(TRUE, (L"BootFileSystemReadBinFile: "
            L"Image overlap 32-bit address window!\r\n"
            ));
        goto cleanUp;
    }

    // Allocate image buffer from UEFI before we actually writing to it.
    if (AllocateImageBuffer(&imageHeader.start, imageHeader.size + IMAGE_EXTRA_MEM_LENGTH) == FALSE) {
        ERRMSG(TRUE, (L"BootLoaderLoadOs():Cannot read Allocate Image Buffer\n"));
        goto cleanUp;
    }

    pRamaddress = (DWORD *)((imageHeader.start+ imageHeader.size) & ~(sizeof(DWORD) - 1));
    InternalMemSetMem(pRamaddress, IMAGE_EXTRA_MEM_LENGTH, 0);

    // Read all records
    for (;;) {
        BootBinFormatRamRecordHeader_t header;
        uint32_t base, top, sum, ix;
        uint8_t* pData;

        // Read record header
        if (BootFileSystemRead(
                hFile, &header, sizeof(header)
                ) != sizeof(header)) {
            ERRMSG(TRUE, (L"BootFileSystemReadBinFile: "
                L"Failed read record header from file!\r\n"
                ));
            goto cleanUp;
        }

        // Check for last record
        if ((header.address == 0) && (header.checksum == 0)) {
            *pAddress = BootImageVAtoPA((void*)header.length);
            ERRMSG(TRUE, (L"BootFileSystemReadBinFile(): Start Address= 0x%X\n",
                *pAddress));
            break;
        }

        // Check if record fit in declated image location
        base = header.address + offset;
        top = base + header.length;
        if ((top < base) || (base < imageBase) || (top > imageTop)) {
            ERRMSG(TRUE, (L"BootFileSystemReadBinFile: "
                L"Invalid record header (0x%08x, 0x%08x, 0x%08x)!\r\n",
                header.address, header.length, offset
                ));
            goto cleanUp;
        }

        // Calculate boot loader address
        pData = BootPAtoCA(BootImageVAtoPA((void*)base));

        // Read record to memory
        if (BootFileSystemRead(hFile, pData, header.length) != header.length) {
            ERRMSG(TRUE, (L"BootFileSystemReadBinFile: "
                L"Failed read record data from file!\r\n"
                ));
            goto cleanUp;
        }

        // Calculate check sum
        for (ix = 0, sum = 0; ix < header.length; ix++) sum += pData[ix];
        if (sum != header.checksum) {
            ERRMSG(TRUE, (L"BootFileSystemReadBinFile: "
                L"Calculated record checksum doesn't match header!\r\n"
                ));
            goto cleanUp;
        }

        DBGMSG(DBGZONE_INFO, (L"Record Header:\r\n"));
        DBGMSG(DBGZONE_INFO, (L"------------------------------------------------\r\n"));
        DBGMSG(DBGZONE_INFO, (L"Record Address ...................... 0x%x\r\n", pData));
        DBGMSG(DBGZONE_INFO, (L"Record Length ....................... 0x%x\r\n", header.length));
        DBGMSG(DBGZONE_INFO, (L"Record CheckSum ..................... 0x%x\r\n", header.checksum));

    }

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

