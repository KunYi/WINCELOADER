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
#include <boot.h>
#include <bootCore.h>

#include "fat32.h"
#include "fat32_funcs.h"
#include "fs.h"

static BOOL FAT32_InitReadBuffer(ULONG StartAddress, ULONG Length);
ULONG FAT32_GetNextCluster(ULONG Cluster);

PBIOSPB32 pFAT32_BPB = NULL;
	
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
    ULONG       FATLBA;				// LBA start of FAT (first)
    ULONG       RootDirLBA;			// LBA start of root directory
    ULONG       DataStartLBA;		// LBA start of data area
} g_FATParms;

BOOL FAT32_IsDataCluster(ULONG Cluster)
{
	Cluster &= 0x0fffffff;
	if (Cluster >= 0x00000002 && Cluster <= 0x0fffffef)
		return(TRUE);

    return(FALSE);
}


BOOL FAT32_IsRsvdCluster(ULONG Cluster)
{
	Cluster &= 0x0fffffff;
	if (Cluster >= 0x0ffffff0 && Cluster <= 0x0ffffff6)
		return(TRUE);

    return(FALSE);
}


BOOL FAT32_IsEOFCluster(ULONG Cluster)
{
	Cluster &= 0x0fffffff;
	if (Cluster >= 0x0ffffff8 && Cluster <= 0x0fffffff)
		return(TRUE);

    return(FALSE);
}


BOOL FAT32_IsBadCluster(ULONG Cluster)
{
	Cluster &= 0x0fffffff;
	if (Cluster == 0x0ffffff7)
		return(TRUE);

    return(FALSE);
}

ULONG FAT32_Cluster2LBA(ULONG Cluster)
{
    return(g_FATParms.DataStartLBA + (Cluster - 2) * pFAT32_BPB->SectsPerClust);
}

ULONG FAT32_LBA2Cluster(ULONG LBA)
{
    return(((LBA-g_FATParms.DataStartLBA) / pFAT32_BPB->SectsPerClust) + 2);
}

ULONG FAT32_NextSector(ULONG Sector)
{
    Sector += 1;

    //
    // Just checking to see if we've used all the sectors in the current cluster.
    //
//(db) Found a bug. If the data start LBA isn't divisble by cluster size, you can fail this
// Need to subtract the starting LBA from sector to get sector offset within volume.
//(db) if( (Sector % pFAT32_BPB->SectsPerClust) == 0 )   
	if( ((Sector - g_FATParms.DataStartLBA) % pFAT32_BPB->SectsPerClust) == 0 )
	
    {
        ULONG Cluster = FAT32_LBA2Cluster( Sector - 1 );
        ULONG NewCluster = FAT32_GetNextCluster( Cluster );
        if( FAT32_IsDataCluster( NewCluster ) && Cluster != NewCluster )
        {
            return FAT32_Cluster2LBA( NewCluster );
        }

        Sector = 0;
    }

    return Sector;
}

