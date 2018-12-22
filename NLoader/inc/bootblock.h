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
#ifndef __BOOT_BLOCK_H
#define __BOOT_BLOCK_H

#include <bootDriver.h>
#include <bootDriverClasses.h>
#include <bootString.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define BOOT_BLOCK_IOCTL(i)     BOOT_IOCTL(BOOT_DRIVER_CLASS_BLOCK, i)

//------------------------------------------------------------------------------

enum BootBlockIoCtl_e {
    BOOT_BLOCK_IOCTL_INFO           = BOOT_BLOCK_IOCTL(0x0001),  // InfoIoCtl
    BOOT_BLOCK_IOCTL_FORMAT         = BOOT_BLOCK_IOCTL(0x0002),  // FormatIoCtl
    BOOT_BLOCK_IOCTL_LOCK_MODE      = BOOT_BLOCK_IOCTL(0x0003),  // LockModeIoCtl
    BOOT_BLOCK_IOCTL_INFO_BINARY    = BOOT_BLOCK_IOCTL(0x0004),  // InfoBinaryIoCtl
    BOOT_BLOCK_IOCTL_INFO_RESERVED  = BOOT_BLOCK_IOCTL(0x0005),  // InfoReservedIoCtl
    BOOT_BLOCK_IOCTL_INFO_PARTITION = BOOT_BLOCK_IOCTL(0x0006),  // InfoPartitionIoCtl
    BOOT_BLOCK_IOCTL_OPEN_BINARY    = BOOT_BLOCK_IOCTL(0x0101),  // OpenBinaryIoCtl
    BOOT_BLOCK_IOCTL_OPEN_RESERVED  = BOOT_BLOCK_IOCTL(0x0102),  // OpenReservedIoCtl
    BOOT_BLOCK_IOCTL_OPEN_PARTITION = BOOT_BLOCK_IOCTL(0x0103),  // OpenPartitionIoCtl
    BOOT_BLOCK_IOCTL_READ           = BOOT_BLOCK_IOCTL(0x0201),  // ReadIoCtl
    BOOT_BLOCK_IOCTL_WRITE          = BOOT_BLOCK_IOCTL(0x0202),  // WriteIoCtl
    BOOT_BLOCK_IOCTL_ERASE          = BOOT_BLOCK_IOCTL(0x0203)   // EraseIoCtl
};

enum BootBlockFlags_e {
    BOOT_BLOCK_SUPPORT_BINARY       = (1 << 0),
    BOOT_BLOCK_SUPPORT_RESERVED     = (1 << 1)
};

typedef struct BootBlockInfoParams_t {
    flags_t flags;
    size_t sectorSize;
    size_t sectors;
    size_t binaryRegions;
    size_t reservedRegions;
    size_t partitions;
} BootBlockInfoParams_t;

typedef struct BootBlockBinaryRegionInfo_t {
    size_t sectors;
} BootBlockBinaryRegionInfo_t;

typedef struct BootBlockReservedRegionInfo_t {
    char name[8];
    size_t sectors;
} BootBlockReservedRegionInfo_t;

typedef struct BootBlockPartitionInfo_t {
    uint8_t fileSystem;
    size_t sectors;
} BootBlockPartitionInfo_t;

enum BootBlockFormatFlags_e {
    BOOT_BLOCK_FORMAT_BINARY_REGIONS = (1 << 0)
};

typedef struct BootBlockFormatInfo_t {
    flags_t flags;
    size_t binaryRegions;
    BootBlockBinaryRegionInfo_t *pBinaryRegionInfo;
    size_t reservedRegions;
    BootBlockReservedRegionInfo_t *pReservedRegionInfo;
    size_t partitions;
    BootBlockPartitionInfo_t *pPartitionInfo;
} BootBlockFormatInfo_t;

enum BootBlockLockMode_e {
    BOOT_BLOCK_LOCK_MODE_ULDR       = 1,
    BOOT_BLOCK_LOCK_MODE_OS         = 2,
    BOOT_BLOCK_LOCK_MODE_HWMON      = 3
};

typedef struct BootBlockLockParams_t {
    enum_t mode;
} BootBlockLockParams_t;

typedef struct BootBlockBinaryInfoParams_t {
    enum_t index;
    size_t sectors;
} BootBlockBinaryInfoParams_t;

typedef struct BootBlockReservedInfoParams_t {
    enum_t index;
    char name[8];
    size_t sectors;
} BootBlockReservedInfoParams_t;

