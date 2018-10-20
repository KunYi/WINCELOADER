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

#define RW_BUFFER_START         0xD000      // Start and length of read buffer.
#define RW_BUFFER_LENGTH_MAX    0x2000      // Reduced from 0x3000 to 0x2000 to protect memory over 0xF000
#define SECTOR_CACHE_START      0xB200      // Should match boot.bib reserved area.

#define SECTOR_SIZE                512      // sector size (in bytes).
#define PARTSIG_SIZE                 2      // partition signature size (in bytes)
#define PARTTBL_SIZE                64      // whole partition table size (in bytes)
#define PARTACTV_FLAG             0x80      // active partition flag
#define MAX_PARTITIONS               4      // total number of partition table entries
#define LAST_LFN_FLAG             0x40      // for last LFN part
#define DELETE_MARK               0xE5      // for DELETE operation

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
{                            //  Offset    Size
    UCHAR  VersionId[8];     //  3         8
    USHORT BytesPerSect;     //  11        2
    UCHAR  SectsPerClust;    //  13        1
    USHORT RsvdSects;        //  14        2
    UCHAR  NumFATs;          //  16        1
    USHORT NumRootEntries;   //  17        2
    USHORT SectsPerPart;     //  19        2
    UCHAR  MediaDesc;        //  21        1
    USHORT SectsPerFAT;      //  22        2
    USHORT SectsPerTrack;    //  24        2
    USHORT NumHeads;         //  26        2
    USHORT NumHiddenSectL;   //  28        4
    USHORT NumHiddenSectH;
    USHORT TotalSectorsL;    //  32        4
    USHORT TotalSectorsH;
    USHORT SectsPerFATL;     //  36        4
    USHORT SectsPerFATH;
    USHORT Flags;            //  40        2
    USHORT FSVersion;        //  42        2
    USHORT RootClusterL;     //  44        4
    USHORT RootClusterH;
    USHORT FSInfo;           //  48        2
    USHORT BackupBootSector; //  50        2
    UCHAR  Reserved[12];     //  52        12
    UCHAR  DriveId;          //  64        1
    UCHAR  Reserved1;        //  65        1
    UCHAR  BootSig;          //  66        1
    USHORT VolumeIdL;        //  67        4
    USHORT VolumeIdH;
    UCHAR  Label[11];        //  71        11
    UCHAR  TypeFAT[8];       //  82        8
} BIOSPB, *PBIOSPB;

//
// FAT directory entry structure.
//
// New, more correct
typedef struct
{
    UCHAR  FileName[8];
    UCHAR  FileExt[3];
    UCHAR  FATTr;
    ULONG  CreateTime;

    USHORT CreateDate;
    USHORT LAccessDate;
    USHORT FirstClustH;
    USHORT LModTime;
    USHORT LModDate;
    USHORT FirstClustL;
    ULONG  FileSize;
} DIRENTRY, *PDIRENTRY;

//
// FAT32 directory entry structure
//

typedef struct
{
    UCHAR  Order;         // Order of this entry for long entries associated with the file.
    USHORT Name1[5];      // Characters 1-5 of long name component.
    UCHAR  Attributes;    // ATTR_LONG_NAME
    UCHAR  Type;
    UCHAR  Checksum;
    USHORT Name2[6];      // Characters 6-11 of this long name component.
    USHORT MustZero;      // This is meaningless and must be zero.
    USHORT Name3[2];      // Characters 12-13 of this long name component.
} DIRENTRY32, *PDIRENTRY32;

//
// Filesystem information structure of FAT32
//
typedef struct
{
    UCHAR  LEADSIG[4];
    UCHAR  MustZero[480];
    UCHAR  SIG[4];
    ULONG  FreeCount;
    ULONG  NextFreeCluster;
    UCHAR  Reserved[12];
    ULONG  TRAILSIG[4];
} FSINFO32, *PFSINFO32;

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

typedef struct
{
    ULONG FileSize;
    ULONG FirstCluster;
} SFILEINFO, *PSFILEINFO;

typedef enum
{
    READ_BY_CLUSTER = 0,
    READ_BY_SECTOR
} READ_MODE;

typedef enum
{
    ATTR_RO = 1,
    ATTR_HID = 2,
    ATTR_SYS = 4,
    ATTR_LAB = 8,
    ATTR_DIR = 16,
    ATTR_ARC = 32,
    ATTR_LFN = ATTR_RO + ATTR_HID + ATTR_SYS + ATTR_LAB, 
    ATTR_MASK = ATTR_RO + ATTR_HID + ATTR_SYS + ATTR_LAB + ATTR_DIR + ATTR_ARC
} FILE_ATTRIBUTE;
#endif  // _FAT32_H_