//
// Initialize the FAT subsystem.
//
BOOL FAT32_FSInit(void)
{
    ULONG ReadBufferLen = 0;
//    ULONG Cluster;
//	UCHAR FATType[9];

	// 
	// BIOS parameter block was left behind by the boot sector code - use it.
	// Note: the address is determined by the master boot record (MBR) - this is the location (-3 bytes) where it
	// loads the boot sector and the sector contains the BIOS parameter block.
	//
    pFAT32_BPB = (PBIOSPB32)0xFA03;

    g_FATParms.FATLBA = pFAT32_BPB->RsvdSects + pFAT32_BPB->NumHiddenSectL + (pFAT32_BPB->NumHiddenSectH << 16);
	g_FATParms.DataStartLBA = g_FATParms.FATLBA + (pFAT32_BPB->NumFATs * (pFAT32_BPB->SectsPerFATL + (pFAT32_BPB->SectsPerFATH << 16)));
    g_FATParms.RootDirLBA   = FAT32_Cluster2LBA( pFAT32_BPB->RootClusterL + (pFAT32_BPB->RootClusterH << 16) );


//#if 0	
	DEBUGMSG(ZONE_INFO, (L"\r\nDrive Info:\r\n"));
    DEBUGMSG(ZONE_INFO, (L" - Drive ID ................... 0x%x\r\n", pFAT32_BPB->DriveId));
    DEBUGMSG(ZONE_INFO, (L" - Sector Size ...............  0x%x\r\n\r\n", pFAT32_BPB->BytesPerSect));
    DEBUGMSG(ZONE_INFO, (L" - Heads ...................... 0x%x\r\n", pFAT32_BPB->NumHeads));
    DEBUGMSG(ZONE_INFO, (L" - Number of Sectors Per Track  0x%x\r\n", pFAT32_BPB->SectsPerTrack));

	DEBUGMSG(ZONE_INFO, (L"FAT Info:\r\n"));
    DEBUGMSG(ZONE_INFO, (L" - Cluster Size ............... 0x%x\r\n", (pFAT32_BPB->SectsPerClust * pFAT32_BPB->BytesPerSect)));
    DEBUGMSG(ZONE_INFO, (L" - Number of FATs ............. 0x%x\r\n", pFAT32_BPB->NumFATs));
    DEBUGMSG(ZONE_INFO, (L" - Number of Sectors Per FAT .. 0x%x\r\n", pFAT32_BPB->SectsPerFAT));
    DEBUGMSG(ZONE_INFO, (L" - Number of Hidden Sectors ... 0x%x%x\r\n", pFAT32_BPB->NumHiddenSectH, pFAT32_BPB->NumHiddenSectL));
    DEBUGMSG(ZONE_INFO, (L" - Number of Reserved Sectors . 0x%x   %d\r\n\r\n", pFAT32_BPB->RsvdSects, pFAT32_BPB->RsvdSects));
    DEBUGMSG(ZONE_INFO, (L" - Root dir location (LBA) .... 0x%x   %d\r\n", g_FATParms.RootDirLBA, g_FATParms.RootDirLBA));
    DEBUGMSG(ZONE_INFO, (L" - FAT location (LBA) ......... 0x%x   %d\r\n", g_FATParms.FATLBA, g_FATParms.FATLBA));
    DEBUGMSG(ZONE_INFO, (L" - Data location (LBA) ........ 0x%x   %d\r\n\r\n", g_FATParms.DataStartLBA, g_FATParms.DataStartLBA));
    DEBUGMSG(1, (L" - Data location (LBA) ........ 0x%x   %d\r\n\r\n", g_FATParms.DataStartLBA, g_FATParms.DataStartLBA));
//#endif
	BootLog (L"\r\nDrive Info:\r\n");
    BootLog (L" - Drive ID ................... 0x%x\r\n", pFAT32_BPB->DriveId);
    BootLog (L" - Sector Size ...............  0x%x\r\n\r\n", pFAT32_BPB->BytesPerSect);
    BootLog (L" - Heads ...................... 0x%x\r\n", pFAT32_BPB->NumHeads);
    BootLog (L" - Number of Sectors Per Track  0x%x\r\n", pFAT32_BPB->SectsPerTrack);

	BootLog (L"FAT Info:\r\n");
    BootLog (L" - Cluster Size ............... 0x%x\r\n", (pFAT32_BPB->SectsPerClust * pFAT32_BPB->BytesPerSect));
    BootLog (L" - Number of FATs ............. 0x%x\r\n", pFAT32_BPB->NumFATs);
    BootLog (L" - Number of Sectors Per FAT .. 0x%x\r\n", pFAT32_BPB->SectsPerFAT);
    BootLog (L" - Number of Hidden Sectors ... 0x%x%x\r\n", pFAT32_BPB->NumHiddenSectH, pFAT32_BPB->NumHiddenSectL);
    BootLog (L" - Number of Reserved Sectors . 0x%x   %d\r\n\r\n", pFAT32_BPB->RsvdSects, pFAT32_BPB->RsvdSects);
    BootLog (L" - Root dir location (LBA) .... 0x%x   %d\r\n", g_FATParms.RootDirLBA, g_FATParms.RootDirLBA);
    BootLog (L" - FAT location (LBA) ......... 0x%x   %d\r\n", g_FATParms.FATLBA, g_FATParms.FATLBA);
    BootLog (L" - Data location (LBA) ........ 0x%x   %d\r\n\r\n", g_FATParms.DataStartLBA, g_FATParms.DataStartLBA);

	ReadBufferLen = READ_BUFFER_LENGTH_MAX;

	//
	// Initialize the read buffer.
	//
	if (!FAT32_InitReadBuffer(READ_BUFFER_START, ReadBufferLen))
	{
		DEBUGMSG(ZONE_ERROR, (L"FAT32:: Failed to initialize read buffer\r\n"));
		return(FALSE);
	}

    return(TRUE); 
}

