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
#ifndef _FAT32_H_
#define _FAT32_H_

#define READ_BUFFER_START       0xC900      // Start and length of read buffer.
#define READ_BUFFER_LENGTH_MAX  0x3000      // Reduced from 0x4000 to 0x3000 to protect memory over 0xF000
#define SECTOR_CACHE_START      0xB200      // Should match boot.bib reserved area.

#define SECTOR_SIZE                512      // sector size (in bytes).
#define PARTSIG_SIZE                 2      // partition signature size (in bytes)
#define PARTTBL_SIZE                64      // whole partition table size (in bytes)
#define PARTACTV_FLAG             0x80      // active partition flag
#define MAX_PARTITIONS               4      // total number of partition table entries

#define FF_CONTIGOUS  0x2                   // the file is contiguous - don't use FAT

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
{                                //  Offset    Size
    UCHAR  VersionId[8];         //  3         8
    UCHAR  MustBeZero[53];       //  11        53
    ULONGLONG PartitionOffset;   //  64        8
    ULONGLONG PartitionLength;   //  72        8
    ULONG FATOffset;             //  80        4
    ULONG FATLength;             //  84        4
    ULONG ClusterHeapOffset;     //  88        4
    ULONG ClusterCount;          //  82        4
    ULONG FirstClusterInRoot;    //  96        4
    ULONG SerialNumber;          //  100       4
    USHORT FSVersion;            //  104       2
    USHORT ExtFlags;             //  106       2
    UCHAR BytesPerSector;        //  108       1
    UCHAR SectorsPerCluster;     //  109       1
    UCHAR NumberOfFATs;          //  110       1
    UCHAR DriveID;               //  111       1
    UCHAR PercentInUse;          //  112       1
    UCHAR Reserved[7];           //  113       7
} BIOSPB, *PBIOSPB;
#pragma pack()


//
// FAT directory entry structure.
//
#pragma pack(1)
//
//  Define the details of the stream extension record.
//

typedef struct _EXFAT_STREAM_RECORD {

    //
    //  Type will have the value of StreamExtensionType (64)
    //

    UCHAR Type  : 7;
    UCHAR InUse : 1;

    UCHAR Flags;

    UCHAR Reserved;

    //
    //  A count of characters in the file name, spanning both this and
    //  subsequent FileNameExtension records.
    //

    UCHAR CharCount;

    //
    //  This field is a hash of the full upcased name.  This allows a much
    //  faster compare when checking for a file name collision.
    //

    USHORT UpcasedNameHash;

    UCHAR Reserved2[2];

    ULONGLONG ValidDataLength;

    UCHAR Reserved3[4];

    //
    //  Define where the file data lives.
    //

    ULONG FirstCluster;

    ULONGLONG FileSize;

} EXFAT_STREAM_RECORD, *PEXFAT_STREAM_RECORD;

#pragma pack()

//
// Function prototypes
//
BOOL FSInit(void);
ULONG FSOpenFile(PCHAR pFileName);
void FSCloseFile(void);
BOOL FSReadFile(PUCHAR pAddress, ULONG Length);
BOOL FSRewind(ULONG offset);

//
// TODO::FileSize should really become a ULONGLONG eventually.
//

//
// Structure used to keep track of file properties.
//
typedef struct
{
    UCHAR Flags;
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

#endif  // _FAT32_H_
