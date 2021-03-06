
#include <boot.h>
#include <bootCore.h>

#include "fat.h"
#include "fat12_funcs.h"
#include "dbgmsg.h"

#include "fs.h"

static BOOL FAT12_InitReadBuffer(ULONG StartAddress, ULONG Length);

PBIOSPB pFAT12_BPB = NULL;

static PUCHAR g_pReadBuffStart = 0;
static ULONG g_ReadBuffLenInClusters = 0;
static ULONG g_ReadBuffLenInSectors = 0;

static READ_MODE g_DiskReadMode = READ_BY_CLUSTER;
static ULONG g_SectorsInCurrentClusterLeftToRead = 0;

// Used by the progress bar stuff
BOOL bMonitorLoadProgress;
static DWORD dwBytesToLoad;
static DWORD dwBytesLoaded;

//
// File information
//
static FILEINFO g_FileInfo;

// This data supplements the BPB information - used to save time.
struct
{
    FAT_TYPE    FATType;            // FAT filesystem type (12, 16, or 32).
    ULONG       FATLBA;             // LBA start of FAT (first)
    ULONG       RootDirLBA;         // LBA start of root directory
    ULONG       DataStartLBA;       // LBA start of data area
} g_FATParms;

//
// Initialize the FAT subsystem.
//
BOOL FAT12_FSInit(void)
{
    ULONG ReadBufferLen = 0;
    UCHAR FATType[9];

    //
    // BIOS parameter block was left behind by the boot sector code - use it.
    // Note: the address is determined by the master boot record (MBR) - this is the location (-3 bytes) where it
    // loads the boot sector and the sector contains the BIOS parameter block.
    //
    pFAT12_BPB = (PBIOSPB)0xFA03;

    // Compute LBA for various disk regions.
    g_FATParms.DataStartLBA = DataStartLBA;
    g_FATParms.RootDirLBA   = g_FATParms.DataStartLBA - ((sizeof(DIRENTRY) * pFAT12_BPB->NumRootEntries) / pFAT12_BPB->BytesPerSect);
    if ((sizeof(DIRENTRY) * pFAT12_BPB->NumRootEntries) % pFAT12_BPB->BytesPerSect)
        --g_FATParms.RootDirLBA;
    g_FATParms.FATLBA       = g_FATParms.RootDirLBA - (pFAT12_BPB->NumFATs * pFAT12_BPB->SectsPerFAT);
    g_FATParms.DataStartLBA = g_FATParms.RootDirLBA + ((sizeof(DIRENTRY) * pFAT12_BPB->NumRootEntries) / pFAT12_BPB->BytesPerSect);


    // Determine FAT filesystem type.
    memcpy(FATType, pFAT12_BPB->TypeFAT, 8);
    FATType[8] = '\0';

    if (memcmp(FATType, "FAT12", 5))
    {
        ERRMSG(MSG_UNKNOWN_FILE_SYSTEM, ("Unknown file system: %s\n", FATType));
        return(FALSE);
    }

#if 0
    SERPRINT("\r\nDrive Info:\r\n");
    SERPRINT(" - Drive ID ................... 0x%x\r\n", pFAT12_BPB->DriveId);
    SERPRINT(" - Sector Size ...............  0x%x\r\n\r\n", pFAT12_BPB->BytesPerSect);
    SERPRINT(" - Heads ...................... 0x%x\r\n", pFAT12_BPB->NumHeads);
    SERPRINT(" - Number of Sectors Per Track  0x%x\r\n", pFAT12_BPB->SectsPerTrack);

    SERPRINT("FAT Info:\r\n");
    SERPRINT(" - FAT Type ................... %d\r\n", g_FATParms.FATType);
    SERPRINT(" - Cluster Size ............... 0x%x\r\n", (pFAT12_BPB->SectsPerClust * pFAT12_BPB->BytesPerSect));
    SERPRINT(" - Number of FATs ............. 0x%x\r\n", pFAT12_BPB->NumFATs);
    SERPRINT(" - Number of Sectors Per FAT .. 0x%x\r\n", pFAT12_BPB->SectsPerFAT);
    SERPRINT(" - Number of Hidden Sectors ... 0x%x%x\r\n", pFAT12_BPB->NumHiddenSectH, pFAT12_BPB->NumHiddenSectL);
    SERPRINT(" - Number of Reserved Sectors . 0x%x\r\n\r\n", pFAT12_BPB->RsvdSects);
    SERPRINT(" - Root dir location (LBA) .... 0x%x\r\n", g_FATParms.RootDirLBA);
    SERPRINT(" - FAT location (LBA) ......... 0x%x\r\n", g_FATParms.FATLBA);
    SERPRINT(" - Data location (LBA) ........ 0x%x\r\n\r\n", g_FATParms.DataStartLBA);
#endif

    // Some BIOS/floppy combinations don't support multi-sector reads.
    if(pFAT12_BPB->DriveId < 0x80)
    {
        ReadBufferLen = pFAT12_BPB->BytesPerSect;
    }
    else
    {
        ReadBufferLen = READ_BUFFER_LENGTH_MAX;
    }

    //
    // Initialize the read buffer.
    //
    if (!FAT12_InitReadBuffer(READ_BUFFER_START, ReadBufferLen))
    {
        ERRMSG(MSG_CANNOT_INIT_READ_BUFFER, ("Failed to initialize read buffer\n"));
        return(FALSE);
    }

    return(TRUE);
}


