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
#ifndef __BOOT_DOWNLOAD_BIN_FORMAT_H
#define __BOOT_DOWNLOAD_BIN_FORMAT_H

#include "include/bootTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

//------------------------------------------------------------------------------

#define BOOT_BIN_SIGNATURE_RAM              "B000FF\x0A"
#define BOOT_BIN_SIGNATURE_SIGNED_RAM       "S000FF\x0A"
#define BOOT_BIN_SIGNATURE_RAW              "N000FF\x0A"
#define BOOT_BIN_SIGNATURE_SIGNED_RAW       "R000FF\x0A"
#define BOOT_BIN_SIGNATURE_STORE            "D000FF\x0A"
#define BOOT_BIN_SIGNATURE_JUMP             "J00000\x0A"

//------------------------------------------------------------------------------

typedef struct BootBinFormatRamHeader_t {
    uint32_t start;
    uint32_t size;
} BootBinFormatRamHeader_t;

typedef struct BootBinFormatRamRecordHeader_t {
    uint32_t address;
    uint32_t length;
    uint32_t checksum;
} BootBinFormatRamRecordHeader_t;

//------------------------------------------------------------------------------

typedef struct BootBinFormatSignature_t {
    uint8_t signature[7];
} BootBinFormatSignature_t;

enum BootBinFormatFlags_e {
    BOOT_BIN_FORMAT_FLAG_CLEAN = (1 << 0)
};

//------------------------------------------------------------------------------

enum BootBinFormatHashType_e {
    BOOT_BIN_FORMAT_HASH_SUM    = 0,
    BOOT_BIN_FORMAT_HASH_SHA1   = 1,
    BOOT_BIN_FORMAT_HASH_SHA256 = 2,

    BOOT_BIN_FORMAT_HASH_OEM = 0x8000
    // OEM can define new hashing algorithms in their own platform-specific
    // BSPs. The definitions must start at BOOT_BIN_FORMAT_HASH_OEM, e.g.
    //    BOOT_BIN_FORMAT_HASH_ALG1 = BOOT_BIN_FORMAT_HASH_OEM + 0,
    //    BOOT_BIN_FORMAT_HASH_ALG2 = BOOT_BIN_FORMAT_HASH_OEM + 1
};
//------------------------------------------------------------------------------

typedef struct BootBinFormatStoreHeader_t {
    uint32_t flags;
    uint32_t sectorSize;
    uint32_t sectors;
    uint32_t segments;
    uint32_t hashType;
    uint32_t hashSize;
    uint32_t hashInfoSize;
} BootBinFormatStoreHeader_t;

typedef struct BootBinFormatStoreSha1Info_t {
    uint32_t publicKeySize;
    uint32_t seedSize;
/*
    uint8_t  publicKey[publicKeySize];
    uint8_t  seed[seedSize];
*/
} BootBinFormatStoreSha1Info_t;

enum BootBinFormatStoreSegmentType_e {
    BOOT_BIN_FORMAT_STORE_SEGMENT_BINARY    = 1,
    BOOT_BIN_FORMAT_STORE_SEGMENT_RESERVED  = 2,
    BOOT_BIN_FORMAT_STORE_SEGMENT_PARTITION = 3
};

typedef struct BootBinFormatStoreSegmentHeader_t {
    uint32_t type;
    uint32_t sectors;
    uint32_t infoSize;
} BootBinFormatStoreSegmentHeader_t;

typedef struct BootBinFormatStoreSegmentBinaryInfo_t {
    uint8_t index;
} BootBinFormatStoreSegmentBinaryInfo_t;

typedef struct BootBinFormatStoreSegmentReservedInfo_t {
    char name[8];
} BootBinFormatStoreSegmentReservedInfo_t;

typedef struct BootBinFormatStoreSegmentPartitionInfo_t {
    uint8_t fileSystem;
    uint8_t index;
} BootBinFormatStoreSegmentPartitionInfo_t;

typedef struct BootBinFormatStoreRecordHeader_t {
    uint32_t segment;
    uint32_t sector;
    uint32_t sectors;
} BootBinFormatStoreRecordHeader_t;

/*
   This is pseudo-C definition of store format

typedef struct BootBinFormatStore_t {
    uint8_t signature[7];
    BootBinFormatStoreHeader_t header;
    uint8_t hashInfo[header.hashInfoSize];
    struct {
        BootBinFormatStoreSegmentHeader_t segmentHeader;
        union {
            BootBinFormatStoreSegmentBinaryInfo_t binaryInfo;
            BootBinFormatStoreSegmentReservedInfo_t reservedInfo;
            BootBinFormatStoreSegmentPartitionInfo_t partitionInfo;
        };
    } segment[header.segments];
    uint8_t hash[header.hashSize];      // Header hash/checksum
    struct {
        BootBinFormatStoreRecordHeader_t recordHeader;
        uint8_t recordData[recordHeader.sectors * header.sectorSize];
        uint8_t recordHash[header.hashSize];
    } record[];
} BootBinFormatStore_t;

*/
//------------------------------------------------------------------------------

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif // __BOOT_DOWNLOAD_BIN_FORMAT_H
