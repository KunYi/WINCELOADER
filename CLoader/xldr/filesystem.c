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
#include "xldr.h"
#include <bootLog.h>

//------------------------------------------------------------------------------

#pragma pack(push, 1)

typedef struct BiosParameterBlockType1_t {
    char     versionId[8];                          // 003
    uint16_t bytesPerSector;                        // 00B
    uint8_t  sectorsPerCluster;                     // 00D
    uint16_t reservedSectors;                       // 00E
    uint8_t  numberOfFats;                          // 010
    uint16_t numberOfRootEntries;                   // 011
    uint16_t sectors;                               // 013
    uint8_t  mediaDescriptor;                       // 015
    uint16_t sectorsPerFat;                         // 016
    uint16_t sectorsPerTrack;                       // 018
    uint16_t numberOfHeads;                         // 01A
    uint32_t hiddenSectors;                         // 01C
    uint32_t totalSectors;                          // 020
    uint8_t  driveId;                               // 024
    uint8_t  mustBeZero1;                           // 025
    uint8_t  extendedBootSignature;                 // 026
    uint32_t volumeSerialNumber;                    // 027
    char     volumeLabel[11];                       // 02B
    char     fatType[8];                            // 036
} BiosParameterBlockType1_t;

typedef struct BiosParameterBlockType2_t {
    char     versionId[8];                          // 003
    uint16_t bytesPerSector;                        // 00B
    uint8_t  sectorsPerCluster;                     // 00D
    uint16_t reservedSectors;                       // 00E
    uint8_t  numberOfFats;                          // 010
    uint16_t mustBeZero1;                           // 011
    uint16_t mustBeZero2;                           // 013
    uint8_t  mediaDescriptor;                       // 015
    uint16_t mustBeZero3;                           // 016
    uint16_t sectorsPerTrack;                       // 018
    uint16_t numberOfHeads;                         // 01A
    uint32_t hiddenSectors;                         // 01C
    uint32_t totalSectors;                          // 020
    uint32_t sectorsPerFat;                         // 024
    uint16_t extendedFlags;                         // 028
    uint16_t fileSystemVersion;                     // 02A
    uint32_t rootDirectoryCluster;                  // 02C
    uint16_t informationSector;                     // 030
    uint16_t backupBootSector;                      // 032
    uint8_t  mustBeZero4[12];                       // 034
    uint8_t  driveId;                               // 040
    uint8_t  mustBeZero5;                           // 041
    uint8_t  extendedBootSignature;                 // 042
    uint32_t volumeSerialNumber;                    // 043
    char     volumeLabel[11];                       // 047
    char     fatType[8];
} BiosParameterBlockType2_t;

typedef struct DirEntry_t {
    char     fileName[8];                           // 00
    char     fileExt[3];                            // 08
    uint8_t  attrib;                                // 0B
    uint8_t  reserved1;                             // 0C
    uint8_t  createTimeFine;                        // 0D
    uint16_t createTime;                            // 0E
    uint16_t createDate;                            // 10
    uint16_t accessData;                            // 12
    uint16_t firstClusterHi;                        // 14
    uint16_t modifyTime;                            // 16
    uint16_t modifyDate;                            // 18
    uint16_t firstCluster;                          // 1A
    uint32_t fileSize;                              // 1C
} DirEntry_t;

typedef struct AddressPacket_t {
    uint16_t size;
    uint16_t blocks;
    uint16_t bufferPtr[2];
    uint32_t blockLo;
    uint32_t blockHi;
} AddressPacket_t;

#pragma pack(pop)

//------------------------------------------------------------------------------

void
BiosInt13(
    uint32_t *pEAX,
    uint32_t *pEBX,
    uint32_t *pECX,
    uint32_t *pEDX,
    uint32_t *pESI,
    uint32_t *pEDI
    );

//------------------------------------------------------------------------------

typedef struct FileSystem_t {
    uint32_t driveId;
    uint32_t bytesPerSector;
    bool_t useExtension;
    uint32_t sectorsPerTrack;
    uint32_t numberOfHeads;
    size_t readSector;
    size_t readOffset;
    size_t bufferSector;
    size_t bufferOffset;
} FileSystem_t;

FileSystem_t s_fileSystem;

//------------------------------------------------------------------------------

