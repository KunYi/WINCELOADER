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
#include <boot.h>
#include <bootCore.h>

#include "exfat.h"
#include "exfat_funcs.h"
#include "dbgmsg.h"

#include "fs.h"

DWORD dwZoneMask;

static BOOL ExFAT_InitReadBuffer(ULONG StartAddress, ULONG Length);
ULONG ExFAT_GetNextCluster(ULONG Cluster);

PBIOSPBEX pExFAT_BPB = NULL;


static PUCHAR g_pReadBuffStart = 0;
static ULONG g_ReadBuffLenInClusters = 0;
static ULONG g_ReadBuffLenInSectors = 0;

static READ_MODE g_DiskReadMode = READ_BY_CLUSTER;
static ULONG g_SectorsInCurrentClusterLeftToRead = 0;

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
} g_FATParms;

BOOL ExFAT_IsDataCluster(ULONG Cluster)
{
    if (Cluster >= 0x00000002 && Cluster <= 0x0fffffef)
        return(TRUE);

    return(FALSE);
}


BOOL ExFAT_IsRsvdCluster(ULONG Cluster)
{
    if (Cluster >= 0x0ffffff0 && Cluster <= 0x0ffffff6)
        return(TRUE);

    return(FALSE);
}


BOOL ExFAT_IsEOFCluster(ULONG Cluster)
{
    if (Cluster == 0x0fffffff)
        return(TRUE);

    return(FALSE);
}

ULONG ExFAT_Cluster2LBA(ULONG Cluster)
{
    return(g_FATParms.DataStartLBA + ((Cluster - 2) << pExFAT_BPB->SectorsPerClusterLog2));
}

ULONG ExFAT_LBA2Cluster(ULONG LBA)
{
    return(((LBA-g_FATParms.DataStartLBA)>>pExFAT_BPB->SectorsPerClusterLog2) + 2);
}

ULONG ExFAT_NextSector(ULONG Sector)
{
    Sector += 1;

    //
    // This may look confusing, but it saves us 16 bytes over a % operation.
    //      if( Sector % ( 1 << pBPB->SectorsPerCluster) == 0 )
    // Just checking to see if we've used all the sectors in the current cluster.
    //
    if( (Sector & ((1 << pExFAT_BPB->SectorsPerClusterLog2)-1)) == 0 )
    {
        ULONG Cluster = LBA2Cluster( Sector - 1 );
        ULONG NewCluster = GetNextCluster( Cluster );
        if( IsDataCluster( NewCluster ) && Cluster != NewCluster )
        {
            return Cluster2LBA( NewCluster );
        }

        Sector = 0;
    }

    return Sector;
}

//
// Initialize the FAT subsystem.
//
BOOL ExFAT_FSInit(void)
{
    UCHAR FATType[8];

    //
    // BIOS parameter block was left behind by the boot sector code - use it.
    // Note: the address is determined by the master boot record (MBR) - this is the location (-3 bytes) where it
    // loads the boot sector and the sector contains the BIOS parameter block.
    //
    pExFAT_BPB = (PBIOSPBEX)0xFA03;

    // Compute LBA for various disk regions.

    g_FATParms.RootDirLBA   = Cluster2LBA( pExFAT_BPB->FirstClusterInRoot );
    // TODO::Eventually we should make this into a ULONGLONG instead of typecasting to a ULONG.
    g_FATParms.FATLBA = pExFAT_BPB->FATOffset + (ULONG)pExFAT_BPB->PartitionOffset;
    g_FATParms.DataStartLBA = ?? 
    //
    // For TexFAT we will use whichever FAT they want us to use.
    //
    if( (pExFAT_BPB->ExtFlags & 0x1) && (pExFAT_BPB->NumberOfFATs == 2) )
    {
        g_FATParms.FATLBA += pExFAT_BPB->FATLength;
    }

    // Determine FAT filesystem type.
    memcpy(FATType, pExFAT_BPB->VersionId, 8);

    if (memcmp(FATType, "EXFAT   ", 8))
    {
        ERRMSG(MSG_UNKNOWN_FILE_SYSTEM, ("Unknown file system: '%s'\n", FATType));
        return(FALSE);
    }

    //
    // Initialize the read buffer.
    //
    if (!ExFAT_InitReadBuffer(READ_BUFFER_START, READ_BUFFER_LENGTH_MAX))
    {
        ERRMSG(MSG_CANNOT_INIT_READ_BUFFER, ("Failed to initialize read buffer\n"));
        return(FALSE);
    }

    return(TRUE);
}

BOOL FSPreHook(void)
{
	return TRUE;
}

