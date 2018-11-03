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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: main.c

Abstract: Utility to update the active partition boot sector and update the first root directory
          entry to support finding and loading the Windows CE BIOS bootloader.  It also transfers
          the bootloader image.

Functions:

Notes:

    The following assumptions have been made - these are areas that might be revisited in a future release:
    
    * The boot sector and bootloader designs assume the loader image will be a contiguous grouping of sectors
      on the storage device.  To help ensure this is true, the storage device should first be formatted and
      before copying other files, this utility should be run to transfer the loader.  Note that bad sectors
      are not accounted for in this utility and could cause problems.
      
    * It's assumed that the storage device is fully prepared before running this utility.  Prepared means that
      there is an configured Active partition on the storage device and that it has been formatted (not to
      include DOS system files).
      
    * The bootloader is assumed to lie in the root directory of the storage device.  If it needs to be placed
      somewhere else, this utility and the boot sector will need to be updated. 

--*/

#include <stdio.h>
#include <malloc.h>      
#include <memory.h>

//
// FAT filesystem constants.
//
#define SECTOR_SIZE      0x200
#define PARTTBL_SIZE        64
#define PARTSIG_SIZE         2
#define PARTACTV_FLAG     0x80
#define MAX_PARTITION        4

//
// Disk constants.
//
#define FIXED_DISK_ID     0x80
#define FLOPPY_DISK_ID       0
#define FLOPPY_IO_RETRIES    2
#define FIXDISK_IO_RETRIES   0

#define TRUE                 1
#define FALSE                0

//
// Helper macros.
//
#define TOLOWER(a) ((a >= 0x41 && a <= 0x5a) ? (a + 0x20) : a)

//
// FAT directory entry structure.
//
#pragma pack(1)
typedef struct
{
    unsigned char  FileName[8];
    unsigned char  FileExt[3];
    unsigned char  FATTr;
    unsigned char  FOO[10];
    unsigned short LModTime;
    unsigned short LModDate;
    unsigned short FirstClust;
    unsigned long  FileSize;
} DIRENTRY, *PDIRENTRY;

//
// BIOS parameter block structure.
//
#pragma pack(1)
typedef struct BIOSPB_TAG
{
    unsigned char  VersionId[8];
    unsigned short BytesPerSect;
    unsigned char  SectsPerClust;
    unsigned short RsvdSects;
    unsigned char  NumFATs;
    unsigned short NumRootEntries;
    unsigned short SectsPerPart;
    unsigned char  MediaDesc;
    unsigned short SectsPerFAT;
    unsigned short SectsPerTrack;
    unsigned short NumHeads;
    unsigned short NumHiddenSectL;
    unsigned short NumHiddenSectH;
    unsigned short TotalSectorsL;
    unsigned short TotalSectorsH;
    unsigned char  DriveId;
    unsigned char  TempVal;
    unsigned char  ExtRecordSig;
    unsigned short VolSerNumL;
    unsigned short VolSerNumH;
    unsigned char  VolLabel[11];
    unsigned char  TypeFAT[8];
} BIOSPB, *PBIOSPB;

//
// MBR partition table entry structure.
//
#pragma pack(1)
typedef struct
{
    unsigned char ActiveFlag;
    unsigned char SSector;
    unsigned char SHead;
    unsigned char SCylinder;
    unsigned char Type;
    unsigned char ESector;
    unsigned char EHead;
    unsigned char ECylinder;
    unsigned long SLBA;
    unsigned long Size;
} PARTTE, *PPARTTE;

//
// User parameters structure.
//
typedef struct
{
    unsigned short Offset;
    unsigned char *pSectorFileName;
    unsigned char *pLoaderFileName;
    unsigned char  DriveNum;
    unsigned char  DriveLetter;
} PARAMS, *PPARAMS;

//
// Globals.
//
unsigned long gActivePartLBA;           // LBA address of active partition.
unsigned char gTempSector[SECTOR_SIZE]; // Buffer for sector reads.