static
bool_t
ReadSector(
    FileSystem_t *pFS,
    size_t sector
    )
{
    bool_t rc = false;
    AddressPacket_t *pPacket;
    uint32_t eax, ebx, ecx, edx, esi, edi;

    if (pFS->useExtension)
        {
        pPacket = (AddressPacket_t *)IMAGE_XLDR_BUFFER_PA;
        pPacket->size = sizeof(*pPacket);
        pPacket->blocks = 1;
        pPacket->bufferPtr[0] = LOWORD((DWORD)&pPacket[1]);
        pPacket->bufferPtr[1] = 0;
        pPacket->blockLo = sector;
        pPacket->blockHi = 0;

        eax = 0x4200;
        edx = pFS->driveId;
        esi = (uint32_t)pPacket;
        BiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
        if ((eax & 0xFF00) != 0) goto cleanUp;
        }
    else
        {
        uint32_t sectorsPerCylinder;
        uint32_t c, h, s;

        sectorsPerCylinder = pFS->sectorsPerTrack * pFS->numberOfHeads;
        c = sector / sectorsPerCylinder;
        s = sector - c * sectorsPerCylinder;
        h = s / pFS->sectorsPerTrack;
        s = s - h * pFS->sectorsPerTrack + 1;

        eax = 0x0201;
        ebx = IMAGE_XLDR_BUFFER_PA + sizeof(AddressPacket_t);
        ecx = (s & 0x3F) | ((c & 0x300) >> 2) | ((c & 0xFF) << 8);
        edx = pFS->driveId | ((h & 0xFF) << 8);

        BiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
        if ((eax & 0xFF00) != 0) goto cleanUp;
        }
    
    // Done
    rc = true;

cleanUp:
    if (!rc) BootLog(
        L"Couldn't read sector %d from drive 0x%02x\r\n", sector, pFS->driveId
        );
    return rc;
}

//------------------------------------------------------------------------------

handle_t
FileSystemInit(
    )
{
    handle_t hFileSystem = NULL;
    uint32_t driveId;
    uint32_t eax, ebx, ecx, edx, esi, edi;
    BiosParameterBlockType1_t *pBpb1;
    BiosParameterBlockType2_t *pBpb2;
    DirEntry_t *pEntry;
    size_t bytesPerSector, sectors, sectorsPerCluster, size;
    size_t fatSector, dataSector, dirSector, sector;
    uint8_t *pSector;


    memset(&s_fileSystem, 0, sizeof(s_fileSystem));

	//(db) The CE formatter zero terminates the fattype string which
	// causes the check below to fail. Tolerate such stupidity with
	// the following fix.

    // Find Bios Parameter Block type and copy required parameters
    pBpb1 = (BiosParameterBlockType1_t*)(IMAGE_XLDR_BOOTSEC_PA + 0x0003);
    pBpb2 = (BiosParameterBlockType2_t*)(IMAGE_XLDR_BOOTSEC_PA + 0x0003);

	//(db) The CE formatter zero terminates the fattype string which
	// causes the check below to fail. Tolerate such stupidity with
	// the following fix immediately below and for the FAT16 check.
    //(db)if (memcmp(pBpb2->fatType, "FAT32   ", sizeof(pBpb2->fatType)) == 0)
    if (memcmp(pBpb2->fatType, "FAT32  ", sizeof(pBpb2->fatType)-1) == 0)
        {
        bytesPerSector = pBpb2->bytesPerSector;
        sectorsPerCluster = pBpb2->sectorsPerCluster;
        sectors = pBpb2->totalSectors;
        driveId = pBpb2->driveId;
        fatSector = pBpb2->hiddenSectors + pBpb2->reservedSectors;
        dataSector = fatSector + pBpb2->numberOfFats * pBpb2->sectorsPerFat;
        dirSector = (pBpb2->rootDirectoryCluster - 2) * sectorsPerCluster;
        dirSector = dirSector + dataSector;
		//(db) fix if necessary for later checks.
		pBpb2->fatType[7] = ' ';
        }
    //(db)else if ((memcmp(pBpb1->fatType, "FAT16   ", sizeof(pBpb1->fatType)) == 0)||
    //(db)         (memcmp(pBpb1->fatType, "FAT12   ", sizeof(pBpb1->fatType)) == 0))
    else if ((memcmp(pBpb1->fatType, "FAT16  ", sizeof(pBpb1->fatType)-1) == 0)||
             (memcmp(pBpb1->fatType, "FAT12  ", sizeof(pBpb1->fatType)-1) == 0))
        {
#if 0
        BootLog(L"bytesPerSector: %04x\r\n", pBPB->bytesPerSector);
        BootLog(L"sectorsPerCluster: %02x\r\n", pBPB->sectorsPerCluster);
        BootLog(L"reservedSectors: %04x\r\n", pBPB->reservedSectors);
        BootLog(L"numberOfFats: %02x\r\n", pBPB->numberOfFats);
        BootLog(L"numberOfRootEntries: %04x\r\n", pBPB->numberOfRootEntries);
        BootLog(L"sectorsPerPartition: %04x\r\n", pBPB->sectorsPerPartition);
        BootLog(L"mediaDescriptor: %02x\r\n", pBPB->mediaDescriptor);
        BootLog(L"sectorsPerFat: %04x\r\n", pBPB->sectorsPerFat);
        BootLog(L"sectorsPerTrack: %04x\r\n", pBPB->sectorsPerTrack);
        BootLog(L"numberOfHeads: %04x\r\n", pBPB->numberOfHeads);
        BootLog(L"hiddenSectors: %08x\r\n", pBPB->hiddenSectors);
        BootLog(L"totalSectors: %08x\r\n", pBPB->totalSectors);
        BootLog(L"driveId: %02x\r\n", pBPB->driveId);
#endif
        bytesPerSector = pBpb1->bytesPerSector;
        sectorsPerCluster = pBpb1->sectorsPerCluster;
        sectors = pBpb1->sectors;
        if (sectors == 0) sectors = pBpb1->totalSectors;
        driveId = pBpb1->driveId;
        fatSector = pBpb1->hiddenSectors + pBpb1->reservedSectors;
        dirSector = fatSector + pBpb1->numberOfFats * pBpb1->sectorsPerFat;
        dataSector = pBpb1->numberOfRootEntries * sizeof(DirEntry_t);
        dataSector = dataSector / bytesPerSector + dirSector;
		//(db) fix if necessary for later checks.
		pBpb1->fatType[7] = ' ';
        }
    else
        {
        BootLog(L"Unsupported file system!\r\n");
        goto cleanUp;
        }

    // Save same basic info
    s_fileSystem.driveId = driveId;
    s_fileSystem.bytesPerSector = bytesPerSector;

    // Find if EDD is supported
    eax = 0x4100;
    ebx = 0x55AA;
    edx = driveId;
    BiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if (((ebx & 0xFFFF) == 0xAA55) && ((ecx & 0x0001) != 0))
        {
        s_fileSystem.useExtension = true;
        }
    else
        {
        eax = 0x0800;
        edx = driveId;
        BiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
        s_fileSystem.useExtension = false;
        s_fileSystem.sectorsPerTrack = ecx & 0x03F;
        s_fileSystem.numberOfHeads = (edx >> 8) + 1;
        }

    // Read directory sector
    pSector = (uint8_t*)(IMAGE_XLDR_BUFFER_PA + sizeof(AddressPacket_t));
    if (!ReadSector(&s_fileSystem, dirSector)) goto cleanUp;

    // Find boot loader file 
    size = 0;
    while (size < bytesPerSector)
        {
        if (memcmp(&pSector[size], "WCELDR     ", 11) == 0) break;
        size += sizeof(DirEntry_t);
        if (size >= bytesPerSector)
            {
            BootLog(L"Couldn't find WCELDR file!\r\n");   
            goto cleanUp;
            }
        }
    pEntry = (DirEntry_t*)&pSector[size];
    sector = (pEntry->firstClusterHi << 16) | pEntry->firstCluster;
    sector = (sector - 2) * sectorsPerCluster + dataSector;    
   
    // Look for sector with signature
    while (sector < sectors)
        {
        if (!ReadSector(&s_fileSystem, sector)) goto cleanUp;
        if (memcmp(pSector, "B000FF\x0A", 7) == 0) break;
        sector++;
        }
    if (sector >= sectors) goto cleanUp;    

    s_fileSystem.readSector = sector;
    s_fileSystem.readOffset = 0;
    s_fileSystem.bufferSector = (size_t)-1;
    s_fileSystem.bufferOffset = 0;
    hFileSystem = &s_fileSystem;

cleanUp:
    return hFileSystem;
}