//
// Open filename specified and return the size.
//
ULONG ExFAT_FSOpenFile(char* pFileName)
{
    USHORT i = 0;
    USHORT j = 0;
    ULONG DirLBA = 1;
    CHAR* Sector = (CHAR*)READ_BUFFER_START;
    CHAR FileName[30] = { 0 };
    BOOL bFound = FALSE;

    // Check caller's parameters.
    if (pFileName == NULL)
        return(0);

    memset( &g_FileInfo, 0, sizeof(FILEINFO) );

    // Convert the filename to the form stored on disk.
    for (i = 0 ; i < 15 && *(pFileName + i) ; i++)
        FileName[i*2] = TO_UPPER(*(pFileName + i));

    // Look for the filename in directory list.
    for( bFound = FALSE, DirLBA = g_FATParms.RootDirLBA ;
         !bFound && DirLBA ;
         DirLBA = NextSector( DirLBA ) )
    {
        // Read a sector from the root directory.
        if (!ReadSectors(pExFAT_BPB->DriveID, DirLBA, 1, Sector))
        {
            ERRMSG(MSG_CANNOT_READ_ROOT_DIR, ("Couldn't read root directory sector (LBA=0x%x)\n", DirLBA));
            return(0);
        }

        // Try to find the specified file in the root directory...
        for (j = 0 ; (j < (1 << (pExFAT_BPB->BytesPerSector - 5))) && !bFound; j++)
        {
            BYTE* pCurrent = Sector + (j << 5);

            //
            // We've found a stream entry, record this in case it's our file.
            //
            if( ((pCurrent[0] & 0x7F) == 64) && (pCurrent[0] & 0x80) )
            {
                EXFAT_STREAM_RECORD* pEntry;
                pEntry = (EXFAT_STREAM_RECORD*)(pCurrent);

                g_FileInfo.Flags = pEntry->Flags;
                g_FileInfo.FileSize = (ULONG)pEntry->FileSize;
                g_FileInfo.FirstCluster = pEntry->FirstCluster;
                g_FileInfo.CurCluster = g_FileInfo.FirstCluster;
                g_FileInfo.pCurReadBuffAddr = g_pReadBuffStart;

            }
            else if( ((pCurrent[0] & 0x7F) == 65) && (pCurrent[0] & 0x80) )
            {
                BYTE Current = 0;
                bFound = TRUE;


                for (i = 0 ; i < 30 ; i++)
                {
                    Current = TO_UPPER( (pCurrent)[2 + i] );
                    if( FileName[i] != Current )
                    {
                        bFound = FALSE;
                    }
                }
            }
        }
    }

    if (!bFound)
    {
        WARNMSG(MSG_FILE_NOT_FOUND, ("File '%s' not found\n", pFileName));
        return(0);
    }
    else
    {
        INFOMSG(MSG_FILE_FOUND, ("Found file '%s' (start=0x%x size=0x%x)\n", pFileName, g_FileInfo.FirstCluster, g_FileInfo.FileSize));
    }

    return(g_FileInfo.FileSize);
}



void ExFAT_FSCloseFile(void)
{
    memset(&g_FileInfo, 0, sizeof(g_FileInfo));

    return;
}