//
// Function prototypes.
//
static int ReadSector(unsigned char Drive, unsigned short Cylinder, unsigned char Head, unsigned char Sector, unsigned char *pBuffer);
static int WriteSector(unsigned char Drive, unsigned short Cylinder, unsigned char Head, unsigned char Sector, unsigned char *pBuffer);
static int WriteBootSector(unsigned char *pSector, unsigned char DriveNum, unsigned short Offset, PBIOSPB pBPB);
static int ParseArguments(int ArgC, char **ArgV, PPARAMS pUserParams);
static int TransferLoader(PPARAMS pParams, PBIOSPB pBPB);
static void LBA2PCHS(unsigned long LBA, unsigned short *pCylinder, unsigned char  *pHead, unsigned char  *pSector, PBIOSPB pBPB);


//
// Main routine.
//
int main(int ArgC, char **ArgV)
{
    PARAMS UserParams;
    FILE *FP = NULL;
    BIOSPB BPB;
    unsigned short DataSize = 0;
    unsigned char Sector[SECTOR_SIZE];

    // If no parameters provided, user is looking for help.
    //
    if (ArgC == 1)
        goto UsageMessage;

    // Parse user arguments.
    //
    if (ParseArguments(ArgC, ArgV, &UserParams))
    {
        printf("ERROR: Invalid command line.\r\n");
        goto UsageMessage;
    }

    // Open the boot sector file for reading.
    //
    FP = fopen(UserParams.pSectorFileName, "rb");
    if (FP == NULL)
    {
        printf("ERROR: Unable to open boot sector image (%s) for reading.\r\n", UserParams.pSectorFileName);
        goto Done;
    }

    // Read file into sector buffer (terminate with EOF encountered or we reach the
    // end of the sector buffer).
    //
    memset(Sector, 0, sizeof(SECTOR_SIZE));
    for (DataSize = 0 ; DataSize < SECTOR_SIZE && !feof(FP) ; DataSize++)
    {
        *(unsigned char *)(Sector + DataSize) = fgetc(FP);
    }

    // Update the boot sector.
    //
    if (WriteBootSector(Sector, UserParams.DriveNum, UserParams.Offset, &BPB))
    {
        printf("ERROR: Unable to write to boot sector.\r\n");
        goto Done;
    }

    // Transfer bootloader (copy it to the storage device, updating the root directory).
    //
    if (TransferLoader(&UserParams, &BPB))
    {
        printf("ERROR: Unable to copy to bootloader.\r\n");
        goto Done;
    }
    
Done:
    if (FP != NULL)
        fclose(FP);

    return(0);

UsageMessage:
    printf("Prepares the storage device to run the Windows CE BIOS bootloader.\r\n\r\n");
    printf("%s [-b:offset] bsectfile bldrfile drive\r\n\r\n", ArgV[0]);
    printf("[-b:offset]      - offsets specified bytes into the sector before writing\r\n");
    printf("bsectfile        - name of file that contains sector data\r\n");
    printf("bldrfile         - name of bootloader executable file\r\n");
    printf("drive            - driver letter (a:, c:, etc.)\r\n");

    return(0);
}


