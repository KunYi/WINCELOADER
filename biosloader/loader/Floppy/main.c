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
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <pehdr.h>
#include <romldr.h>
#include <bootarg.h>
#include <parser.h>
#include <util.h>
#include "bldr.h"
#include "debug.h"
#include "fs.h"
#include "ini.h"
#include "video.h"
#include "splash.h"
#include "decoder.h"

//
// Globals
//

extern ULONG DataStartLBA;

// Gets replaced by RomImage with real address (needed to find copy section data)
ROMHDR * volatile const pTOC = (ROMHDR *)-1;

//
// Function prototypes
//
static BOOL LoadBINFile(PCHAR pFileName, PULONG pImgLoc);
static BOOL CopyDataSections(ROMHDR *const pTOC);

// Debug Zone support
DWORD dwZoneMask;

//
// C code entry point
//
void blMain(void)
{
    BOOT_ARGS     *pBootArgs = NULL;
    ULONG         ImgAddr = 0;

    // Locate and assign a pointer to boot argument block.
    ((ULONG)(*(PBYTE *)BOOT_ARG_PTR_LOCATION_NP)) = BOOT_ARG_LOCATION_NP;
    pBootArgs = (BOOT_ARGS *) ((ULONG)(*(PBYTE *)BOOT_ARG_PTR_LOCATION_NP));

    // Copy the initialized data section (so initialized globals are valid).
    // Note: this uses the pTOC pointer above and requires romimage to have fixes up the address during build.
    if (!CopyDataSections(pTOC))
    {
        ERRMSG(MSG_CANNOT_COPY_DATA_SECTIONS, ("Cannot copy initialized data section(s). Halting.\n"));
        goto Halt;
    }

    // Display loader startup banner.
    RTLMSG(TRUE, ("%s %s.%s (Built %s)\r\n", BLDR_SPLASH_MESSAGE, BLDR_VERSION_MAJOR, BLDR_VERSION_MINOR, __DATE__));

    // Write splash text
    WriteText("Version ");
    WriteText(BLDR_VERSION_MAJOR);
    WriteText(".");
    WriteText(BLDR_VERSION_MINOR);
    WriteText(" (Built ");
    WriteText(__DATE__);
    WriteText(")\r\n");

    // Initial DebugMask
    dwZoneMask = 0x0000000F;

    // Call FAT handler initialization
    if (!BINCOMPRESS_INIT())
    {
        ERRMSG(MSG_CANNOT_INIT_FAT, ("Filesystem init failed. Halting.\n"));
        goto Halt;
    }

    // Set up boot loader block
    //bMonitorLoadProgress = FALSE;
    InitBootArgs(pBootArgs);

    // Splash
    if (pBootArgs->ucVideoMode != 0xFF)
    {
        InitSplashScreen();
    }

    // Check if the user pressed any key to change the bin file selection
    // Load the BIN file to RAM.
    //bMonitorLoadProgress = TRUE;
    if (SetUserSelectableBinFile(IniParams.Delay) != BLSTATUS_OK || !LoadBINFile(IniParams.szUserSelectableImage, &ImgAddr))
    {
        if (!LoadBINFile(IniParams.szImage, &ImgAddr))
        {
            INFOMSG(MSG_TRYING_BAK_IMAGE, ("Trying backup image file\n"));
            if (!LoadBINFile(IniParams.szBackupImage, &ImgAddr))
            {
                ERRMSG(MSG_CANNOT_LOAD_BIN, ("Failed to load BIN image\n"));
                goto Halt;
            }
        }
    }
    //bMonitorLoadProgress = FALSE;

    // Initialize requested video mode
    if (pBootArgs->ucVideoMode != 0xFF)
    {
        if (InitVideo(pBootArgs) != BLSTATUS_OK)
        {
            WARNMSG(MSG_FALLBACK_VIDEO_MODE, ("Setting fallback video mode (320x200x256)\n"));
            SetFallbackVideoMode(pBootArgs);
        }
    }

    // Disable all PIC interrupts.
    __asm
    {
        mov     al, 0FFh
        out     021h, al
    }

    RTLMSG(TRUE, ("Jumping to image (address=0x%x)\n\n", ImgAddr));

    // Jump to loaded image.
    if (ImgAddr != 0)
        ((PFN_LAUNCH)(ImgAddr))();

    // Shouldn't get here.
    Halt:

    // Wait forever...
    while (1)
        ;
}