typedef struct BootBlockPartitionInfoParams_t {
    enum_t index;
    char fileSystem;
    enum_t fileSystemIndex;
    size_t sectors;
} BootBlockPartitionInfoParams_t;

typedef struct BootBlockOpenBinaryParams_t {
    enum_t index;
    handle_t hSection;
} BootBlockOpenBinaryParams_t;

typedef struct BootBlockOpenReservedParams_t {
    char name[8];
    handle_t hSection;
} BootBlockOpenReservedParams_t;

typedef struct BootBlockOpenPartitionParams_t {
    uchar fileSystem;
    enum_t index;
    handle_t hSection;
} BootBlockOpenPartitionParams_t;

typedef struct BootBlockReadParams_t {
    size_t sector;
    size_t sectors;
    void *pBuffer;
} BootBlockReadParams_t;

typedef struct BootBlockWriteParams_t {
    size_t sector;
    size_t sectors;
    void *pBuffer;
} BootBlockWriteParams_t;

//------------------------------------------------------------------------------

#define BootBlockDeinit         BootDriverDeinit
#define BootBlockIoCtl          BootDriverIoCtl

//------------------------------------------------------------------------------
//
//  Function:  BootBlockInfo
//
__inline
bool_t
BootBlockInfo(
    handle_t hDriver,
    flags_t *pFlags,
    size_t  *pSectorSize,
    size_t  *pSectors,
    enum_t *pBinaryRegions,
    enum_t *pReservedRegions,
    enum_t *pPartitions
    )
{
    bool_t rc = FALSE;
    BootBlockInfoParams_t params;

    if (BootDriverIoCtl(
            hDriver, BOOT_BLOCK_IOCTL_INFO, &params, sizeof(params)
            ))
        {
        if (pFlags != NULL) *pFlags = params.flags;
        if (pSectorSize != NULL) *pSectorSize = params.sectorSize;
        if (pSectors != NULL) *pSectors = params.sectors;
        if (pBinaryRegions != NULL) *pBinaryRegions = params.binaryRegions;
        if (pReservedRegions != NULL) *pReservedRegions = params.reservedRegions;
        if (pPartitions != NULL) *pPartitions = params.partitions;
        rc = TRUE;
        }
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BootBlockSectorSize
//
__inline
size_t
BootBlockSectorSize(
    handle_t hDriver
    )
{
    size_t sectorSize = 0;
    BootBlockInfoParams_t params;

    if (BootDriverIoCtl(
            hDriver, BOOT_BLOCK_IOCTL_INFO, &params, sizeof(params)
            ))
        {
        sectorSize = params.sectorSize;
        }
    return sectorSize;
}

//------------------------------------------------------------------------------
//
//  Function:  BootBlockFormat
//
__inline
bool_t
BootBlockFormat(
    handle_t hDriver,
    BootBlockFormatInfo_t *pInfo
    )
{
    return BootDriverIoCtl(
        hDriver, BOOT_BLOCK_IOCTL_FORMAT, pInfo, sizeof(*pInfo)
        );
}

//------------------------------------------------------------------------------
//
//  Function:  BootBlockLockMode
//
//  This function sets store to specified lock mode.
//
__inline
bool_t
BootBlockLockMode(
    handle_t hDriver,
    enum_t mode
    )
{
    BootBlockLockParams_t params;

    params.mode = mode;
    return BootDriverIoCtl(
        hDriver, BOOT_BLOCK_IOCTL_LOCK_MODE, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------
//
//  Function:  BootBlockBinaryRegionInfo
//
__inline
bool_t
BootBlockBinaryRegionInfo(
    handle_t hDriver,
    enum_t index,
    size_t *pSectors
    )
{
    bool_t rc = false;
    BootBlockBinaryInfoParams_t params;

    params.index = index;
    if (BootDriverIoCtl(
            hDriver, BOOT_BLOCK_IOCTL_INFO_BINARY, &params, sizeof(params)
            ))
        {
        if (pSectors != NULL) *pSectors = params.sectors;
        rc = true;        
        }
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BootBlockReservedRegionInfo
//
__inline
bool_t
BootBlockReservedRegionInfo(
    handle_t hDriver,
    enum_t index,
    char *pName,
    size_t nameSize,
    size_t *pSectors
    )
{
    bool_t rc = false;
    BootBlockReservedInfoParams_t params;

    params.index = index;
    if (BootDriverIoCtl(
            hDriver, BOOT_BLOCK_IOCTL_INFO_RESERVED, &params, sizeof(params)
            ))
        {
        if (pName != NULL)
            {
            memset(pName, 0, nameSize);
            BootStringCchCopyA(pName, sizeof(params.name), params.name);
            }
        if (pSectors != NULL) *pSectors = params.sectors;
        rc = true;        
        }
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BootBlockPartitionInfo
//
__inline
bool_t
BootBlockPartitionInfo(
    handle_t hDriver,
    enum_t index,
    uchar *pFileSystem,
    enum_t *pIndex,
    size_t *pSectors
    )
{
    bool_t rc = false;
    BootBlockPartitionInfoParams_t params;

    params.index = index;
    if (BootDriverIoCtl(
            hDriver, BOOT_BLOCK_IOCTL_INFO_PARTITION, &params, sizeof(params)
            ))
        {
        if (pFileSystem != NULL) *pFileSystem = params.fileSystem;
        if (pIndex != NULL) *pIndex = params.fileSystemIndex;
        if (pSectors != NULL) *pSectors = params.sectors;
        rc = true;        
        }
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BootBlockOpenBinaryRegion
//
//  This function opens binary region on store.
//
__inline
handle_t
BootBlockOpenBinaryRegion(
    handle_t hDriver,
    enum_t index
    )
{
    handle_t hSection = NULL;
    BootBlockOpenBinaryParams_t params;
    
    params.index = index;
    if (BootDriverIoCtl(
            hDriver, BOOT_BLOCK_IOCTL_OPEN_BINARY, &params, sizeof(params)
            ))
        {
        hSection = params.hSection;
        }
    return hSection;
}

//------------------------------------------------------------------------------
//
//  Function:  BootBlockOpenReservedRegion
//
//  This function opens reserved region on store.
//
__inline
handle_t
BootBlockOpenReservedRegion(
    handle_t hDriver,
    cstring_t name
    )
{
    handle_t hSection = NULL;
    BootBlockOpenReservedParams_t params;

    BootStringCchCopyA(params.name, sizeof(params.name), name);
    if (BootDriverIoCtl(
            hDriver, BOOT_BLOCK_IOCTL_OPEN_RESERVED, &params, sizeof(params)
            ))
        {
        hSection = params.hSection;
        }
    return hSection;
}

//------------------------------------------------------------------------------
//
//  Function:  BootBlockOpenPartition
//
//  This function opens reserved region on store.
//
__inline
handle_t
BootBlockOpenPartition(
    handle_t hDriver,
    uchar fileSystem,
    enum_t index
    )
{
    handle_t hSection = NULL;
    BootBlockOpenPartitionParams_t params;

    params.fileSystem = fileSystem;
    params.index = index;
    if (BootDriverIoCtl(
            hDriver, BOOT_BLOCK_IOCTL_OPEN_PARTITION, &params, sizeof(params)
            ))
        {
        hSection = params.hSection;
        }
    return hSection;
}

//------------------------------------------------------------------------------
//
//  Function:  BootBlockClose
//
__inline
bool_t
BootBlockClose(
    handle_t hSection
    )
{
    return BootDriverDeinit(hSection);
}

//------------------------------------------------------------------------------
//
//  Function:  BootBlockRead
//
__inline
bool_t
BootBlockRead(
    handle_t hSection,
    size_t sector,
    size_t sectors,
    void *pBuffer
    )
{
    BootBlockReadParams_t params;

    params.sector = sector;
    params.sectors = sectors;
    params.pBuffer = pBuffer;
    return BootDriverIoCtl(
        hSection, BOOT_BLOCK_IOCTL_READ, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------
//
//  Function:  BootBlockWrite
//
__inline
bool_t
BootBlockWrite(
    handle_t hSection,
    size_t sector,
    size_t sectors,
    void *pBuffer
    )
{
    BootBlockWriteParams_t params;

    params.sector = sector;
    params.sectors = sectors;
    params.pBuffer = pBuffer;
    return BootDriverIoCtl(
        hSection, BOOT_BLOCK_IOCTL_WRITE, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------
//
//  Function:  BootBlockErase
//
__inline
bool_t
BootBlockErase(
    handle_t hSection
    )
{
    return BootDriverIoCtl(hSection, BOOT_BLOCK_IOCTL_ERASE, NULL, 0);
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_BLOCK_H