// 
// Write the specified sector buffer, with offset applied, to the boot sector and return the BIOS parameter
// block (BPB) contents to the caller.
//
static int WriteBootSector(unsigned char *pSector, unsigned char DriveNum, unsigned short Offset, PBIOSPB pBPB)
{
    int Status = 0;
    unsigned short Cylinder;
    unsigned char Head, Sector;
    unsigned char Count;
    PPARTTE pPartTable = NULL;

    // Check parameters.
    //
    if (!pSector || !pBPB)
        return(-1);

    // To find the active boot partition on a fixed disk, we need to look at the MBR.
    // At the end of the MBR is the partition table information.  An active flag
    // denotes the active partition entry and from this we can find the location 
    // of the boot sector.
    //
    // Note - we can skip much of this if we're writing to a floppy because the boot
    // sector is the first sector on the floppy.
    //

    if (DriveNum & FIXED_DISK_ID)   // Fixed disk.
    {
        // Read MBR.
        //
        if ((Status = ReadSector(DriveNum, 0, 0, 1, gTempSector)) != 0)
        {
            printf("ERROR: Unable to read the MBR (status=0x%x).\r\n", Status);
            return(-1);
        }

        // Search through partition table in the MBR for the active partition.
        //
        pPartTable = (PPARTTE)(gTempSector + SECTOR_SIZE - PARTSIG_SIZE - PARTTBL_SIZE);

        for(Count = 0 ; (Count < MAX_PARTITION) && (pPartTable->ActiveFlag != PARTACTV_FLAG); ++Count)
            ++pPartTable;

        // Find active partition?
        //
        if (Count == MAX_PARTITION)
        {
            printf("ERROR: Unable to find active partition on drive 0x%x.\r\n", DriveNum);
            return(-1);
        }

        // Record starting cylinder, header, and sector values of active partition - this is the 
        // location of the partition's boot sector.
        //
        Cylinder = pPartTable->SCylinder;
        Head     = pPartTable->SHead;
        Sector   = pPartTable->SSector;

        gActivePartLBA = pPartTable->SLBA;

    #if 0
        printf("INFO: Found active partition (C:H:S = 0x%x:0x%x:0x%x).\r\n\r\n", Cylinder, Head, Sector);

        printf("Active Flag ...... 0x%02x\r\n", pPartTable->ActiveFlag);
        printf("Start Cylinder ... 0x%02x\r\n", pPartTable->SCylinder);
        printf("Start Head ....... 0x%02x\r\n", pPartTable->SHead);
        printf("Start Sector ..... 0x%02x\r\n", pPartTable->SSector);
        printf("Type ............. 0x%02x\r\n", pPartTable->Type);
        printf("End Cylinder ..... 0x%02x\r\n", pPartTable->ECylinder);
        printf("End Head ......... 0x%02x\r\n", pPartTable->EHead);
        printf("End Sector ....... 0x%02x\r\n", pPartTable->ESector);
        printf("Start LBA ........ 0x%08x\r\n", pPartTable->SLBA);
        printf("Size in Sectors .. 0x%08x\r\n\r\n", pPartTable->Size);
    #endif

    }
    else            // Floppy.
    {
        Cylinder = 0;
        Head     = 0;
        Sector   = 1;   

        gActivePartLBA = 0;

    #if 0
        printf("INFO: Floppy disk boot sector (C:H:S = 0x%x:0x%x:0x%x).\r\n\r\n", Cylinder, Head, Sector);
    #endif
    }
    
    //
    // User specify an offset to be applied to the boot sector data before writing? 
    //
    if (Offset)
    {
        // If we're writing to an offset within the boot sector, read the sector from disk
        // first to preserve the leading bytes.  Typically this is done to preserve the BIOS
        // Parameter Block (BPB) placed in the sector by the format utility.
        //
        if ((Status = ReadSector(DriveNum, Cylinder, Head, Sector, gTempSector)) != 0)
        {
            printf("ERROR: Unable to read boot sector (status=0x%x).\r\n", Status);
            return(-1);
        }
    }

    // Overlay the callers data.
    //
    memcpy((gTempSector + Offset), pSector, (SECTOR_SIZE - Offset));

    // Copy the BIOS parameter block for the caller.
    //
    memcpy(pBPB, (gTempSector + 3), sizeof(BIOSPB));

    // Write the boot sector data back to disk.
    //
    Status = WriteSector(DriveNum, Cylinder, Head, Sector, gTempSector);
    if (Status)
    {
        printf("ERROR: Unable to write boot sector (status=0x%x).\r\n", Status);
        return(-1);
    }

    return(0);
}

