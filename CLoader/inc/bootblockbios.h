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
#ifndef __BOOT_BLOCK_BIOS_H
#define __BOOT_BLOCK_BIOS_H

//(db)#include <bootBlock.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#pragma pack(push, 1)

typedef struct BootBlockBiosParameterBlockType1_t {
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
} BootBlockBiosParameterBlockType1_t;

typedef struct BootBlockBiosParameterBlockType2_t {
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
} BootBlockBiosParameterBlockType2_t;

typedef struct BootBlockBiosParameterBlockTypeEx_t {
    char     versionId[8];                          // 003
    uint8_t  mustBeZero1[53];                       // 00B
    uint64_t partitionOffset;                       // 040
    uint64_t partitionLength;                       // 048
    uint32_t fatOffset;                             // 050
    uint32_t fatLength;                             // 054
    uint32_t clusterHeapOffset;                     // 058
    uint32_t clusterCount;                          // 05C
    uint32_t firstRootDirCluster;                   // 060
    uint32_t volumeSerialNumber;                    // 064
    uint16_t fileSystemVersion;                     // 068
    uint16_t extendedFlags;                         // 06A
    uint8_t  bytesPerSectorLog2;                    // 06C
    uint8_t  sectorsPerClusterLog2;                 // 06D
    uint8_t  numberOfFats;                          // 06E
    uint8_t  driveId;                               // 06F
    uint8_t  percentInUse;                          // 070
    uint8_t  mustBeZero2[7];                        // 071
} BootBlockBiosParameterBlockTypeEx_t;

#pragma pack(pop)

//------------------------------------------------------------------------------

//(db)
//enum BootBlockBiosIoCtl_e {
//    BOOT_BLOCK_IOCTL_DATA_SECTORS = BOOT_BLOCK_IOCTL(0x8001)
//};

typedef struct BootBlockDataSectorParams_t {
    size_t sector;
    size_t sectors;
} BootBlockDataSectorsParams_t;

//------------------------------------------------------------------------------

handle_t
BootBlockBiosInit(
    enum_t diskId
    );

//(db)
////------------------------------------------------------------------------------
//
//__inline
//bool_t
//BootBlockDataSectors(
//    handle_t hSection,
//    size_t *pSector,
//    size_t *pSectors
//    )
//{
//    bool_t rc = false;
//    BootBlockDataSectorsParams_t params;
//
//    if (BootDriverIoCtl(
//            hSection, BOOT_BLOCK_IOCTL_DATA_SECTORS, &params, sizeof(params)
//            ))
//        {
//        if (pSector != NULL) *pSector = params.sector;
//        if (pSectors != NULL) *pSectors = params.sectors;
//        rc = true;
//        }
//    return rc;
//}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_BLOCK_BIOS_H
