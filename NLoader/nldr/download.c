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
#include <FileSysAPI.h>

//////////////////////////////////////////
//
// BIN image and record headers
//
#pragma pack(1)
typedef struct			// Image header (one per BIN image)
{
    CHAR SyncBytes[7];
    ULONG ImageAddr;
    ULONG ImageLen;
} IMAGEHDR, *PIMAGEHDR;
#pragma pack()

#pragma pack(1)
typedef struct			// Record header (one per section in image)
{
    ULONG RecordAddr;
    ULONG RecordLen;
    ULONG RecordChksum;
} RECORDHDR, *PRECORDHDR;
#pragma pack()


//
// Load a BIN file into memory and jump to the start address.
//
BOOL LoadBINFile(PCHAR pFileName, PULONG pImageLoc)
{
    IMAGEHDR ImageHdr;
    RECORDHDR RecordHdr;

    //
    // Check callers arguments
    //
    if (!pFileName || !pImageLoc)
        return(FALSE);

    *pImageLoc = 0;

    //
    // Open the BIN file for reading
    //
    if (!FSOpenFile(pFileName))
    {
        return(FALSE);
    }

    //
    // Read the BIN file header - we don't do anything with this, but
    // we need to read past it to get to the record header.
    //
    if (!FSReadFile((PCHAR)&ImageHdr, sizeof(IMAGEHDR)))
    {
        DEBUGMSG(ZONE_ERROR,(L"Cannot read image header\n"));
        return(FALSE);
    }

    //
    // Display image header
    //
    DEBUGMSG(ZONE_INFO | ZONE_DOWNLOAD, (L"\r\nImage Header:\r\n"));
    DEBUGMSG(ZONE_INFO | ZONE_DOWNLOAD, (L"------------------------------------------------\r\n"));
    DEBUGMSG(ZONE_INFO | ZONE_DOWNLOAD, (L"Sync Bytes ................. '%c%c%c%c%c%c'\r\n", ImageHdr.SyncBytes[0], ImageHdr.SyncBytes[1], ImageHdr.SyncBytes[2], ImageHdr.SyncBytes[3], ImageHdr.SyncBytes[4], ImageHdr.SyncBytes[5]));
    DEBUGMSG(ZONE_INFO | ZONE_DOWNLOAD, (L"Image Address .............. 0x%x\r\n", ImageHdr.ImageAddr));
    DEBUGMSG(ZONE_INFO | ZONE_DOWNLOAD, (L"Image Length ............... 0x%x\r\n", ImageHdr.ImageLen));

    do
    {
        //
        // Read the next BIN file record header
        //
        if (!FSReadFile((PCHAR)&RecordHdr, sizeof(RECORDHDR)))
        {
            DEBUGMSG(ZONE_ERROR,(L"Cannot read record header\n"));
            return(FALSE);
        }

        //
        // Display Record Header
        //
        DEBUGMSG(ZONE_DOWNLOAD, (L"Record Header:\r\n"));
        DEBUGMSG(ZONE_DOWNLOAD, (L"------------------------------------------------\r\n"));
        DEBUGMSG(ZONE_DOWNLOAD, (L"Record Address ...................... 0x%x\r\n", RecordHdr.RecordAddr));
        DEBUGMSG(ZONE_DOWNLOAD, (L"Record Length ....................... 0x%x\r\n", RecordHdr.RecordLen));
        DEBUGMSG(ZONE_DOWNLOAD, (L"Record CheckSum ..................... 0x%x\r\n", RecordHdr.RecordChksum));

        //
        // Last Header?
        //
        if (RecordHdr.RecordAddr == 0 && RecordHdr.RecordChksum == 0)
        {
		    DEBUGMSG(ZONE_INFO | ZONE_DOWNLOAD, (L"\r\nImage Start Address:   0x%x\r\n", RecordHdr.RecordLen));
            // The last record header contains the image start address (in the length field).
            //
            *pImageLoc = RecordHdr.RecordLen;
            break;
        }

        //
        // Read the entire record into RAM.
        //
        if (!FSReadFile((PUCHAR)RecordHdr.RecordAddr, RecordHdr.RecordLen))
        {
            DEBUGMSG(ZONE_ERROR,(L"Cannot read record information (address=0x%x)\n", RecordHdr.RecordAddr));
            return(FALSE);
        }

    }
    while (1);

    //
    // Close file
    //
    FSCloseFile();

    return(TRUE);
}
