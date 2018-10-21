//
// Copyright (c) Microsoft Corporation.  All rights reserved.
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
#include <bootarg.h>
#include "fat32.h"
#include "debug.h"
#include "bldr.h"
#include "splash.h"
#include "fs.h"
#include "video.h"

extern ULONG DataStartLBA;

BOOLEAN ReadSectors(UCHAR DriveID, ULONG LBA, USHORT nSectors, PUCHAR pBuffer);
BOOLEAN WriteSector(UCHAR DriveID, ULONG LBA, PUCHAR pBuffer);
static BOOL InitReadBuffer(ULONG StartAddress, ULONG Length);
ULONG GetNextCluster(ULONG Cluster);

PBIOSPB pBPB = NULL;

static PUCHAR g_pReadBuffStart = 0;
static ULONG g_ReadBuffLenInClusters = 0;
static ULONG g_ReadBuffLenInSectors = 0;

static READ_MODE g_DiskReadMode = READ_BY_CLUSTER;
static ULONG g_SectorsInCurrentClusterLeftToRead = 0;
static ULONG g_CurrentSector = 0;

//
// File information
//
static FILEINFO g_FileInfo;

// This data supplements the BPB information - used to save time.
struct
{
    ULONG       FATLBA;             // LBA start of FAT (first)
    ULONG       RootDirLBA;         // LBA start of root directory
    ULONG       DataStartLBA;       // LBA start of data area
    ULONG       FsInfoLBA;
} g_FATParms;



BOOL IsDataCluster(ULONG Cluster)
{
    Cluster &= 0x0fffffff;
    if (Cluster >= 0x00000002 && Cluster <= 0x0fffffef)
        return(TRUE);

    return(FALSE);
}


BOOL IsRsvdCluster(ULONG Cluster)
{
    Cluster &= 0x0fffffff;
    if (Cluster >= 0x0ffffff0 && Cluster <= 0x0ffffff6)
        return(TRUE);

    return(FALSE);
}


BOOL IsEOFCluster(ULONG Cluster)
{
    Cluster &= 0x0fffffff;
    if (Cluster >= 0x0ffffff8 && Cluster <= 0x0fffffff)
        return(TRUE);

    return(FALSE);
}


BOOL IsBadCluster(ULONG Cluster)
{
    Cluster &= 0x0fffffff;
    if (Cluster == 0x0ffffff7)
        return(TRUE);

    return(FALSE);
}

ULONG Cluster2LBA(ULONG Cluster)
{
    return(g_FATParms.DataStartLBA + (Cluster - 2) * pBPB->SectsPerClust);
}

ULONG LBA2Cluster(ULONG LBA)
{
    return(((LBA-g_FATParms.DataStartLBA) / pBPB->SectsPerClust) + 2);
}

ULONG NextSector(ULONG Sector)
{
    ULONG Cluster = LBA2Cluster( Sector++ );
    if (Cluster != LBA2Cluster(Sector)) {
        Cluster = GetNextCluster( Cluster );
        if ( IsDataCluster( Cluster )) 
            return Cluster2LBA( Cluster );
        else 
            Sector = 0;
    }
    return Sector;
}

static void UpdateFSInfo(ULONG Cluster, ULONG NumCluster)
{
    PFSINFO32 pFSInfo = (PFSINFO32)RW_BUFFER_START;

    if (g_FATParms.FsInfoLBA == 0)
        return;

    if (!ReadSectors( pBPB->DriveId, g_FATParms.FsInfoLBA, 1, (PUCHAR)pFSInfo))
    {
        ERRMSG(MSG_CANNOT_READ_SECTOR, ("Failed to read FSInfo(0x%x) sector.\n", g_FATParms.FsInfoLBA));
        return;
    }
    if (0xFFFFFFFF == pFSInfo->FreeCount)
        return;

    pFSInfo->FreeCount += NumCluster;
    pFSInfo->NextFreeCluster = Cluster;
    WriteSector(pBPB->DriveId, g_FATParms.FsInfoLBA, (PUCHAR)pFSInfo);
}