//------------------------------------------------------------------------------

bool_t
FileSystemRead(
    handle_t hFileSystem,
    void *pBuffer,
    size_t size
    )
{
    bool_t rc = false;
    FileSystem_t *pFileSystem = (FileSystem_t*)hFileSystem;
    uint8_t *pSector;
    size_t count;

    pSector = (uint8_t *)(IMAGE_XLDR_BUFFER_PA + sizeof(AddressPacket_t));
    
    while (size > 0)
        {
        if (pFileSystem->readSector != pFileSystem->bufferSector)
            {
            if (!ReadSector(pFileSystem, pFileSystem->readSector)) goto cleanUp;
            pFileSystem->bufferSector = pFileSystem->readSector;
            pFileSystem->bufferOffset = 0;
            }
        count = pFileSystem->bytesPerSector - pFileSystem->bufferOffset;
        if (count > size) count = size;
        memcpy(pBuffer, &pSector[pFileSystem->bufferOffset], count);
        pBuffer = ((uint8_t*)pBuffer) + count;
        pFileSystem->bufferOffset += count;
        pFileSystem->readOffset += count;
        if (pFileSystem->readOffset >= pFileSystem->bytesPerSector)
            {
            pFileSystem->readSector++;
            pFileSystem->readOffset = 0;
            }
        size -= count;
        }        

    // Done
    rc = true;

cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------

void
FileSystemDeinit(
    handle_t hFileSystem
    )
{
    UNREFERENCED_PARAMETER(hFileSystem);
}

//------------------------------------------------------------------------------

