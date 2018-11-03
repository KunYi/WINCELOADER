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
#include <bootDownloadNotify.h>
#include <bootTransportEdbgNotify.h>

//------------------------------------------------------------------------------

void
OEMBootNotify(
    void *pContext,
    enum_t notifyCode,
    void *pNotifyInfo,
    size_t notifyInfoSize
    )
{
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(notifyInfoSize);

    switch (notifyCode)
        {
        case BOOT_NOTIFY_TRANSPORT_EDBG_DHCP_DISCOVER:
            {
            BootNotifyTransportEdbgDhcpDiscover_t *pInfo = pNotifyInfo;

            BootLog(
                L"Send DHCP Discover Message (attempt %d)\r\n", 
                pInfo->attempt
                );
            }
            break;
        case BOOT_NOTIFY_TRANSPORT_EDBG_DHCP_BOUND:
            {
            BootNotifyTransportEdbgDhcpBound_t *pInfo = pNotifyInfo;
        
            BootLog(
                L"Got Response from DHCP server %d.%d.%d.%d with address "
                L"%d.%d.%d.%d\r\n",
                ((uint8_t *)&pInfo->serverIp)[0], 
                ((uint8_t *)&pInfo->serverIp)[1],
                ((uint8_t *)&pInfo->serverIp)[2],
                ((uint8_t *)&pInfo->serverIp)[3],
                ((uint8_t *)&pInfo->clientIp)[0],
                ((uint8_t *)&pInfo->clientIp)[1],
                ((uint8_t *)&pInfo->clientIp)[2],
                ((uint8_t *)&pInfo->clientIp)[3]
                );
            }
            break;
        case BOOT_NOTIFY_TRANSPORT_EDBG_ARP_REQUEST:
            {
            BootNotifyTransportEdbgArpRequest_t *pInfo = pNotifyInfo;
        
            BootLog(
                L"Send ARP request for address %d.%d.%d.%d\r\n",
                ((uint8_t *)&pInfo->ip)[0], ((uint8_t *)&pInfo->ip)[1],
                ((uint8_t *)&pInfo->ip)[2], ((uint8_t *)&pInfo->ip)[3]
                );
            }
            break;
        case BOOT_NOTIFY_TRANSPORT_EDBG_ARP_RESPONSE:
            {
            BootNotifyTransportEdbgArpResponse_t *pInfo = pNotifyInfo;

            if (pInfo->timeout)
                {
                BootLog(
                    L"No ARP response, assuming ownership of %d.%d.%d.%d\r\n",
                    ((uint8_t *)&pInfo->ip)[0], ((uint8_t *)&pInfo->ip)[1],
                    ((uint8_t *)&pInfo->ip)[2], ((uint8_t *)&pInfo->ip)[3]
                    );
                }
            else
                {
                BootLog(
                    L"Got ARP response, rejecting ownership of %d.%d.%d.%d\r\n",
                    ((uint8_t *)&pInfo->ip)[0], ((uint8_t *)&pInfo->ip)[1],
                    ((uint8_t *)&pInfo->ip)[2], ((uint8_t *)&pInfo->ip)[3]
                    );
                }
            }
            break;
        case BOOT_NOTIFY_TRANSPORT_EDBG_BOOTME:
            {
            BootNotifyTransportEdbgBootMe_t *pInfo = pNotifyInfo;
        
            BootLog(
                L"Send BOOTME Message (device name %hs, attempt %d)\r\n",
                pInfo->name, pInfo->attempt
                );
            }
            break;
        case BOOT_NOTIFY_TRANSPORT_EDBG_SERVER_ACK:
            {
            BootNotifyTransportEdbgServerAck_t *pInfo = pNotifyInfo;
        
            BootLog(
                L"Accept download from %d.%d.%d.%d port %d\r\n",
                ((uint8_t *)&pInfo->serverIp)[0], 
                ((uint8_t *)&pInfo->serverIp)[1],
                ((uint8_t *)&pInfo->serverIp)[2],
                ((uint8_t *)&pInfo->serverIp)[3],
                pInfo->serverPort
                );
            }
            break;
        case BOOT_NOTIFY_DOWNLOAD_START:
            {
            BootNotifyDownloadStart_t *pInfo = pNotifyInfo;

            switch (pInfo->imageType)
                {
                case BOOT_DOWNLOAD_IMAGE_RAM:
                    BootLog(
                        L"Start download RAM image, size 0x%08x bytes\r\n", 
                        pInfo->imageSize
                        );
                    break;
                case BOOT_DOWNLOAD_IMAGE_STORE:
                    BootLog(
                        L"Start download STORE image, size %d bytes\r\n", 
                        pInfo->imageSize
                        );
                    break;
                }
            }
            break;
        case BOOT_NOTIFY_DOWNLOAD_CONTINUE:
            {
            static uint32_t shareDone = 0;
            BootNotifyDownloadContinue_t *pInfo = pNotifyInfo;
            if (pInfo->shareDone != shareDone)
                {
                shareDone = pInfo->shareDone;
                BootLog(L"%3d%%\b\b\b\b", pInfo->shareDone);
                }
            }
            break;
        case BOOT_NOTIFY_DOWNLOAD_DONE:
            BootLog(L"Download Done\r\n");
            break;
        }            

}

//------------------------------------------------------------------------------