static ULONG ReleaseFileClusters(PSFILEINFO sfi)
{
    PUCHAR pSectorCache = (PUCHAR)SECTOR_CACHE_START;   // Sector cache is where the sector used to read the FAT cluster chains lives.
    ULONG Sector;
    ULONG ByteOffset;
    ULONG Count;
    ULONG NextCluster = sfi->FirstCluster;
    ULONG NumOfCluster = sfi->FileSize / pBPB->SectsPerClust / pBPB->BytesPerSect;
    NumOfCluster += (sfi->FileSize % (pBPB->SectsPerClust * pBPB->BytesPerSect)) ? 1 : 0;
#ifdef _DBG_RELCLS
    SERPRINT("++ReleaseFileClusters(), Cluster:0x%x, Size:%d, TotalClusters:%d\n", 
        NextCluster, sfi->FileSize, NumOfCluster);
#endif
    if (!IsDataCluster(NextCluster))
        return 0;

    // initialize
    g_CurrentSector = 0;
    Count = 0;

    while (NumOfCluster--)
    {
#ifdef _DBG_RELCLS
        SERPRINT("**  Cluster:0x%x\n", NextCluster);
#endif
        if (!IsDataCluster(NextCluster))
        {
            ERRMSG(MSG_BROKEN_CLUSTER_CHAIN, ("Cluster chain broken: 0x%x, Sector:0x%x, ByteOffset:0x%x\n", 
                NextCluster, Sector, ByteOffset));
            return 0;
        }

        // Compute sector where our FAT entry lives.
        Sector = NextCluster * sizeof(ULONG);
        ByteOffset = Sector & (pBPB->BytesPerSect-1);
        Sector /= pBPB->BytesPerSect;
        Sector += g_FATParms.FATLBA;
        if (g_CurrentSector != Sector)
        {
            if (g_CurrentSector != 0)
                WriteSector( pBPB->DriveId, g_CurrentSector, pSectorCache);

            if (!ReadSectors( pBPB->DriveId, Sector, 1, pSectorCache))
            {
                ERRMSG(MSG_CANNOT_READ_SECTOR, "Failed to read FAT sector:0x%x". Sector);
                g_CurrentSector = 0;
                return 0;
            }
            g_CurrentSector = Sector;
        }

        NextCluster = *(PULONG)(pSectorCache + ByteOffset);
        *(PULONG)(pSectorCache + ByteOffset) = 0;
        Count++;
        if (IsEOFCluster(NextCluster))
            break;
    }

    if (g_CurrentSector != 0)
        WriteSector( pBPB->DriveId, g_CurrentSector, pSectorCache);

#ifdef _DBG_RELCLS
    SERPRINT("--ReleaseFileClusters(). LastCluster:0x%x, NumOfCluster:%d, Count%d\n",
        NextCluster, NumOfCluster, Count);
#endif
    return Count;
}

