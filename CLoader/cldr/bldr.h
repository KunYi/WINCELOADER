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

#include <boot.h>

#define VERSION_MAJOR           1
#define VERSION_MINOR           3

//------------------------------------------------------------------------------

#define IMAGE_SHARE_BOOT_ARGS_PA            0x001FF000
#define IMAGE_SHARE_BOOT_ARGS_PTR_PA        0x001FFFFC

//------------------------------------------------------------------------------

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

typedef struct BootLoader_t {

    enum_t   driveId;                   // Boot driver BIOS id

    uint32_t ramTop;                    // Top of RAM
    uint32_t videoRam;                  // Video RAM
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

	WORD    wVideoMode;
    size_t  displayWidth;               // Display mode for Windows CE
    size_t  displayHeight;
    size_t  displayBpp;

    size_t  displayPhysicalWidth;
    size_t  displayPhysicalHeight;

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

//enum_t
//BootLoaderPowerOn(
//    BootLoader_t* pLoader
//    );
//
//enum_t
//BootLoaderConfig(
//    BootLoader_t* pLoader
//    );
//
//enum_t
//BootLoaderDownload(
//    BootLoader_t* pLoader
//    );
//
//enum_t
//BootLoaderPreLoad(
//    BootLoader_t* pLoader
//    );
//
//bool_t
//BootLoaderLoadUldr(
//    BootLoader_t* pLoader
//    );
//
//bool_t
//BootLoaderLoadOs(
//    BootLoader_t* pLoader
//    );
//
////------------------------------------------------------------------------------
//    
//bool_t
//WriteConfigToDisk(
//    BootLoader_t *pLoader
//    );
//    
//bool_t
//ReadConfigFromDisk(
//    BootLoader_t *pLoader
//    );
//    
//void
//DefaultConfig(
//    BootLoader_t *pLoader
//    );

//------------------------------------------------------------------------------

void
InitSerialEcho(
    uint8_t comPort
    );

#endif // __BLDR_H