//
// Open filename specified and return the size.
//
ULONG FAT32_FSOpenFile(char* pFileName)
{
    USHORT i = 0;
    USHORT j = 0;
    BOOL bFound = FALSE;
	ULONG DirLBA = 0;
    ULONG PreviousLBA = 0;
    CHAR* Sector = (CHAR*)READ_BUFFER_START;
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
        i++;
        for (j=0 ; i < 12 && *(pFileName + i) != '\0' ; i++, j++)
            FileName[8 + j] = TO_UPPER(*(pFileName + i));
    }

    // Look for the filename in directory list.
    for( DirLBA = g_FATParms.RootDirLBA; !bFound && DirLBA ; DirLBA = FAT32_NextSector( DirLBA ) )
    {
        //BootLog(L"Reading root directory sector (LBA=0x%x)...\r\n", DirLBA);

		// Read a sector from the root directory.
        if (!ReadSectors(pFAT32_BPB->DriveId, DirLBA, 1, Sector))
        {
            DEBUGMSG(ZONE_ERROR, (L"Couldn't read root directory sector (LBA=0x%x)\r\n", DirLBA));
            return(0);
        }

		// Try to find the specified file in the root directory...
        for (pDirEntry = (PDIRENTRY)Sector, j = 0 ; j < (pFAT32_BPB->BytesPerSect / sizeof(DIRENTRY)) ; j++, pDirEntry++)
        {
            if (!memcmp(FileName, pDirEntry->FileName, 11))
            {
				// Found it.
                bFound = TRUE;
                break;
            }
        }
        if( bFound )
        {
            //
            // If we don't break here then NextSector will run and, through 
            // a call to ReadSectors, overwrite our buffer and we'll return an
            // invalid size and sector to read from.
            //
            break;
        }
    } 

    if (!bFound || pDirEntry == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"File '%S' not found\r\n", pFileName)); 
        return(0);
    }
    else
    {
        DEBUGMSG(ZONE_INFO, (L"Found file '%S' (start=0x%x size=0x%x)\r\n", pFileName, pDirEntry->FirstClust, pDirEntry->FileSize)); 
    }
   
	//
    // Save file parameters
	//
    g_FileInfo.FileSize            = pDirEntry->FileSize;
	g_FileInfo.FirstCluster        = pDirEntry->FirstClust;
	g_FileInfo.CurCluster          = pDirEntry->FirstClust;
	g_FileInfo.NumContigClusters   = 0;
    g_FileInfo.pCurReadBuffAddr    = g_pReadBuffStart;
	g_FileInfo.NumReadBuffBytes    = 0;

/*
    if (bMonitorLoadProgress)
    {
        dwBytesToLoad = g_FileInfo.FileSize;
        dwBytesLoaded = 0;
        SetProgressValue(0);
    }
*/

    return(g_FileInfo.FileSize);
}



void FAT32_FSCloseFile(void)
{
    memset(&g_FileInfo, 0, sizeof(g_FileInfo));

    return;
}


static BOOL FAT32_InitReadBuffer(ULONG StartAddress, ULONG Length)
{
	// Check arguments.
	//
	if (StartAddress == 0)
		return(FALSE);

	// Determine how many clusters are in the read buffer.
	//
	g_ReadBuffLenInClusters = Length / (pFAT32_BPB->BytesPerSect * pFAT32_BPB->SectsPerClust);

	// Determine how many sectors are in the read buffer.
	//
	g_ReadBuffLenInSectors = Length / pFAT32_BPB->BytesPerSect ;


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

ULONG FAT32_GetNextCluster(ULONG Cluster)
{
	ULONG Sector = 0;
	ULONG ByteOffset = 0;
	PUCHAR pSectorCache = (PUCHAR)SECTOR_CACHE_START;	// Sector cache is where the sector used to read the FAT cluster chains lives.
	static ULONG CurrentSector = 0;
	ULONG NextCluster = 0;

	// If we're passed an EOF cluster, return it.
	//
	if (FAT32_IsEOFCluster(Cluster))
        return(Cluster);

	// Is caller giving us a valid cluster?
	//
    if (!FAT32_IsDataCluster(Cluster))
	{
        DEBUGMSG(ZONE_ERROR, (L"Bad cluster number\r\n"));
	    return(0);	// 0 isn't a valid cluster number (at least for our purposes).
	}

	// Compute sector where our FAT entry lives.
	//
    Sector = Cluster * sizeof(ULONG);
    ByteOffset = Sector & (pFAT32_BPB->BytesPerSect-1);
    Sector /= pFAT32_BPB->BytesPerSect;
    Sector += g_FATParms.FATLBA;

	// If the sector we're interested in isn't in our cache, get it.
	//
	if (CurrentSector != Sector)
	{
		if (!ReadSectors( pFAT32_BPB->DriveId, Sector, 1, pSectorCache))
		{
//			TODO: Only a message?
//			BootLog(L"GetNextCluster - unable to read sector.\r\n");
		}

		CurrentSector = Sector;
	}

	// Locate next cluster number...
	//
	NextCluster = *(PULONG)(pSectorCache + ByteOffset);

    //BootLog(L"GetNextCluster - cluster=0x%x  next cluster=0x%x.\r\n", Cluster, NextCluster);
	
    // Return the next cluster value.
	//
	return(NextCluster);
}


CLUSTTYPE FAT32_GetNextClusterGroup(ULONG StartCluster, PULONG pNumberClusters)
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
        NextCluster = FAT32_GetNextCluster(CurrentCluster);
		if (NextCluster != (CurrentCluster + 1))
			break;
	} while(FAT32_IsDataCluster(NextCluster));

	// Why did we break out of the above loop?
	//
	if (FAT32_IsEOFCluster(NextCluster))
		return(EOF_CLUSTER);
	if (FAT32_IsBadCluster(NextCluster))
		return(BAD_CLUSTER);
	if (FAT32_IsRsvdCluster(NextCluster))
		return(RSVD_CLUSTER);

	return(DATA_CLUSTER);
}


