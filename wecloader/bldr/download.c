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
#include "bldr.h"
#include <bootBlockUtils.h>
#include <bootTransportEdbg.h>


//------------------------------------------------------------------------------

static
bool_t
RamImage(
    BootLoader_t *pLoader,
    handle_t hDownload
    )
{
    bool_t rc = false;
    BootDownloadRamImageInfo_t info;
    void *pAddress, *pData;
    uint32_t address, addressSize, dataSize;

    // Get image information (at this moment only start and size is known
    if (!BootDownloadRamImageInfo(hDownload, &info)) goto cleanUp;
    address = BootImageVAtoPA((void*)info.address);
    if ((address < 0x00200000) || ((address + info.size) > pLoader->ramTop))
        {
        BOOTMSG(ZONE_ERROR, (
            L"Downloaded Image doesn't fit to this device memory!\r\n"
            ));
        goto cleanUp;
        }
    
    // Continue get chunk of image data while there is some...
    while (BootDownloadGetData(
            hDownload, &pAddress, &addressSize, &pData, &dataSize
            ))
        {
        if (pData == NULL)
            {
            pLoader->runAddress = BootImageVAtoPA((void*)dataSize);
            rc = true;
            break;
            }
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------


// Gets the Passive Kitl setting from the download driver (which got it from Visual Studio/Platform Builder) 
// Ultimately this setting gets passed on to the BOOTARGS for use during CE during startup.
static
bool_t
GetOSConfigSettings(
    BootLoader_t* pLoader,
    handle_t hDownload
    )
{
    bool_t bPassiveKitl = false;

    // Get the passive KITL setting recieved from Platform Builder
    if( BootDownloadGetOSInfo(hDownload, 
            BOOT_DOWNLOAD_GET_OS_INFO_TYPE_PASSIVE_KITL, &bPassiveKitl,
            sizeof (bPassiveKitl)))
        {
        if( bPassiveKitl )
            {
                pLoader->kitlFlags |= BOOT_TRANSPORT_EDBG_FLAG_PASSIVE;
            }
        }
            
    // Get the KitlTransportMediaType from Platform Builder
    pLoader->KitlTransportMediaType = 0;
    BootDownloadGetOSInfo(hDownload, 
            BOOT_DOWNLOAD_GET_OS_INFO_TYPE_KITL_TRANSPORT,
            &(pLoader->KitlTransportMediaType),
            sizeof(pLoader->KitlTransportMediaType));
       

    return true;        
}


//------------------------------------------------------------------------------

static
bool_t
StoreImage(
    BootLoader_t* pLoader,
    handle_t hDownload
    )
{
    bool_t rc = false;
    handle_t hBlock = NULL, hSegment = NULL;
    BootDownloadStoreInfo_t imageInfo;
    uint32_t segment = (uint32_t)-1;
    size_t sectors;


    // Get handle for store
    hBlock = OEMBootCreateDriver(pLoader, BOOT_DRIVER_CLASS_BLOCK, 0);
    if (hBlock == NULL) goto cleanUp;

    // Format store for image
    BOOTMSG(ZONE_INFO, (L"Formating storage for image...\r\n"));
    if (!BootBlockFormatForStoreDownload(hBlock, hDownload)) goto cleanUp;
    BOOTMSG(ZONE_INFO, (L"Format done\r\n"));

    // We will need image info later...
    if (!BootDownloadStoreImageInfo(hDownload, &imageInfo)) goto cleanUp;

    // Continue get chunk of image data while we
    sectors = 0;
    while (sectors < imageInfo.sectors)
        {
        BootDownloadStoreRecordInfo_t *pInfo;
        uint8_t *pData;
        size_t infoSize, dataSize;

        // Get next record
        if (!BootDownloadGetData(
                hDownload, &pInfo, &infoSize, &pData, &dataSize
                ))
            {
            BOOTMSG(ZONE_ERROR, (L"DownloadStoreImage: "
                L"Unexpected image end (%d, %d)!\r\n",
                sectors, imageInfo.sectors
                ));
            goto cleanUp;
            }

        // Check for info consistency && end of image
        if (infoSize != sizeof(*pInfo)) goto cleanUp;
        // If it is new segment we have to re-open store
        if (segment != pInfo->segment)
            {
            BootDownloadStoreSegmentInfo_t *pSegment;
            if (segment != -1)
                {
                BootBlockClose(hSegment);
                hSegment = NULL;
                }
            segment = pInfo->segment;
            pSegment = &imageInfo.pSegment[pInfo->segment];
            switch (pSegment->type)
                {
                case BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_REGION:
                    hSegment = BootBlockOpenReservedRegion(
                        hBlock, pSegment->name
                        );
                    if (hSegment == NULL)
                        {
                        BOOTMSG(ZONE_ERROR, (L"DownloadStoreImage: "
                            L"Failed open reserved region '%hs'!\r\n",
                            pSegment->name
                            ));
                        goto cleanUp;
                        }
                    break;
                case BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_PARTITION:
                    hSegment = BootBlockOpenPartition(
                            hBlock, pSegment->fileSystem, 0
                            );
                    if (hSegment == NULL)
                        {
                        BOOTMSG(ZONE_ERROR, (L"DownloadStoreImage: "
                            L"Failed open partition 0x%02x!\r\n",
                            pSegment->fileSystem
                            ));
                        goto cleanUp;
                        }
                    break;
                default:
                    goto cleanUp;
                }
            }

        // Write
        if (!BootBlockWrite(hSegment, pInfo->sector, pInfo->sectors, pData))
            goto cleanUp;

        // We get some sectors...
        sectors += pInfo->sectors;
        }

    // Done
    BOOTMSG(ZONE_INFO, (L"Download & Flash Done\r\n"));
    rc = true;
    pLoader->formatUserStore = true;


    {
        // make sure there is a config block on disk
        BootLoader_t CurrentLoader;
        memcpy(&CurrentLoader, pLoader, sizeof(BootLoader_t));
        if (!ReadConfigFromDisk(&CurrentLoader)) {
            // there wasn't - write one
            DefaultConfig(&CurrentLoader);
            WriteConfigToDisk(&CurrentLoader);
        }
    }

cleanUp:
    BootBlockClose(hSegment);
    BootBlockDeinit(hBlock);
    return rc;
}

//------------------------------------------------------------------------------

enum_t
BootLoaderDownload(
    BootLoader_t* pLoader
    )
{
    enum_t rc = (enum_t)BOOT_STATE_FAILURE;
    handle_t hDownload;
    enum_t type;


    // Open download driver
    hDownload = OEMBootCreateDriver(pLoader, BOOT_DRIVER_CLASS_DOWNLOAD, 0);
    if (hDownload == NULL) goto cleanUp;

    // Get image type & continue download
    type = BootDownloadImageType(hDownload);
    switch (type)
        {
        case BOOT_DOWNLOAD_IMAGE_JUMP:
            if(!GetOSConfigSettings(pLoader, hDownload)) goto cleanUp;
            rc = BOOT_STATE_RUN;
            break;
        case BOOT_DOWNLOAD_IMAGE_RAM:
            if (!RamImage(pLoader, hDownload)) goto cleanUp;
            if(!GetOSConfigSettings(pLoader, hDownload)) goto cleanUp;         
            rc = BOOT_STATE_RUN;
            break;
        case BOOT_DOWNLOAD_IMAGE_STORE:
            if (!StoreImage(pLoader, hDownload)) goto cleanUp;
            rc = BOOT_STATE_PRELOAD;
            break;
        }

cleanUp:
    BootDownloadDeinit(hDownload);
    return rc;
}

//------------------------------------------------------------------------------