//
// Parse and validate user's arguments.
//
static int ParseArguments(int ArgC, char **ArgV, PPARAMS pUserParams)
{
    char DriveLetter;

    // Check for valid arguments.
    //
    if (ArgC < 4 && ArgC != 5)
        return(-1);

    if (!pUserParams)
        return(-1);

    memset(pUserParams, 0, sizeof(PARAMS));

    // "argv[0] -b offset sectfile bldrfile drive".

    // Get drive letter.
    //
    DriveLetter = TOLOWER(ArgV[ArgC - 1][0]);

    // Make sure user is passing a valid driver letter.
    //
    if (DriveLetter < 'a' || DriveLetter > 'z')
    {
        printf("ERROR: Invalid drive letter (%c:).\r\n", DriveLetter);
        return(-1);
    }
    if (DriveLetter < 'c')
    {
        pUserParams->DriveNum = (FLOPPY_DISK_ID + (DriveLetter - 'a'));
    }
    else
    {
        pUserParams->DriveNum = (FIXED_DISK_ID + (DriveLetter - 'c'));
    }
    pUserParams->DriveLetter = DriveLetter;

    // Get file name.
    //
    pUserParams->pSectorFileName = ArgV[ArgC - 3];  
    pUserParams->pLoaderFileName = ArgV[ArgC - 2];  

    // Check for optional byte offset
    //
    if (ArgC > 4)
    {
        if (!memicmp(ArgV[1], "-b:", 3))
        {
            pUserParams->Offset = (unsigned char)atoi(&ArgV[1][3]);     // Not a space since the above bounds check would have caught it.

            // Make sure offset value fits within a sector.
            //
            if (pUserParams->Offset >= SECTOR_SIZE)
            {
                printf("ERROR: Offset (0x%x)should be less than sector size (0x%x).\r\n", pUserParams->Offset, SECTOR_SIZE);
                return(-1);
            }
        }
    }

    return(0);
}


//
// Read a disk sector using the BIOS INT 13h command.
//
static int ReadSector(unsigned char Drive, unsigned short Cylinder, unsigned char Head, unsigned char Sector, unsigned char *pBuffer)
{
    int Status = 0;
    unsigned char Retries = 0;

    // If we're reading from a floppy, retry the operation a couple of times to allow for the floppy to spin up.
    //
    if (Drive < FIXED_DISK_ID)
        Retries = FLOPPY_IO_RETRIES;
    else
        Retries = FIXDISK_IO_RETRIES;

    do
    {
        _asm
        {
            push   ax
            push   bx
            push   cx
            push   dx
    
            mov    ah, 02h          ; BIOS read sector command.
            mov    al, 1            ; Read 1 sector.
            mov    dx, Cylinder     ; Cylinder number.
            mov    cl, 06H
            shl    dh, cl
            or     dh, Sector       ; Sector number.
            mov    cx, dx
            xchg   ch, cl
            mov    dl, Drive        ; Drive number.
            mov    dh, Head         ; Head number.
            mov    bx, pBuffer      ; Buffer address.
            int    13h
            jnc    RS_Done
            lea    bx, Status
            mov    [bx], ah        
RS_Done:
            pop    dx
            pop    cx
            pop    bx
            pop    ax
        }
    }
    while(Status && Retries--);

    return(Status);
}


//
// Write a disk sector using the BIOS INT 13h command.
//
static int WriteSector(unsigned char Drive,  unsigned short Cylinder, unsigned char Head, unsigned char Sector, unsigned char *pBuffer)
{
    int Status = 0;
    unsigned char Retries = 0;

    // If we're writing to a floppy, retry the operation a couple of times to allow for the floppy to spin up.
    //
    if (Drive < FIXED_DISK_ID)
        Retries = FLOPPY_IO_RETRIES;
    else
        Retries = FIXDISK_IO_RETRIES;

    do
    {
        _asm
        {
            push   ax
            push   bx
            push   cx
            push   dx
    
            mov    ah, 03h          ; BIOS write sector command.
            mov    al, 1            ; Read 1 sector.
            mov    dx, Cylinder     ; Cylinder number.
            mov    cl, 06H
            shl    dh, cl
            or     dh, Sector       ; Sector number.
            mov    cx, dx
            xchg   ch, cl
            mov    dl, Drive        ; Drive number.
            mov    dh, Head         ; Head number.
            mov    bx, pBuffer      ; Buffer address.
            int    13h
            jnc    WS_Done
            lea    bx, Status
            mov    [bx], ah        
WS_Done:
            pop    dx
            pop    cx
            pop    bx
            pop    ax
        }
    }
    while(Status && Retries--);

    return(Status);
}