//
// Initialize the FAT subsystem.
//
BOOL FSInit(void)
{
    PFSINFO32 pFSInfo = (PFSINFO32)RW_BUFFER_START;
    ULONG ReadBufferLen = 0;
    ULONG Tmp;
    USHORT Count;
    UCHAR FATType[9];

    // 
    // BIOS parameter block was left behind by the boot sector code - use it.
    // Note: the address is determined by the master boot record (MBR) - this is the location (-3 bytes) where it
    // loads the boot sector and the sector contains the BIOS parameter block.
    //
    pBPB = (PBIOSPB)0xFA03;

    Tmp = pBPB->RootClusterL + (pBPB->RootClusterH << 16);

    // Compute LBA for various disk regions.
    g_FATParms.DataStartLBA = DataStartLBA;
    g_FATParms.RootDirLBA   = Cluster2LBA( Tmp );
    g_FATParms.FATLBA       = g_FATParms.DataStartLBA - (pBPB->NumFATs * (pBPB->SectsPerFATL + (pBPB->SectsPerFATH << 16)));

    // Determine FAT filesystem type.
    memcpy(FATType, pBPB->TypeFAT, 8);
    FATType[8] = '\0';

    if (memcmp(FATType, "FAT32", 5))
    {
        ERRMSG(MSG_UNKNOWN_FILE_SYSTEM, ("Unknown file system: '%s'\n", FATType));
        return(FALSE);
    }

#if 0
    SERPRINT("\nDrive Info:\n");
    SERPRINT(" - Drive ID ................... 0x%x\n", pBPB->DriveId);
    SERPRINT(" - Sector Size ...............  0x%x\n\n", pBPB->BytesPerSect);
    SERPRINT(" - Heads ...................... 0x%x\n", pBPB->NumHeads);
    SERPRINT(" - Number of Sectors Per Track  0x%x\n\n", pBPB->SectsPerTrack);

    SERPRINT("FAT Info:\n");
    SERPRINT(" - FAT Type ................... %s\n", FATType);
    SERPRINT(" - Cluster Size ............... 0x%x\n", (pBPB->SectsPerClust * pBPB->BytesPerSect));
    SERPRINT(" - Number of FATs ............. 0x%x\n", pBPB->NumFATs);
    SERPRINT(" - Number of Sectors Per FAT .. 0x%x%x\n", pBPB->SectsPerFATH, pBPB->SectsPerFATL);
    SERPRINT(" - Number of Hidden Sectors ... 0x%x%x\n", pBPB->NumHiddenSectH, pBPB->NumHiddenSectL);
    SERPRINT(" - Number of Reserved Sectors . 0x%x\n\n", pBPB->RsvdSects);
    SERPRINT(" - Root dir location (LBA) .... 0x%x\n", g_FATParms.RootDirLBA);
    SERPRINT(" - FAT location (LBA) ......... 0x%x\n", g_FATParms.FATLBA);
    SERPRINT(" - Data location (LBA) ........ 0x%x\n", g_FATParms.DataStartLBA);
#endif

    ReadBufferLen = RW_BUFFER_LENGTH_MAX;

    //
    // Initialize the read buffer.
    //
    if (!InitReadBuffer(RW_BUFFER_START, ReadBufferLen))
    {
        ERRMSG(MSG_CANNOT_INIT_READ_BUFFER, ("Failed to initialize read buffer\n"));
        return(FALSE);
    }

    // Find FS Info
    Count = pBPB->RsvdSects;
    Tmp = g_FATParms.FATLBA - Count + 1;
    g_FATParms.FsInfoLBA = 0;
    do {
        if (ReadSectors(pBPB->DriveId, Tmp, 1, (PUCHAR)pFSInfo)) {
            UCHAR HEADSIG[4] = { 'R', 'R', 'a', 'A' };
            UCHAR STRUCTSIG[4] = { 'r', 'r', 'A', 'a' };
            UCHAR TRAILSIG[4] = { 0, 0, 0x55, 0xAA };
            if ((memcmp(pFSInfo->LEADSIG, HEADSIG, 4) == 0) &&
                (memcmp(pFSInfo->SIG, STRUCTSIG, 4) == 0) &&
                (memcmp(pFSInfo->TRAILSIG, TRAILSIG, 4) == 0))
            {
                g_FATParms.FsInfoLBA = Tmp;
                break;
            }
        }
        Tmp++;
    } while(--Count);

    return(TRUE); 
}

static CHAR UNI2STR(USHORT code)
{
    if (code < 128)
        return (CHAR)code;
    return 0;
}