//
// Load a BIN file into memory and jump to the start address.
//
static BOOL LoadBINFile(PCHAR pFileName, PULONG pImageLoc)
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
    if (!BINCOMPRESS_OPEN_FILE(pFileName))
    {
        return(FALSE);
    }


    //
    // Read the BIN file header - we don't do anything with this, but
    // we need to read past it to get to the record header.
    //
    if (!BINCOMPRESS_READ_FILE((PCHAR)&ImageHdr, sizeof(IMAGEHDR)))
    {
        ERRMSG(MSG_CANNOT_READ_IMG_HEADER, ("Cannot read image header\n"));
        return(FALSE);
    }

    //
    // Display image header
    //
#if 0
    SERPRINT("\r\nImage Header:\r\n");
    SERPRINT("------------------------------------------------\r\n");
    SERPRINT("Sync Bytes ................. '%c%c%c%c%c%c'\r\n", ImageHdr.SyncBytes[0], ImageHdr.SyncBytes[1], ImageHdr.SyncBytes[2], ImageHdr.SyncBytes[3], ImageHdr.SyncBytes[4], ImageHdr.SyncBytes[5]);
    SERPRINT("Image Address .............. 0x%x\r\n", ImageHdr.ImageAddr);
    SERPRINT("Image Length ............... 0x%x\r\n", ImageHdr.ImageLen);
#endif  // 0.

    do
    {
        //
        // Read the next BIN file record header
        //
        if (!BINCOMPRESS_READ_FILE((PCHAR)&RecordHdr, sizeof(RECORDHDR)))
        {
            ERRMSG(MSG_CANNOT_READ_REC_HEADER, ("Cannot read record header\n"));
            return(FALSE);
        }

        //
        // Display Record Header
        //
#if 0
        SERPRINT("Record Header:\r\n");
        SERPRINT("------------------------------------------------\r\n");
        SERPRINT("Record Address ...................... 0x%x\r\n", RecordHdr.RecordAddr);
        SERPRINT("Record Length ....................... 0x%x\r\n", RecordHdr.RecordLen);
        SERPRINT("Record CheckSum ..................... 0x%x\r\n", RecordHdr.RecordChksum);
#endif  // 0.

        //
        // Last Header?
        //
        if (RecordHdr.RecordAddr == 0 && RecordHdr.RecordChksum == 0)
        {
            // The last record header contains the image start address (in the length field).
            //
            *pImageLoc = RecordHdr.RecordLen;
            break;
        }

        //
        // Read the entire record into RAM.
        //
        if (!BINCOMPRESS_READ_FILE((PUCHAR)RecordHdr.RecordAddr, RecordHdr.RecordLen))
        {
            ERRMSG(MSG_CANNOT_READ_REC_INFO, ("Cannot read record information (address=0x%x)\n", RecordHdr.RecordAddr));
            return(FALSE);
        }

    }
    while (1);

    //
    // Close file
    //
    BINCOMPRESS_CLOSE_FILE();

    return(TRUE);
}


static BOOL CopyDataSections(ROMHDR *const pTOC)
{
    ULONG Loop;
    COPYentry *pCEntry;

    if (pTOC == (ROMHDR *const) -1)
        return(FALSE);

    // This is where the data sections become valid... don't read globals until after this
    //
    for (Loop = 0; Loop < pTOC->ulCopyEntries; Loop++)
    {
        pCEntry = (COPYentry *)(pTOC->ulCopyOffset + Loop * sizeof(COPYentry));
        if (pCEntry->ulCopyLen)
            memcpy((LPVOID)pCEntry->ulDest, (LPVOID)pCEntry->ulSource, pCEntry->ulCopyLen);
        if (pCEntry->ulCopyLen < pCEntry->ulDestLen)
            memset((LPVOID)(pCEntry->ulDest + pCEntry->ulCopyLen), 0, pCEntry->ulDestLen - pCEntry->ulCopyLen);
    }

    return(TRUE);
}
