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
#ifndef __BLDR_H
#define __BLDR_H

//------------------------------------------------------------------------------

#include "include/boot.h"
//------------------------------------------------------------------------------

#define IMAGE_SHARE_BOOT_ARGS_PA            0x001FFF00
#define IMAGE_SHARE_BOOT_ARGS_PTR_PA        0x001FFFFC

#define VERSION_MAJOR           1
#define VERSION_MINOR           3

#define KTS_NONE         63         // no transport
#define KTS_PASSIVE_MODE 0x40       // flag for passive kitl mode

#define  IMAGE_EXTRA_MEM_LENGTH	0x400000


#define WINCE_OSIMAGE_NAME      "NK.BIN"

//------------------------------------------------------------------------------
#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

//////////////////////////////////////////
//
// BIN image and record headers
//
typedef struct          // Image header (one per BIN image)
{
    UCHAR SyncBytes[7];
    unsigned long ImageAddr;
    unsigned long ImageLen;
} IMAGEHDR, *PIMAGEHDR;

typedef struct          // Record header (one per section in image)
{
    unsigned long RecordAddr;
    unsigned long RecordLen;
    unsigned long RecordChksum;
} RECORDHDR, *PRECORDHDR;

#pragma pack(pop)   /* restore original alignment from stack */

typedef enum DeviceType_e {
    DeviceTypeStore = 1,
    DeviceTypeEdbg
} DeviceType_e;

typedef enum IfcType_e {
    IfcTypeUndefined = -1,
    IfcTypeInternal = 0,
    IfcTypePci = 5
} IfcType_e;

typedef struct Device_t {
    wcstring_t name;
    enum_t type;
    enum_t ifc;
    uint32_t id;
    const void *pDriver;
} Device_t;

typedef struct DeviceLocation_t {
    enum_t type;
    enum_t ifc;
    enum_t busNumber;
    uint32_t location;
} DeviceLocation_t;

//------------------------------------------------------------------------------
#define BOOT_CONFIG_SIGNATURE           "BootCfg\x0A"
#define BOOT_CONFIG_VERSION             1
#define OAL_ARGS_UPDATEMODE             1

//------------------------------------------------------------------------------
#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct BootConfig_t {

    uint8_t signature[8];               // Configuration signature
    uint32_t version;                   // Configuration version

    uint32_t ipAddress;                 // Boot/Kitl IP address
    flags_t  kitlFlags;                 // Kitl flags

    DeviceLocation_t bootDevice;        // Boot device location
    DeviceLocation_t kitlDevice;        // Kitl device location

    size_t  displayWidth;
    size_t  displayHeight;
    size_t  displayBpp;

    size_t  displayLogicalWidth;
    size_t  displayLogicalHeight;

    uint8_t  comPort;                   // Debug port
    uint8_t  baudDivisor;               // Debug port

    flags_t  imageUpdateFlags;

} BootConfig_t;
#pragma pack(pop)   /* restore original alignment from stack */

//------------------------------------------------------------------------------

enum BootModeFlags_e {
    BootModeDownload = (1 << 0),
    BootModeCleanOs  = (1 << 1),
    BootModeLoadUldr = (1 << 2),
    BootModeRamImage = (1 << 3)
};

//------------------------------------------------------------------------------

enum BootState_e {
    BOOT_STATE_CONFIG = (BOOT_STATE_OEM + 0),
    BOOT_STATE_DOWNLOAD,
    BOOT_STATE_PRELOAD,
    BOOT_STATE_LOAD_ULDR,
    BOOT_STATE_LOAD_OS
};

//------------------------------------------------------------------------------

#define UEFI_BLSTATUS_OK                     0
#define UEFI_BLSTATUS_ERROR                  1
#define UEFI_BLSTATUS_NO_INI_FILE            2
#define UEFI_BLSTATUS_NO_VIDEO_MODE_FOUND    3
#define UEFI_BLSTATUS_NO_USER_SELECTION      4

typedef unsigned long UEFI_BLSTATUS;
//------------------------------------------------------------------------------

typedef struct BootLoader_t {

    enum_t   driveId;                   // Boot driver BIOS id

    uint64_t ramTop;                    // Top of RAM
    uint64_t videoRam;                  // Video RAM
    bool_t   ramestimated;              // the RAM value was not retrieved from the bios

    uint8_t  eddBiosMajor;              // Bios EDD services version supported
    uint8_t  eddBiosMinor;
    uint16_t eddBiosIfcs;               // Bios EDD interfce support bitmap

    uint8_t  apmBiosMajor;              // Bios APM version supported
    uint8_t  apmBiosMinor;
    uint8_t  apmBiosFlags;

    uint8_t  pciBiosMajor;              // Bios Pci version supported
    uint8_t  pciBiosMinor;
    uint8_t  pciLastBus;                // Last Pci Bus in system
    uint8_t  pciBusMode;                // Special cycle & config flags

    uint8_t  vbeBiosMajor;              // Bios Vesa version supported
    uint8_t  vbeBiosMinor;

    size_t   devices;                   // Supported devices info
    const Device_t *pDevices;

    uint32_t runAddress;                // Run address

    uint32_t ipAddress;                 // Boot/Kitl IP address
    flags_t  kitlFlags;                 // Kitl flags
    uint32_t KitlTransportMediaType;    // Media type recieved from Platform Builder

    DeviceLocation_t bootDevice;        // Boot device location
    DeviceLocation_t kitlDevice;        // Kitl device location

    size_t  displayWidth;               // Display mode for Windows CE
    size_t  displayHeight;
    size_t  displayBpp;

    size_t  displayLogicalWidth;
    size_t  displayLogicalHeight;

    uint8_t  comPort;                   // Debug port
    uint8_t  baudDivisor;

    flags_t  imageUpdateFlags;

    bool_t formatUserStore;
} BootLoader_t;

//------------------------------------------------------------------------------

void
BootArgsInit(
    bool_t force
    );

void*
BootArgsQuery(
    BootLoader_t *pLoader,
    enum_t type
    );

//------------------------------------------------------------------------------

enum_t
BootLoaderPowerOn(
    BootLoader_t* pLoader
    );

enum_t
BootLoaderConfig(
    BootLoader_t* pLoader
    );

enum_t
BootLoaderDownload(
    BootLoader_t* pLoader
    );

enum_t
BootLoaderPreLoad(
    BootLoader_t* pLoader
    );

bool_t
BootLoaderLoadUldr(
    BootLoader_t* pLoader
    );

enum_t
BootLoaderLoadOs(
    BootLoader_t* pLoader
    );

wchar_t
ReadKeyboardCharacter(
    );
//------------------------------------------------------------------------------

void
InitSerialEcho(
    uint8_t comPort
    );

BOOLEAN
AllocateImageBuffer(
    UINT32 *pImageStartingAddr,
    UINT32 ImageLength
    );

wchar_t
ReadKeyboardCharacter(
    );

#endif // __BLDR_H