static void FillFileNameBuff(CHAR* buf, PDIRENTRY32 pDirE32)
{
    USHORT i = 0;
    for ( i = 0; i < 5; i++)
        buf[i] = UNI2STR(pDirE32->Name1[i]);
    for ( i = 0; i < 6; i++)
        buf[i + 5] = UNI2STR(pDirE32->Name2[i]);
    for ( i = 0; i < 2; i++)
        buf[i + 11] = UNI2STR(pDirE32->Name3[i]);
}

static ULONG findDirDocuments(void)
{
    ULONG DirLBA = 0;
    USHORT i = 0;
    PUCHAR Sector = (PUCHAR)RW_BUFFER_START;
    PDIRENTRY pDirEntry = NULL;
    CHAR buff[27]= { 0 };
    CONST CHAR  Docs[] = "Documents and Settings";

    for( DirLBA = g_FATParms.RootDirLBA; DirLBA ; DirLBA = NextSector( DirLBA ) )
    {
        // Read a sector from the root directory.
        if (!ReadSectors(pBPB->DriveId, DirLBA, 1, Sector))
        {
            ERRMSG(MSG_CANNOT_READ_ROOT_DIR, ("Couldn't read root directory sector (LBA=0x%x)\n", DirLBA));
            return(0);
        }

        // Try to find the "Documents and Settings" in the root directory
        for (pDirEntry = (PDIRENTRY)Sector, i = 0 ; i < (pBPB->BytesPerSect / sizeof(DIRENTRY)) ; i++, pDirEntry++)
        {
            if (!(pDirEntry->FileName[0] | pDirEntry->FATTr))
                return 0; // not found;

            if ((pDirEntry->FATTr  & ATTR_MASK) == ATTR_LFN)
            {
                PDIRENTRY32 pDirEntry32 = (PDIRENTRY32)pDirEntry;
                UCHAR sno = pDirEntry32->Order ^ LAST_LFN_FLAG;

                // for avoid error entry
                // MAX_PATH(260)/13 == 20
                if (sno > 20)
                    continue;

                if (sno == 2)
                {
                    FillFileNameBuff((buff+13), pDirEntry32++);
                    FillFileNameBuff(buff, pDirEntry32);
                }

                pDirEntry += sno;
                i += sno;

                // check pDirEntry in sector spaces
                if ((PUCHAR)pDirEntry >= (Sector + pBPB->BytesPerSect))
                    break;

                if ((sno == 2) &&
                    (pDirEntry->FATTr & ATTR_DIR) &&
                    (pDirEntry->FileName[0] == 'D') &&
                    (memcmp(Docs, buff, sizeof(Docs)) == 0)
                    )
                {
                    return Cluster2LBA(pDirEntry->FirstClustL + (pDirEntry->FirstClustH << 16));
                }
            }
        }
    }
    return 0; // not found;
}

