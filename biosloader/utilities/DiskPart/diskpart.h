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

#define EXFAT_SECTORS_TO_CRC 0x0B
#define EXFAT_BOOT_SECTORS   0x0C

enum StoreObjects { soDisk,
                    soPart,
                    soInvalid };

enum PartitionTypes { ptPrimary,
                      ptExtended,
                      ptLogical,
                      ptInvalid };

inline void DisplayPrompt() { _tprintf( _T("DISKPART> ") ); }
inline void DisplayHeader()
{
    _tprintf( _T("Microsoft Windows CE Disk Partitioning Utility\n") );
    _tprintf( _T("Copyright 2006\n\n") );
}


bool ParseCommandLine( TCHAR* pCmdLine, CSimpleString* strScriptFile );
void PrintCommandLineHelp();
bool SizeIsZero( const TCHAR* strSize );
bool GetCommand( FILE* pInput, CSimpleString* strCommand );
bool ParseCommand( CSimpleString* strCommand,
                   CSimpleString* strOp,
                   CSimpleString* strParams );
bool ExecCommand( CSimpleString* strOp, CSimpleString* strParams );

DWORD GetNumberOfDisks( ULONG* ulNumDisks );
HANDLE GetDisk( ULONG ulDisk );
bool GetDiskInfo( ULONG ulDisk, STOREINFO* pStoreInfo );

bool SelectDisk( ULONG ulDisk );

DWORD GetNumberOfPartitions( ULONG* ulNumParts );
HANDLE GetPartition( ULONG ulPart );

bool SelectPartition( ULONG ulPart );

void PrintDiskHeader();
void PrintDisk( ULONG ulDisk, const STOREINFO& StoreInfo );

bool ReadMBR( BYTE** ppSector, DWORD* pdwSize );
bool MBRIsValid( BYTE* pSector, const STOREINFO& StoreInfo );
USHORT EntriesInUse( BYTE* pSector, const STOREINFO& StoreInfo );
bool SectorIsFree( DWORD dwSector, BYTE* pMBR, const STOREINFO& StoreInfo );

//
// Commands
//
bool ActivatePartition( CSimpleString* strParams );
bool CleanDisk();
bool CreatePartition( CSimpleString* strParams );
bool SetPartitionType( CSimpleString* strParams );
bool FormatPartition( CSimpleString* strParams );
bool DeletePartition();
bool PrintHelp( CSimpleString* strParams );
bool ListObjects( CSimpleString* strParams );
bool DeActivatePartition();
bool SelectObject( CSimpleString* strParams );
bool DumpMBR();
bool DumpStoreInfo();
bool DumpPartInfo();
bool ZeroSector( CSimpleString* strParams );
bool DumpSector( CSimpleString* strParams );
bool WriteMBRCode( CSimpleString* strParams );
bool WriteBootSector( CSimpleString* strParams );
bool FixPartOffset();

//
// exFAT only functions
//
bool FixCRC();
bool SetBackupBootSection();

//
// Command Help
//
void PrintActiveHelp();
void PrintSelectHelp();
void PrintListHelp();
void PrintCreateHelp();
void PrintMBRCodeHelp();
void PrintZeroSectorHelp();
void PrintDumpSectorHelp();
void PrintWriteBootSectorHelp();

//
// Structure for FAT/FAT32 BPB.
//
#pragma pack(1)

typedef struct
{
    BYTE    BPB_jmpBoot[3];
    BYTE    BPB_OEMName[8];
    WORD    BPB_BytesPerSector;     /* Sector size                      */
    BYTE    BPB_SectorsPerCluster;  /* sectors per allocation unit      */
    WORD    BPB_ReservedSectors;    /* DOS reserved sectors             */
    BYTE    BPB_NumberOfFATs;       /* Number of FATS                   */
    WORD    BPB_RootEntries;        /* Number of directories            */
    WORD    BPB_TotalSectors;       /* Partition size in sectors        */
    BYTE    BPB_MediaDescriptor;    /* Media descriptor                 */
    WORD    BPB_SectorsPerFAT;      /* Fat sectors                      */
    WORD    BPB_SectorsPerTrack;    /* Sectors Per Track                */
    WORD    BPB_Heads;              /* Number heads                     */
    DWORD   BPB_HiddenSectors;      /* Hidden sectors                   */
    DWORD   BPB_BigTotalSectors;    /* Number sectors                   */
} BASE_BPB, *PBASE_BPB;

typedef struct
{
        BASE_BPB BPB_Base;
        DWORD   BPB_SectorsPerFAT32;
        WORD    BPB_ExtFlags;
        WORD    BPB_FSVersion;
        DWORD   BPB_RootCluster;
        WORD    BPB_FSInfoSec;
        WORD    BPB_BkUpBootSec;
        BYTE    BPB_Reserved[12];
        BYTE    BPB_DriveNumber;
        BYTE    BPB_Reserved1;
        BYTE    BPB_BootSignature;
        DWORD   BPB_VolumeID;
        BYTE    BPB_VolumeLabel[11];
        BYTE    BPB_FileSystemType[8];
} BPB32, *PBPB32;

//
// exFAT BPB
//
typedef struct
{                                //  Offset    Size
    BYTE JmpBoot[3];             //  0         3
    UCHAR VersionId[8];          //  3         8
    UCHAR MustBeZero[53];        //  11        53
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
} BPBEX, *PBPBEX;

#pragma pack()
