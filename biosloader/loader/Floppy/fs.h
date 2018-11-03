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
#pragma once

#include <windows.h>

typedef BOOL  (* _FSInit)(void);
typedef ULONG (* _FSOpenFile)(char*);
typedef void  (* _FSCloseFile)(void);
typedef BOOL  (* _FSReadFile)(char*, ULONG);
typedef BOOL  (* _FSRewind)(ULONG);

BOOL  FSInit();
ULONG FSOpenFile( char* pFileName );
void  FSCloseFile( void );
BOOL  FSReadFile( unsigned char* pAddress, ULONG Length );
BOOL  FSRewind( ULONG Offset );

//////////////////////////////////////////
//
// BIN image and record headers
//
#pragma pack(1)
typedef struct          // Image header (one per BIN image)
{
    CHAR SyncBytes[7];
    ULONG ImageAddr;
    ULONG ImageLen;
} IMAGEHDR, *PIMAGEHDR;
#pragma pack()

#pragma pack(1)
typedef struct          // Record header (one per section in image)
{
    ULONG RecordAddr;
    ULONG RecordLen;
    ULONG RecordChksum;
} RECORDHDR, *PRECORDHDR;
#pragma pack()

typedef struct
{
    UCHAR DriveId;
    USHORT NumHeads;
    USHORT SectorsPerTrack;
} DRIVE_INFO, *PDRIVE_INFO;