static void DelDefaultVol(ULONG DirLBA, PSFILEINFO sfInfo)
{
    USHORT i = 0;
    PUCHAR Sector = (PUCHAR)RW_BUFFER_START;
    PDIRENTRY pDirEntry = NULL;
    PDIRENTRY32 pDirEntry32 = NULL;
    CHAR buff[14]= { 0 };
    CONST CHAR  Docs[] = "default.vol";
    CONST CHAR  sDocs[] = "DEFAULT VOL";

    for (; DirLBA; DirLBA = NextSector( DirLBA ))
    {
        if (!ReadSectors(pBPB->DriveId, DirLBA, 1, Sector))
        {
            ERRMSG(MSG_CANNOT_READ_DOCS_DIR, ("Couldn't read directory sector (LBA=0x%x)\n", DirLBA));
            return;
        }

        // Try to find the "default" in the directory
        for (pDirEntry = (PDIRENTRY)Sector, i = 0; i < (pBPB->BytesPerSect / sizeof(DIRENTRY)); i++, pDirEntry++)
        {
            // check end of entry
            if (!(pDirEntry->FileName[0] | pDirEntry->FATTr))
                break;

            if ((pDirEntry->FATTr & ATTR_MASK) == ATTR_LFN)
            {
                UCHAR sno = pDirEntry32->Order ^ LAST_LFN_FLAG;;
                pDirEntry32 = (PDIRENTRY32)pDirEntry;

                // for avoid error entry and 0xE5 delete entry
                // MAX_PATH(260)/13 == 20
                if (sno > 20)
                    continue;

                if (sno == 1)
                    FillFileNameBuff(buff, pDirEntry32);

                pDirEntry += sno;
                i += sno;

                // check pDirEntry in sector spaces
                if ((PUCHAR)pDirEntry >= (Sector + pBPB->BytesPerSect))
                    break;

                // for LFN record
                if ((sno == 1) &&
                    (memcmp(Docs, buff, 11) == 0) &&
                    (pDirEntry->FileName[0] == 'D'))
                    *((PULONG)buff) = 0xAAF02501; // magic code for found
                else
                    *((PULONG)buff) = 0;

            } // End of LFN
            // for 8.3 Name
            if ((memcmp(sDocs, pDirEntry->FileName, 11) == 0) ||
                 (*((PULONG)buff) == 0xAAF02501))
            {
                if (*((PULONG)buff) == 0xAAF02501)
                    pDirEntry32->Order = DELETE_MARK;

                sfInfo->FileSize = pDirEntry->FileSize;
                sfInfo->FirstCluster = pDirEntry->FirstClustL + (pDirEntry->FirstClustH << 16);
                pDirEntry->FileName[0] = DELETE_MARK;
                WriteSector(pBPB->DriveId, DirLBA, Sector);
                return;
            }
        }
    }
}

BOOL FSPreHook(void)
{
    ULONG DirLBA = 0;
    CHAR* Sector = (CHAR*)RW_BUFFER_START;
    SFILEINFO sFileInfo = { 0 };

	if (DirLBA == 0) // for normal
		return TRUE;

    DirLBA = findDirDocuments();
    if (!DirLBA)
        return FALSE;

    // locate and delete default.vol
    DelDefaultVol(DirLBA, &sFileInfo);
    if (IsDataCluster(sFileInfo.FirstCluster))
    {
        ULONG NumOfCluster = 0;
        if (NumOfCluster = ReleaseFileClusters(&sFileInfo))
            UpdateFSInfo(sFileInfo.FirstCluster, NumOfCluster);
    }

    return TRUE;
}

//
// Open filename specified and return the size.
//
ULONG FSOpenFile(char* pFileName)
{
    USHORT i = 0;
    BOOL bFound = FALSE;
    ULONG DirLBA = 0;
    CHAR* Sector = (CHAR*)RW_BUFFER_START;
    PDIRENTRY pDirEntry = NULL;
    CHAR FileName[11] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};

    // Check caller's parameters.
    if (pFileName == NULL)
        return(0);

    // Convert the filename to the form stored on disk.
    for (i = 0 ; i < 8 && *(pFileName + i) != '\0' && *(pFileName + i) != '.' ; i++)
        FileName[i] = TO_UPPER(*(pFileName + i));
    
    if (*(pFileName + i) == '.')
    {
        USHORT j = 0;
        i++;
        for (j=0 ; i < 12 && *(pFileName + i) != '\0' ; i++, j++)
            FileName[8 + j] = TO_UPPER(*(pFileName + i));
    }

    // Look for the filename in directory list.
    for( DirLBA = g_FATParms.RootDirLBA; !bFound && DirLBA ; DirLBA = NextSector( DirLBA ) )
    {
        // Read a sector from the root directory.
        if (!ReadSectors(pBPB->DriveId, DirLBA, 1, Sector))
        {
            ERRMSG(MSG_CANNOT_READ_ROOT_DIR, ("Couldn't read root directory sector (LBA=0x%x)\n", DirLBA));
            return(0);
        }

        // Try to find the specified file in the root directory
        for (pDirEntry = (PDIRENTRY)Sector, i = 0 ; i < (pBPB->BytesPerSect / sizeof(DIRENTRY)) ; i++, pDirEntry++)
        {
            if (!memcmp(FileName, pDirEntry->FileName, 11))
            {
                ULONG FirstClust = (pDirEntry->FirstClustH << 16) + pDirEntry->FirstClustL;
                g_FileInfo.FileSize            = pDirEntry->FileSize;
                g_FileInfo.FirstCluster        = FirstClust;
                g_FileInfo.CurCluster          = FirstClust;
                g_FileInfo.NumContigClusters   = 0;
                g_FileInfo.pCurReadBuffAddr    = g_pReadBuffStart;
                g_FileInfo.NumReadBuffBytes    = 0;
                INFOMSG(MSG_FILE_FOUND, ("Found file '%s' (start=0x%x size=0x%x)\n",
                    pFileName, FirstClust, pDirEntry->FileSize));
                return (g_FileInfo.FileSize);
            }
        }
    }

    WARNMSG(MSG_FILE_NOT_FOUND, ("File '%s' not found\n", pFileName));
    return(0);
}



