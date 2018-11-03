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
#ifndef _FAT_H_
#define _FAT_H_

#define READ_BUFFER_START       0xC900      // Start and length of read buffer.
#define READ_BUFFER_LENGTH_MAX  0x3000      // Reduced from 0x4000 to 0x3000 to protect memory over 0xF000
#define SECTOR_CACHE_START      0xA900      // Should match boot.bib reserved area.

#define SECTOR_SIZE                512      // sector size (in bytes).
#define PARTSIG_SIZE                 2      // partition signature size (in bytes)
#define PARTTBL_SIZE                64      // whole partition table size (in bytes)
#define PARTACTV_FLAG             0x80      // active partition flag
#define MAX_PARTITIONS               4      // total number of partition table entries

//
// Recognized FAT filesystem types.
//
typedef enum
{
    FAT_12 = 0,
    FAT_16,
    FAT_32
} FAT_TYPE;

//
// FAT cluster types.
//
typedef enum
{
    EOF_CLUSTER = 1,
    DATA_CLUSTER,
    BAD_CLUSTER,
    RSVD_CLUSTER
} CLUSTTYPE, *PCLUSTTYPE;

//
// BIOS parameter block structure (part of boot sector).
//
#pragma pack(1)
typedef struct BIOSPB_TAG
{
    UCHAR  VersionId[8];
    USHORT BytesPerSect;
    UCHAR  SectsPerClust;
    USHORT RsvdSects;
    UCHAR  NumFATs;
    USHORT NumRootEntries;
    USHORT SectsPerPart;
    UCHAR  MediaDesc;
    USHORT SectsPerFAT;
    USHORT SectsPerTrack;
    USHORT NumHeads;
    USHORT NumHiddenSectL;
    USHORT NumHiddenSectH;
    USHORT TotalSectorsL;
    USHORT TotalSectorsH;
    UCHAR  DriveId;
    UCHAR  TempVal;
    UCHAR  ExtRecordSig;
    USHORT VolSerNumL;
    USHORT VolSerNumH;
    UCHAR  VolLabel[11];
    UCHAR  TypeFAT[8];
} BIOSPB, *PBIOSPB;


//
// FAT directory entry structure.
//
#pragma pack(1)
typedef struct
{
    UCHAR  FileName[8];
    UCHAR  FileExt[3];
    UCHAR  FATTr;
    UCHAR  FOO[10];
    USHORT LModTime;
    USHORT LModDate;
    USHORT FirstClust;
    ULONG  FileSize;
} DIRENTRY, *PDIRENTRY;


//
// Function prototypes
//
BOOL FATInit(void);
ULONG FATOpenFile(PCHAR pFileName);
void FATCloseFile(void);
BOOL FATReadFile(PUCHAR pAddress, ULONG Length);
BOOL FATRewind(ULONG offset);

//
// Structure used to keep track of file properties.
//
typedef struct
{
    ULONG FileSize;
    ULONG FirstCluster;
    ULONG CurCluster;
    ULONG NumContigClusters;
    PUCHAR pCurReadBuffAddr;
    ULONG NumReadBuffBytes;
} FILEINFO, *PFILEINFO;

typedef enum
{
    READ_BY_CLUSTER = 0,
    READ_BY_SECTOR
} READ_MODE;

#endif  // _FAT_H_