//
// Transfer the bootloader to the storage device and update the root directory.
//
static int TransferLoader(PPARAMS pParams, PBIOSPB pBPB)
{
    unsigned long RootDirLBA = 0;
    int Status = 0;
    unsigned short Cylinder;
    unsigned char Head, Sector;
    PDIRENTRY pDirEntry = NULL;
#define MAX_COMMAND_STRING  50
    unsigned char SystemCommand[MAX_COMMAND_STRING];

    //
    // This function transfers the bootloader image to the target drive.  The
    // loader image must be stored in a contiguous grouping of sectors and the
    // loader file name must be first in the FAT directory.  Both constraints
    // are required for the boot sector to locate and load the bootloader.
    //

    // Determine the active partitions root directory LBA and convert it to a physical CHS value
    // which is needed by the BIOS.
    //
    RootDirLBA = gActivePartLBA + (pBPB->NumFATs * pBPB->SectsPerFAT) + pBPB->RsvdSects;
    LBA2PCHS(RootDirLBA, &Cylinder, &Head, &Sector, pBPB);    

    // Read the first sector of the root directory.
    //
    if ((Status = ReadSector(pParams->DriveNum, Cylinder, Head, Sector, gTempSector)) != 0)
    {
        printf("ERROR: Unable to read a root directory sector (status=0x%x).\r\n", Status);
        return(-1);
    }

    // Update the first entry of the root directory so a later copy will use it.
    // MS-DOS's IO.SYS might be here if this is a formatted system disk.
    //
    pDirEntry = (PDIRENTRY)gTempSector;

#if 0
    printf("FileName: 0x%x %c%c%c%c%c%c%c.%c%c%c.\r\n", pDirEntry->FileName[0], pDirEntry->FileName[1], pDirEntry->FileName[2], pDirEntry->FileName[3], pDirEntry->FileName[4], pDirEntry->FileName[5], pDirEntry->FileName[6], pDirEntry->FileName[7], pDirEntry->FileExt[0], pDirEntry->FileExt[1], pDirEntry->FileExt[2]);
    printf("Attrib:   0x%x\r\n", pDirEntry->FATTr);
    printf("LModTime: 0x%x\r\n", pDirEntry->LModTime);
    printf("LModDate: 0x%x\r\n", pDirEntry->LModDate);
    printf("First Cluster: 0x%x\r\n", pDirEntry->FirstClust);
    printf("File Size: 0x%x\r\n", pDirEntry->FileSize);
#endif

    // Initialize first directory entry in order for copy to make use of it (it's typically updated by the DOS sys tool).
    //
    memset(pDirEntry->FileName, ' ', 8);
    memset(pDirEntry->FileName, ' ', 3);
    pDirEntry->FileName[0] = 0xe5;          // Deleted file tag.
    pDirEntry->FATTr       = 0;
    memset(pDirEntry->FOO, ' ', 10);
    pDirEntry->LModTime    = 0;
    pDirEntry->LModDate    = 0;
    pDirEntry->FirstClust  = 0;
    pDirEntry->FileSize    = 0;
   
    // Write the updated boot sector back to disk.
    // 
    if ((Status = WriteSector(pParams->DriveNum, Cylinder, Head, Sector, gTempSector)) != 0)
    {
        printf("ERROR: Unable to write root a directory sector (status=0x%x).\r\n", Status);
        return(-1);
    }

    // Copy the bootloader image.  It's assumed that the copy will be to a
    // contiguous group of sectors.  This might not be true if the disk
    // already contains files (documented setup requirement) or if the disk
    // contains bad sector(s).
    //
    // TODO - this area should be revisited.
    //
    sprintf(SystemCommand, "copy %s %c:", pParams->pLoaderFileName, pParams->DriveLetter);
    SystemCommand[MAX_COMMAND_STRING - 1] = '\0';

    if (system(SystemCommand) == -1)
    {
        printf("ERROR: Unable to transfer bootloader.\r\n");
        return(-1);
    }

    return(0);
}


//
// Convert a LBA address to a physical CHS address that's used by the BIOS.
//
static void LBA2PCHS(unsigned long LBA, unsigned short *pCylinder, unsigned char  *pHead, unsigned char  *pSector, PBIOSPB pBPB)
{
    unsigned short Temp = 0;

    // Do the math...
    *pCylinder = (unsigned short)(LBA / (pBPB->NumHeads * pBPB->SectsPerTrack));
    Temp       = (unsigned short)(LBA % (pBPB->NumHeads * pBPB->SectsPerTrack));
    *pHead     = (unsigned char)(Temp / pBPB->SectsPerTrack);
    *pSector   = (unsigned char)(Temp % pBPB->SectsPerTrack) + 1;
}