void FSCloseFile(void)
{
    memset(&g_FileInfo, 0, sizeof(g_FileInfo));
    return;
}


static BOOL InitReadBuffer(ULONG StartAddress, ULONG Length)
{
    // Check arguments.
    //
    if (StartAddress == 0)
        return(FALSE);

    // Determine how many clusters are in the read buffer.
    //
    g_ReadBuffLenInClusters = Length / (pBPB->BytesPerSect * pBPB->SectsPerClust);

    // Determine how many sectors are in the read buffer.
    //
    g_ReadBuffLenInSectors = Length / pBPB->BytesPerSect ;


    if (!g_ReadBuffLenInClusters)
    {
        // The read buffer is smaller than a cluster size
        g_DiskReadMode = READ_BY_SECTOR;
        g_SectorsInCurrentClusterLeftToRead = 0;
    }

    // Store read buffer start and the adjusted length.
    //
    g_pReadBuffStart        = (PUCHAR)StartAddress;

    return(TRUE);
}

ULONG GetNextCluster(ULONG Cluster)
{
    ULONG Sector = 0;
    ULONG ByteOffset = 0;
    PUCHAR pSectorCache = (PUCHAR)SECTOR_CACHE_START;   // Sector cache is where the sector used to read the FAT cluster chains lives.
    ULONG NextCluster = 0;

    // If we're passed an EOF cluster, return it.
    //
    if (IsEOFCluster(Cluster))
        return(Cluster);

    // Is caller giving us a valid cluster?
    //
    if (!IsDataCluster(Cluster))
    {
        ERRMSG(MSG_BAD_CLUSTER_NUMBER, ("Bad cluster number\n"));
        return(0);  // 0 isn't a valid cluster number (at least for our purposes).
    }

    // Compute sector where our FAT entry lives.
    //
    Sector = Cluster * sizeof(ULONG);
    ByteOffset = Sector & (pBPB->BytesPerSect-1);
    Sector /= pBPB->BytesPerSect;
    Sector += g_FATParms.FATLBA;

    // If the sector we're interested in isn't in our cache, get it.
    //
    if (g_CurrentSector != Sector)
    {
        if (!ReadSectors( pBPB->DriveId, Sector, 1, pSectorCache))
        {
//          TODO: Only a message?
//          SERPRINT("GetNextCluster - unable to read sector.\r\n");
        }

        g_CurrentSector = Sector;
    }

    // Locate next cluster number...
    //
    NextCluster = *(PULONG)(pSectorCache + ByteOffset);

    //SERPRINT("GetNextCluster - cluster=0x%x  next cluster=0x%x.\r\n", Cluster, NextCluster);

    // Return the next cluster value.
    //
    return(NextCluster);
}