static BOOL ExFAT_InitReadBuffer(ULONG StartAddress, ULONG Length)
{
    USHORT BytesPerSector = 1 << pExFAT_BPB->BytesPerSectorLog2;
    USHORT SectorsPerCluster = 1 << pExFAT_BPB->SectorsPerClusterLog2;
    // Check arguments.
    //
    if (StartAddress == 0)
        return(FALSE);

    // Determine how many clusters are in the read buffer.
    //
    g_ReadBuffLenInClusters = Length / (BytesPerSector * SectorsPerCluster);

    // Determine how many sectors are in the read buffer.
    //
    g_ReadBuffLenInSectors = Length / BytesPerSector;


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


ULONG ExFAT_GetNextCluster(ULONG Cluster)
{
    ULONG Sector = 0;
    ULONG ByteOffset = 0;
    PUCHAR pSectorCache = (PUCHAR)SECTOR_CACHE_START;   // Sector cache is where the sector used to read the FAT cluster chains lives.
    static ULONG CurrentSector = 0;
    ULONG NextCluster = 0;

    // If we're passed an EOF cluster, return it.
    //
    if (ExFAT_IsEOFCluster(Cluster))
        return(Cluster);

    // Is caller giving us a valid cluster?
    //
    if (!ExFAT_IsDataCluster(Cluster))
    {
        ERRMSG(MSG_BAD_CLUSTER_NUMBER, ("Bad cluster number\n"));
        return(0);  // 0 isn't a valid cluster number (at least for our purposes).
    }

    // Compute sector where our FAT entry lives.
    //
    Sector = Cluster << 2;
    ByteOffset = Sector & ((1 << pExFAT_BPB->BytesPerSectorLog2)-1);
    Sector = Sector >> pExFAT_BPB->BytesPerSectorLog2;
    Sector += g_FATParms.FATLBA;

    // If the sector we're interested in isn't in our cache, get it.
    //
    if (CurrentSector != Sector)
    {
        if (!ReadSectors( pExFAT_BPB->DriveID, Sector, 1, pSectorCache))
        {
//          TODO: Only a message?
//          SERPRINT("GetNextCluster - unable to read sector.\r\n");
        }

        CurrentSector = Sector;
    }

    // Locate next cluster number...
    //
    NextCluster = *(PULONG)(pSectorCache + ByteOffset);

//    SERPRINT("GNC: cluster=0x%x  next cluster=0x%x\r\n", Cluster, NextCluster);

    // Return the next cluster value.
    //
    return(NextCluster);
}


CLUSTTYPE ExFAT_GetNextClusterGroup(ULONG StartCluster, PULONG pNumberClusters)
{
    ULONG CurrentCluster = 0;
    ULONG NextCluster = 0;

    if( g_FileInfo.Flags & FF_CONTIGOUS )
    {
        ULONG BytesPerCluster = (1 << ( pExFAT_BPB->BytesPerSectorLog2 + pExFAT_BPB->SectorsPerClusterLog2 )) - 1;

        *pNumberClusters =
            g_FileInfo.FileSize >> (pExFAT_BPB->BytesPerSectorLog2 + pExFAT_BPB->SectorsPerClusterLog2);

        if( g_FileInfo.FileSize & BytesPerCluster )
        {
            *pNumberClusters += 1;
        }

        return EOF_CLUSTER;
    }

    *pNumberClusters = 0;

    // TODO - could be a bad cluster - check for it here.

    // Determine number of contiguous clusters.
    //
    NextCluster = StartCluster;
    do
    {
        CurrentCluster = NextCluster;
        *pNumberClusters = *pNumberClusters + 1;
        NextCluster = ExFAT_GetNextCluster(CurrentCluster);
        if (NextCluster != (CurrentCluster + 1))
            break;
    } while(ExFAT_IsDataCluster(NextCluster));

    // Why did we break out of the above loop?
    //
    if (ExFAT_IsEOFCluster(NextCluster))
        return(EOF_CLUSTER);
    if (ExFAT_IsRsvdCluster(NextCluster))
        return(RSVD_CLUSTER);

    return(DATA_CLUSTER);
}


BOOL ExFAT_FSReadFile(unsigned char * pAddress, ULONG Length)
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
                if (IsEOFCluster(g_FileInfo.CurCluster))
                {
                    ERRMSG(MSG_EOF_CLTR_IN_FAT_CHAIN, ("Reached EOF cluster in FAT file chain\n"));
                    return(FALSE);
                }
                g_FileInfo.CurCluster = GetNextCluster(g_FileInfo.CurCluster);
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
            g_SectorsInCurrentClusterLeftToRead = (1 << pBPB->SectorsPerCluster);
        }


        // Get logical block of start of the current cluster
        CurrentLBA = ExFAT_Cluster2LBA(g_FileInfo.CurCluster);

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
            CurrentLBA += ((1 << pExFAT_BPB->SectorsPerClusterLog2) - g_SectorsInCurrentClusterLeftToRead);
        }
        else
        {
            // Multiple clusters can fit in the buffer
            // Determine the number of clusters we can read at this time.
            //
            ClustersToRead = MIN(g_ReadBuffLenInClusters, g_FileInfo.NumContigClusters);
            SectorsToRead = (USHORT)(ClustersToRead * (1 << pExFAT_BPB->SectorsPerClusterLog2));
        }

        // Read sectors/clusters from storage.
        //
        if (!ReadSectors(pExFAT_BPB->DriveID, CurrentLBA , SectorsToRead, g_pReadBuffStart))
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
                g_SectorsInCurrentClusterLeftToRead = (1 << pExFAT_BPB->SectorsPerClusterLog2);
            }
        }

        g_FileInfo.NumContigClusters    -=  ClustersToRead;
        g_FileInfo.CurCluster           +=  ClustersToRead;
        g_FileInfo.pCurReadBuffAddr     =   g_pReadBuffStart;
        if(g_DiskReadMode == READ_BY_SECTOR)
        {
            g_FileInfo.NumReadBuffBytes = SectorsToRead * (1 << pExFAT_BPB->BytesPerSectorLog2);
        }
        else // g_DiskReadMode == READ_BY_CLUSTER
        {
            g_FileInfo.NumReadBuffBytes = ClustersToRead *
                                          (1 << pExFAT_BPB->SectorsPerClusterLog2) *
                                          (1 << pExFAT_BPB->BytesPerSectorLog2);
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
BOOL ExFAT_FSRewind(ULONG Offset)
{
    g_FileInfo.pCurReadBuffAddr -= Offset;
    g_FileInfo.NumReadBuffBytes += Offset;

    return TRUE;
}
