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

#ifndef _SPLASH_H
#define _SPLASH_H

// NOTE: A 24-bit screen will be requested
#define SPLASH_SCREEN_WIDTH                 640
#define SPLASH_SCREEN_HEIGHT                480

#define PROGRESS_BAR_WIDTH                  100
#define PROGRESS_BAR_HEIGHT                 10
#define PROGRESS_BAR_TOP_MARGIN             5
#define PROGRESS_BAR_COLOUR                 0x00383838      // 6bits per primary
#define PROGRESS_BAR_UPDATE_THRESHOLD       25

#define SPLASH_IMAGE_FILE_NAME              "splash.bmp"

#define COMPRESSED_SPLASH_IMAGE_FILE_NAME   "splash.bmx"

typedef struct _BMP_HEADER
{
    WORD    Type;
    DWORD   Size;
    DWORD   Reserved;
    DWORD   Offset;
    DWORD   headerSize;
    DWORD   Width;
    DWORD   Height;
    WORD    Planes;
    WORD    BitsPerPixel;
    DWORD   Compression;
    DWORD   SizeImage;
    DWORD   XPixelsPerMeter;
    DWORD   YPixelsPerMeter;
    DWORD   ColorsUsed;
    DWORD   ColorsImportant;
} BMP_HEADER;

// Public function prototypes
BLSTATUS InitSplashScreen();
void SetProgressValue(int progress);

#endif