BOOL FAT32_FSReadFile( unsigned char * pAddress, ULONG Length)
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
		DEBUGMSG(ZONE_ERROR, (L"FAT must be initialized before reading files\r\n"));
		return(FALSE);
	}

    //BootLog(L"Length: 0x%x\r\n", Length);
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
    //BootLog(L"Checking cluster: 0x%x\r\n", g_FileInfo.CurCluster);
				if (FAT32_IsEOFCluster(g_FileInfo.CurCluster))
				{
					DEBUGMSG(ZONE_ERROR, (L"Reached EOF cluster in FAT file chain\r\n"));
					return(FALSE);
				}
				g_FileInfo.CurCluster = FAT32_GetNextCluster(g_FileInfo.CurCluster);
    //BootLog(L"Next cluster: 0x%x\r\n", g_FileInfo.CurCluster);
			}

			// Get the next contiguous grouping of cluster and return the type of the last cluster in the list.
			switch(FAT32_GetNextClusterGroup(g_FileInfo.CurCluster, &g_FileInfo.NumContigClusters))
			{
			case DATA_CLUSTER:
				break;

			case EOF_CLUSTER:
				break;

			case RSVD_CLUSTER:
				DEBUGMSG(ZONE_WARN, (L"Found reserved cluster in FAT file chain\r\n"));
				return FALSE;
				break;

			case BAD_CLUSTER:
				DEBUGMSG(ZONE_WARN, (L"Found bad cluster in FAT file chain\r\n"));
				return FALSE;
                
			default:
				DEBUGMSG(ZONE_WARN, (L"Cannot find contiguous cluster grouping (start cluster = 0x%x)\r\n", g_FileInfo.CurCluster));
				return FALSE; 
			}


			// We are pointing at new cluster, need this info for large cluster mode
			g_SectorsInCurrentClusterLeftToRead = pFAT32_BPB->SectsPerClust;
		}


		// Get logical block of start of the current cluster
		CurrentLBA = FAT32_Cluster2LBA(g_FileInfo.CurCluster);
		
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
			CurrentLBA += (pFAT32_BPB->SectsPerClust - g_SectorsInCurrentClusterLeftToRead);
		}
		else
		{
			// Multiple clusters can fit in the buffer
			// Determine the number of clusters we can read at this time.
			//
			ClustersToRead = MIN(g_ReadBuffLenInClusters, g_FileInfo.NumContigClusters);
			SectorsToRead = (USHORT)(ClustersToRead * pFAT32_BPB->SectsPerClust);
		}
    
		// Read sectors/clusters from storage.
		//
		if (!ReadSectors(pFAT32_BPB->DriveId, CurrentLBA , SectorsToRead, g_pReadBuffStart))
		{
			DEBUGMSG(ZONE_ERROR, (L"Failed to read data from storage (start LBA = 0x%x, no of sectors = 0x%x)\r\n", CurrentLBA, SectorsToRead));
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
				g_SectorsInCurrentClusterLeftToRead = pFAT32_BPB->SectsPerClust;
			}
		}

		g_FileInfo.NumContigClusters	-=	ClustersToRead;
		g_FileInfo.CurCluster			+=	ClustersToRead;
		g_FileInfo.pCurReadBuffAddr		=	g_pReadBuffStart;
		if(g_DiskReadMode == READ_BY_SECTOR)
		{
			g_FileInfo.NumReadBuffBytes = SectorsToRead * pFAT32_BPB->BytesPerSect;
		}
		else // g_DiskReadMode == READ_BY_CLUSTER
		{
			g_FileInfo.NumReadBuffBytes = ClustersToRead * pFAT32_BPB->SectsPerClust * pFAT32_BPB->BytesPerSect;
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
BOOL FAT32_FSRewind(ULONG Offset)
{
    g_FileInfo.pCurReadBuffAddr -= Offset;
	g_FileInfo.NumReadBuffBytes += Offset;

    return TRUE;
}
