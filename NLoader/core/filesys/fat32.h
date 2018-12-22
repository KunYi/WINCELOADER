//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
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

#ifdef __cplusplus
extern "C" {
#endif

#define READ_BUFFER_START       0xC900		// Start and length of read buffer.
#define READ_BUFFER_LENGTH_MAX  0x3000		// Reduced from 0x4000 to 0x3000 to protect memory over 0xF000
#define SECTOR_CACHE_START      0xB200		// Should match boot.bib reserved area.

#define SECTOR_SIZE                512		// sector size (in bytes).
#define PARTSIG_SIZE                 2		// partition signature size (in bytes)
#define PARTTBL_SIZE                64		// whole partition table size (in bytes)
#define PARTACTV_FLAG             0x80		// active partition flag
#define MAX_PARTITIONS               4		// total number of partition table entries

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
} BIOSPB32, *PBIOSPB32;
#pragma pack()


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
#pragma pack()

//
// FAT32 directory entry structure
//
#pragma pack(1)
typedef struct
{
    UCHAR  Order;         // Order of this entry for long entries associated with the file.
    UCHAR  Name1[10];     // Characters 1-5 of long name component.
    UCHAR  Attributes;    // ATTR_LONG_NAME
    UCHAR  Type;
    UCHAR  Checksum;
    UCHAR  Name2[12];     // Characters 6-11 of this long name component.
    USHORT FirstCluster;  // This is meaningless and must be zero.
    UCHAR  Name3[4];      // Characters 12-13 of this long name component.
} DIRENTRY32, *PDIRENTRY32;
#pragma pack()

////
//// Function prototypes
////
//BOOL FAT32_FSInit(void);
//ULONG FAT32_FSOpenFile(PCHAR pFileName);
//void FAT32_FSCloseFile(void);
//BOOL FAT32_FSReadFile(PUCHAR pAddress, ULONG Length);
//BOOL FAT32_FSRewind(ULONG offset);

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

#ifdef __cplusplus
}
#endif

#endif	// _FAT32_H_
