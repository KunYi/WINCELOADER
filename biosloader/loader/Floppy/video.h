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

#ifndef _VIDEO
#define _VIDEO

#pragma pack(1)
typedef struct _VESA_GENERAL_INFO
{
    UCHAR   szSignature[4];
    WORD    wVersion;
    PCHAR   pszVendor;
    DWORD   dwCapabilities;
    PWORD   pModeList;
    WORD    wTotalMemory;
    WORD    wOemSoftwareRev;
    PCHAR   pszVenderName;
    PCHAR   pszProductName;
    PCHAR   pszProductRev;
    UCHAR   ucReserved[222];
    UCHAR   ucOemData[256];
} VESA_GENERAL_INFO;

#pragma pack(1)
typedef struct _VESA_MODE_INFO
{
    short           wModeAttributes;
    unsigned char   ucWindowAAttributes;
    unsigned char   ucWindowBAttributes;
    short           wWindowGranularity;
    short           wWindowSize;
    short           wWindowASegment;
    short           wWindowBSegment;
    unsigned long   pWindowSchemeFunction;
    unsigned short  wBytesPerScanLine;
    unsigned short  wXResolution;
    unsigned short  wYResolution;
    unsigned char   ucXCharSize;
    unsigned char   ucYCharSize;
    unsigned char   ucNumberOfPlanes;
    unsigned char   ucBitsPerPixel;
    unsigned char   ucNumberOfBanks;
    unsigned char   ucMemoryModel;
    unsigned char   ucBankSize;
    unsigned char   ucNumberOfImagePages;
    unsigned char   ucReserved1;
    unsigned char   ucRedMaskSize;
    unsigned char   ucRedFieldPosition;
    unsigned char   ucGreenMaskSize;
    unsigned char   ucGreenFieldPosition;
    unsigned char   ucBlueMaskSize;
    unsigned char   ucBlueFieldPosition;
    unsigned char   ucRsvdMaskSize;
    unsigned char   ucRsvdFieldPosition;
    unsigned char   ucDirectColorModeInfo;
    unsigned long   dwPhysBasePtr;
    unsigned char   ucReserved2[212];
} VESA_MODE_INFO;

extern VESA_MODE_INFO minfo;

// Public functions prototypes
BLSTATUS InitVideo(BOOT_ARGS * pBootArgs);
BLSTATUS FindVESAMode(BOOT_ARGS * pBootArgs);
BLSTATUS SetVESAMode(WORD vesaMode);
void SetFallbackVideoMode(BOOT_ARGS * pBootArgs);
void WriteText(PCHAR text);

#endif