ULONG FAT12_Cluster2LBA(ULONG Cluster)
{
    return(g_FATParms.DataStartLBA + (Cluster - 2) * pFAT12_BPB->SectsPerClust);
}


//
// Open filename specified and return the size.
//
ULONG FAT12_FSOpenFile(char* pFileName)
{
    USHORT i = 0;
    USHORT j = 0;
    BOOL bFound = FALSE;
    ULONG DirLBA = 0;
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
    for (bFound = FALSE, i = 0, DirLBA = g_FATParms.RootDirLBA ; !bFound && i < pFAT12_BPB->NumRootEntries ; DirLBA++)
    {
        //SERPRINT("Reading root directory sector (LBA=0x%x)...\r\n", DirLBA);

        // Read a sector from the root directory.
        if (!ReadSectors(pFAT12_BPB->DriveId, DirLBA, 1, Sector))
        {
            ERRMSG(MSG_CANNOT_READ_ROOT_DIR, ("Couldn't read root directory sector (LBA=0x%x)\n", DirLBA));
            return(0);
        }

        // Try to find the specified file in the root directory...
        for (pDirEntry = (PDIRENTRY)Sector, j = 0 ; j < (pFAT12_BPB->BytesPerSect / sizeof(DIRENTRY)) && i < pFAT12_BPB->NumRootEntries ; j++, i++, pDirEntry++)
        {
            if (   !memcmp(FileName,     pDirEntry->FileName, sizeof(pDirEntry->FileName))
                && !memcmp(&FileName[8], pDirEntry->FileExt,  sizeof(pDirEntry->FileExt)))
            {
                // Found it.
                bFound = TRUE;
                break;
            }
        }
    }

    if (!bFound || pDirEntry == NULL)
    {
        WARNMSG(MSG_FILE_NOT_FOUND, ("File '%s' not found\n", pFileName));
        return(0);
    }
    else
    {
        INFOMSG(MSG_FILE_FOUND, ("Found file '%s' (start=0x%x size=0x%x)\n", pFileName, pDirEntry->FirstClust, pDirEntry->FileSize));
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



void FAT12_FSCloseFile(void)
{
    memset(&g_FileInfo, 0, sizeof(g_FileInfo));

    return;
}


static BOOL FAT12_InitReadBuffer(ULONG StartAddress, ULONG Length)
{
    // Check arguments.
    //
    if (StartAddress == 0)
        return(FALSE);

    // Determine how many clusters are in the read buffer.
    //
    g_ReadBuffLenInClusters = Length / (pFAT12_BPB->BytesPerSect * pFAT12_BPB->SectsPerClust);

    // Determine how many sectors are in the read buffer.
    //
    g_ReadBuffLenInSectors = Length / pFAT12_BPB->BytesPerSect ;


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

BOOL FAT12_IsDataCluster(ULONG Cluster)
{
    if (Cluster >= 0x002 && Cluster <= 0xfef)
        return(TRUE);

    return(FALSE);
}


BOOL FAT12_IsRsvdCluster(ULONG Cluster)
{
    if (Cluster >= 0xff0 && Cluster <= 0xff6)
        return(TRUE);

    return(FALSE);
}


BOOL FAT12_IsEOFCluster(ULONG Cluster)
{
    if (Cluster >= 0xff8 && Cluster <= 0xfff)
        return(TRUE);

    return(FALSE);
}


BOOL FAT12_IsBadCluster(ULONG Cluster)
{
    if (Cluster == 0xff7)
        return(TRUE);

    return(FALSE);
}


ULONG FAT12_GetNextCluster(ULONG Cluster)
{
    ULONG Sector = 0;
    ULONG ByteOffset = 0;
    PUCHAR pSectorCache = (PUCHAR)SECTOR_CACHE_START;   // Sector cache is where the sector used to read the FAT cluster chains lives.
    static ULONG CurrentSector = 0;
    ULONG NextCluster = 0;

    // If we're passed an EOF cluster, return it.
    //
    if (FAT12_IsEOFCluster(Cluster))
        return(Cluster);

    // Is caller giving us a valid cluster?
    //
    if (!FAT12_IsDataCluster(Cluster))
    {
        ERRMSG(MSG_BAD_CLUSTER_NUMBER, ("Bad cluster number\n"));
        return(0);  // 0 isn't a valid cluster number (at least for our purposes).
    }

    // Compute sector where our FAT entry lives.
    //
    Sector = (Cluster * 3) / 2;     // Every FAT12 cluster is 1.5 bytes.
    ByteOffset = Sector % pFAT12_BPB->BytesPerSect;
    Sector /= pFAT12_BPB->BytesPerSect;
    Sector += g_FATParms.FATLBA;

    // If the sector we're interested in isn't in our cache, get it.
    //
    if (CurrentSector != Sector)
    {
        if (!ReadSectors( pFAT12_BPB->DriveId, Sector, 1, pSectorCache))
        {
//          TODO: Only a message?
//          SERPRINT("FAT12_GetNextCluster - unable to read sector.\r\n");
        }

        CurrentSector = Sector;
    }

// Locate next cluster number...
//
    // Need to worry about cluster number crossing a sector boundary.
    if (ByteOffset == ((ULONG)pFAT12_BPB->BytesPerSect - 1))
    {
        NextCluster = (ULONG)(*(PUCHAR)(pSectorCache + ByteOffset));

        // Now we need to read the next sector.
        //
        ++Sector;
        if (!ReadSectors(pFAT12_BPB->DriveId, Sector, 1, pSectorCache))
        {
//          TODO: Only a message?
//          SERPRINT("FAT12_GetNextCluster - unable to read sector.\r\n");
        }
        CurrentSector = Sector;

        NextCluster |= (ULONG)((*(PUCHAR)pSectorCache) << 8);
    }
    else
        NextCluster = (ULONG)(*(PUSHORT)(pSectorCache + ByteOffset));

    // Since every FAT12 entry is 1.5 bytes, we either need to shift or mask based
    // on whether the previous cluster number was odd or even.
    //
    if (Cluster & 0x1)
        NextCluster = NextCluster >> 4;
    else
        NextCluster = NextCluster & 0xfff;

//    SERPRINT("FAT12_GetNextCluster - cluster=0x%x  next cluster=0x%x.\r\n", Cluster, NextCluster);

    // Return the next cluster value.
    //
    return(NextCluster);
}


CLUSTTYPE FAT12_GetNextClusterGroup(ULONG StartCluster, PULONG pNumberClusters)
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
        NextCluster = FAT12_GetNextCluster(CurrentCluster);
        if (NextCluster != (CurrentCluster + 1))
            break;
    } while(FAT12_IsDataCluster(NextCluster));

    // Why did we break out of the above loop?
    //
    if (FAT12_IsEOFCluster(NextCluster))
        return(EOF_CLUSTER);
    if (FAT12_IsBadCluster(NextCluster))
        return(BAD_CLUSTER);
    if (FAT12_IsRsvdCluster(NextCluster))
        return(RSVD_CLUSTER);

    return(DATA_CLUSTER);
}


BOOL FAT12_FSReadFile( unsigned char * pAddress, ULONG Length)
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
                if (FAT12_IsEOFCluster(g_FileInfo.CurCluster))
                {
                    ERRMSG(MSG_EOF_CLTR_IN_FAT_CHAIN, ("Reached EOF cluster in FAT file chain\n"));
                    return(FALSE);
                }
                g_FileInfo.CurCluster = FAT12_GetNextCluster(g_FileInfo.CurCluster);
            }

            // Get the next contiguous grouping of cluster and return the type of the last cluster in the list.
            switch(FAT12_GetNextClusterGroup(g_FileInfo.CurCluster, &g_FileInfo.NumContigClusters))
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
            g_SectorsInCurrentClusterLeftToRead = pFAT12_BPB->SectsPerClust;
        }


        // Get logical block of start of the current cluster
        CurrentLBA = FAT12_Cluster2LBA(g_FileInfo.CurCluster);

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
            CurrentLBA += (pFAT12_BPB->SectsPerClust - g_SectorsInCurrentClusterLeftToRead);
        }
        else
        {
            // Multiple clusters can fit in the buffer
            // Determine the number of clusters we can read at this time.
            //
            ClustersToRead = MIN(g_ReadBuffLenInClusters, g_FileInfo.NumContigClusters);
            SectorsToRead = (USHORT)(ClustersToRead * pFAT12_BPB->SectsPerClust);
        }

        // Read sectors/clusters from storage.
        //
        if (!ReadSectors(pFAT12_BPB->DriveId, CurrentLBA , SectorsToRead, g_pReadBuffStart))
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
                g_SectorsInCurrentClusterLeftToRead = pFAT12_BPB->SectsPerClust;
            }
        }

        g_FileInfo.NumContigClusters    -=  ClustersToRead;
        g_FileInfo.CurCluster           +=  ClustersToRead;
        g_FileInfo.pCurReadBuffAddr     =   g_pReadBuffStart;
        if(g_DiskReadMode == READ_BY_SECTOR)
        {
            g_FileInfo.NumReadBuffBytes = SectorsToRead * pFAT12_BPB->BytesPerSect;
        }
        else // g_DiskReadMode == READ_BY_CLUSTER
        {
            g_FileInfo.NumReadBuffBytes = ClustersToRead * pFAT12_BPB->SectsPerClust * pFAT12_BPB->BytesPerSect;
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
BOOL FAT12_FSRewind(ULONG Offset)
{
    g_FileInfo.pCurReadBuffAddr -= Offset;
    g_FileInfo.NumReadBuffBytes += Offset;

    return TRUE;
}