CLUSTTYPE GetNextClusterGroup(ULONG StartCluster, PULONG pNumberClusters)
{
    ULONG CurrentCluster = 0;
    ULONG NextCluster = 0;

    *pNumberClusters = 0;

    // TODO - could be a bad cluster - check for it here.

    // Determine number of contiguous clusters.
    //
    NextCluster = StartCluster;
    do
    {
        CurrentCluster = NextCluster;
        *pNumberClusters = *pNumberClusters + 1;
        NextCluster = GetNextCluster(CurrentCluster);
        if (NextCluster != (CurrentCluster + 1))
            break;
    } while(IsDataCluster(NextCluster));

    // Why did we break out of the above loop?
    //
    if (IsEOFCluster(NextCluster))
        return(EOF_CLUSTER);
    if (IsBadCluster(NextCluster))
        return(BAD_CLUSTER);
    if (IsRsvdCluster(NextCluster))
        return(RSVD_CLUSTER);

    return(DATA_CLUSTER);
}


BOOL FSReadFile( unsigned char * pAddress, ULONG Length)
{
    ULONG  ClustersToRead = 0;
    USHORT SectorsToRead;
    ULONG  CurrentLBA;

    // Validate callers parameters.
    //
    if (!pAddress || Length == 0)
        return(FALSE);

    // Make sure callers called InitReadBuffer before calling us.
    //
    if (!g_pReadBuffStart )
    {
        ERRMSG(MSG_FAT_NOT_INITIALIZED, ("FAT must be initialized before reading files\n"));
        return(FALSE);
    }

    //SERPRINT("Length: 0x%x\r\n", Length);
    do
    {
        // Is there data in the read buffer?
        //
        if (g_FileInfo.NumReadBuffBytes)
        {
            if (Length <= g_FileInfo.NumReadBuffBytes)
            {
                // We can satisfy the read request with what's in the read buffer already.
                //
                memcpy(pAddress, g_FileInfo.pCurReadBuffAddr, Length);
                g_FileInfo.NumReadBuffBytes  -= Length;
                g_FileInfo.pCurReadBuffAddr  += Length;
                return(TRUE);
            }
            else
            {
                // Empty the read buffer before loading more.
                //
                memcpy(pAddress, g_FileInfo.pCurReadBuffAddr, g_FileInfo.NumReadBuffBytes);
                Length   -= g_FileInfo.NumReadBuffBytes;
                pAddress += g_FileInfo.NumReadBuffBytes;
            }
        }
    
        //
        // Load the entire read buffer with more data.
        //
    
        // Determine next grouping of clusters that are contiguous (on the storage device).
        if (g_FileInfo.NumContigClusters == 0)
        {
            // If the current cluster isn't the first cluster in the file, then the value is +1 from the last
            // cluster in the contiguous grouping.  Subtract one, follow the link and seek a new grouping.
            //
            if (g_FileInfo.CurCluster != g_FileInfo.FirstCluster)
            {
                --g_FileInfo.CurCluster;
    //SERPRINT("Checking cluster: 0x%x\r\n", g_FileInfo.CurCluster);
                if (IsEOFCluster(g_FileInfo.CurCluster))
                {
                    ERRMSG(MSG_EOF_CLTR_IN_FAT_CHAIN, ("Reached EOF cluster in FAT file chain\n"));
                    return(FALSE);
                }
                g_FileInfo.CurCluster = GetNextCluster(g_FileInfo.CurCluster);
    //SERPRINT("Next cluster: 0x%x\r\n", g_FileInfo.CurCluster);
            }

            // Get the next contiguous grouping of cluster and return the type of the last cluster in the list.
            switch(GetNextClusterGroup(g_FileInfo.CurCluster, &g_FileInfo.NumContigClusters))
            {
            case DATA_CLUSTER:
                break;

            case EOF_CLUSTER:
                break;

            case RSVD_CLUSTER:
                WARNMSG(MSG_RESERVED_CLTR_IN_CHAIN, ("Found reserved cluster in FAT file chain\n"));
                return FALSE;
                break;

            case BAD_CLUSTER:
                WARNMSG(MSG_BAD_CLTR_IN_CHAIN, ("Found bad cluster in FAT file chain\n"));
                return FALSE;
                
            default:
                WARNMSG(MSG_CANNOT_GET_NEXT_CLTR, ("Cannot find contiguous cluster grouping (start cluster = 0x%x)\n", g_FileInfo.CurCluster));
                return FALSE; 
            }


            // We are pointing at new cluster, need this info for large cluster mode
            g_SectorsInCurrentClusterLeftToRead = pBPB->SectsPerClust;
        }

        // Get logical block of start of the current cluster
        CurrentLBA = Cluster2LBA(g_FileInfo.CurCluster);
        
        if(g_DiskReadMode == READ_BY_SECTOR)
        {
            // We want a full buffer read or just enough sectors to finish off the cluster
            ClustersToRead = 0;
            if(g_SectorsInCurrentClusterLeftToRead > g_ReadBuffLenInSectors)
            {
                SectorsToRead = (USHORT)(g_ReadBuffLenInSectors);
            }
            else
            {
                SectorsToRead = (USHORT)(g_SectorsInCurrentClusterLeftToRead);
            }

            // Adjust current logical block due to sectors we read already
            CurrentLBA += (pBPB->SectsPerClust - g_SectorsInCurrentClusterLeftToRead);
        }
        else
        {
            // Multiple clusters can fit in the buffer
            // Determine the number of clusters we can read at this time.
            //
            ClustersToRead = MIN(g_ReadBuffLenInClusters, g_FileInfo.NumContigClusters);
            SectorsToRead = (USHORT)(ClustersToRead * pBPB->SectsPerClust);
        }
    
        // Read sectors/clusters from storage.
        //
        if (!ReadSectors(pBPB->DriveId, CurrentLBA , SectorsToRead, g_pReadBuffStart))
        {
            ERRMSG(MSG_CANNOT_READ_DATA, ("Failed to read data from storage (start LBA = 0x%x, no of sectors = 0x%x)\n", CurrentLBA, SectorsToRead));
            return(FALSE);
        }

        // Update read management data.
        //
        if(g_DiskReadMode == READ_BY_SECTOR)
        {
            // We have read some sectors
            g_SectorsInCurrentClusterLeftToRead -= SectorsToRead;

            // Have we finished reading a full cluster?
            if(!g_SectorsInCurrentClusterLeftToRead)
            {
                //Used to increment our current cluster pointers
                ClustersToRead = 1;

                // Reset number of sectors since we are pointing to new cluster
                g_SectorsInCurrentClusterLeftToRead = pBPB->SectsPerClust;
            }
        }

        g_FileInfo.NumContigClusters    -=  ClustersToRead;
        g_FileInfo.CurCluster           +=  ClustersToRead;
        g_FileInfo.pCurReadBuffAddr     =   g_pReadBuffStart;
        if(g_DiskReadMode == READ_BY_SECTOR)
        {
            g_FileInfo.NumReadBuffBytes = SectorsToRead * pBPB->BytesPerSect;
        }
        else // g_DiskReadMode == READ_BY_CLUSTER
        {
            g_FileInfo.NumReadBuffBytes = ClustersToRead * pBPB->SectsPerClust * pBPB->BytesPerSect;
        }

/*
        if (bMonitorLoadProgress)
        {
            dwBytesLoaded += g_FileInfo.NumReadBuffBytes;
            SetProgressValue(dwBytesLoaded/(dwBytesToLoad/1000));
        }
*/
    
    }
    while (Length);

    return(TRUE);
}

//
// We assume that the caller does not attempt to rewind
// more that it has read so far
//
BOOL FSRewind(ULONG Offset)
{
    g_FileInfo.pCurReadBuffAddr -= Offset;
    g_FileInfo.NumReadBuffBytes += Offset;

    return TRUE;
}
