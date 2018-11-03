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
#include <bootTerminalDebug.h>
#include <bootDisplayBios.h>
#include <bootTransportEdbg.h>
#include <bootTransportFileSys.h>
#include <bootDownloadBin.h>
#include <bootBlockBios.h>
#include <bootFileSystemFat.h>
#include <bootPci.h>
#include <bootString.h>
#include <ceddk.h>


//------------------------------------------------------------------------------

static BYTE Splice(const UINT16 wMAC [3], BYTE byte)
{
    byte = byte%6;
    return byte%2?LOBYTE(wMAC[2-(byte>>1)]):HIBYTE(wMAC[2-(byte>>1)]);
}

static char ToASCII(BYTE b)
{
    b = b&0xf;
    return b<10?b+'0':b+'7';
}


static
bool_t
TransportName(
    void * pContext,
    uint16_t mac[3],
    string_t buffer,
    size_t bufferSize
    )
{
#ifdef OLD_BOOTME_NAME
        static struct {
        uint32_t  code;
        cstring_t prefix;
    } s_prefixes[] = {
        { 0x004033, "AD" }, // Addtron
        { 0x004005, "LS" }, // LinkSys
        { 0x002078, "LS" }, // LinkSys
        { 0x00C0F0, "KS" }, // Kingston
        { 0x000000, "RT" }, // RealTek
        { 0x00900B, "RT" }, // RealTek
        { 0x00D0C9, "RT" }, // RealTek
        { 0x00E04C, "RT" }, // RealTek
        { 0x0050BA, "DL" }, // D-Link
        { 0x00A0CC, "NG" }, // Netgear
        { 0x0003FF, "DC" }, // DEC
        { 0x00800f, "NC" }  // NetChip
    };

    enum_t ix;
    uint32_t base, code;
    size_t count = 4;

    UNREFERENCED_PARAMETER(pContext);

    BootStringCchCopyA (buffer, bufferSize, "CEPC");
    code = ((mac[0] & 0x00FF) << 16) | (mac[0] & 0xFF00) | (mac[1] & 0x00FF);
    for (ix = 0; ix < dimof(s_prefixes); ix++)
        {
        if (s_prefixes[ix].code == code)
            {
            enum_t iy = 0;
            while (count < (bufferSize - 1))
                {
                if (s_prefixes[ix].prefix[iy] == '\0') break;
                buffer[count++] = s_prefixes[ix].prefix[iy++];
                }
            break;
            }
        }

    code = (mac[2] >> 8) | ((mac[2] & 0x00ff) << 8);
    base = 10000;
    while ((base > code) && (base != 0)) base /= 10;
    if (base == 0)
        {
        if (count < (bufferSize - 1)) buffer[count++] = '0';
        }
    else
        {
        while ((base > 0) && (count < (bufferSize - 1)))
            {
            buffer[count++] = (char)(code/base + '0');
            code %= base;
            base /= 10;
            }
        }

    while (count < bufferSize) buffer[count++] = '\0';

#else
    size_t count;
    UNREFERENCED_PARAMETER(pContext);
    if (bufferSize > 16)
        bufferSize = 16; // "PC-ffffffffffff\0" -> 15 chars plus null

    BootStringCchCopyA (buffer, bufferSize, "PC-");
    count = strlen(buffer);

    while (count < bufferSize - 1) {
        BYTE b = Splice(mac, (BYTE)((bufferSize - count)>>1) - 1);
        if ((bufferSize - count - 1) % 2 != 1)
            buffer[count++] = ToASCII(b>>4);
        buffer[count++] = ToASCII(b&0xf);
    }
    buffer[bufferSize-1] = 0;
#endif

    // Done
    return true;
}

//------------------------------------------------------------------------------

static
handle_t
CreateTerminal(
    BootLoader_t * pLoader,
    enum_t index
    )
{
    handle_t hDriver = NULL;
    UNREFERENCED_PARAMETER(pLoader);

    if (index != 0) goto cleanUp;

    hDriver = BootTerminalDebugInit();
    if (hDriver == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: CreateTerminal: "
            L"Terminal driver initialization failed!\r\n"
            ));
        goto cleanUp;
        }

cleanUp:
    return hDriver;
}

//------------------------------------------------------------------------------

static
handle_t
CreateTransport(
    BootLoader_t *pLoader,
    enum_t index
    )
{
    handle_t hDriver = NULL;
    const BootEdbgDriver_t *pDriver = NULL;
    void *pAddress = NULL;
    uint16_t mac[3] = { 0, 0, 0 };
    uint32_t ip;
    enum_t ix;

    // At this point we know if we can use the serial port for debug info
    InitSerialEcho(pLoader->comPort);

    // There is only transport driver 0 and we support only Edbg devices...
    if (index != 0) goto cleanUp;
    if (pLoader->bootDevice.type != DeviceTypeEdbg) goto cleanUp;

    // Look in device table for matching one
    for (ix = 0; ix < pLoader->devices; ix++)
        {
        const Device_t *pDevice = &pLoader->pDevices[ix];
        if (pLoader->bootDevice.ifc != pDevice->ifc) continue;
        switch (pDevice->ifc)
            {
            case IfcTypePci:
                {
                BootPciLocation_t pciLoc;
                uint32_t id;

                // Get device id on boot device location
                pciLoc.logicalLoc = pLoader->bootDevice.location;
                id = BootPciGetId(pciLoc);
                if (id == pDevice->id)
                    {
                    pDriver = pDevice->pDriver;
                    pAddress = BootPciMbarToVA(pciLoc, 0, false);
                    }
                }
                break;
            }
        if (pDriver != NULL) break;
        }

    // If we didn't find any, fail...
    if (pDriver == NULL) goto cleanUp;

    // Depending on DHCP flag set ip address    
    if ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_DHCP) != 0)
        ip = 0;
    else
        ip = pLoader->ipAddress;
    
    // Create EDBG Transport
    hDriver = BootTransportEdbgInit(
        pLoader, pDriver, TransportName, pAddress, pLoader->bootDevice.location, mac, ip
        );

cleanUp:
    BOOTMSG(ZONE_ERROR && (hDriver == NULL), (L"ERROR: CreateTransport: "
        L"Transport driver initialization failed!\r\n"
        ));
    return hDriver;
}

//------------------------------------------------------------------------------

static
handle_t
CreateDownload(
    BootLoader_t *pLoader,
    enum_t index
    )
{
    handle_t hDriver = NULL;
    handle_t hTransport;

    if (index != 0) goto cleanUp;

    hTransport = OEMBootCreateDriver(pLoader, BOOT_DRIVER_CLASS_TRANSPORT, 0);
    if (hTransport == NULL) goto cleanUp;

    hDriver = BootDownloadBinInit(pLoader, hTransport);
    if (hDriver == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: CreateDownload: "
            L"Download driver initialization failed!\r\n"
            ));
        goto cleanUp;
        }

cleanUp:
    return hDriver;
}

//------------------------------------------------------------------------------

static
handle_t
CreateDisplay(
    BootLoader_t * pLoader,
    enum_t index
    )
{
    handle_t hDriver = NULL;
    UNREFERENCED_PARAMETER(pLoader);

    if (index != 0) goto cleanUp;

    hDriver = BootDisplayBiosInit();
    if (hDriver == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: CreateDisplay: "
            L"Download driver initialization failed!\r\n"
            ));
        goto cleanUp;
        }

cleanUp:
    return hDriver;
}

//------------------------------------------------------------------------------

static
handle_t
CreateBlock(
    BootLoader_t *pLoader,
    enum_t index
    )
{
    handle_t hDriver = NULL;

    if (index != 0) goto cleanUp;

    hDriver = BootBlockBiosInit(pLoader->driveId);
    if (hDriver == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: CreateBlock: "
            L"Block driver initialization failed!\r\n"
            ));
        goto cleanUp;
        }

cleanUp:
    return hDriver;
}

//------------------------------------------------------------------------------

static
handle_t
CreateFileSys(
    BootLoader_t *pLoader,
    enum_t index
    )
{
    handle_t hDriver = NULL;
    handle_t hBlock;

    if (index != 0) goto cleanUp;

    hBlock = OEMBootCreateDriver(pLoader, BOOT_DRIVER_CLASS_BLOCK, 0);
    if (hBlock == NULL) goto cleanUp;
    
    hDriver = BootFileSystemFatInit(hBlock, 0);
    if (hDriver == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: CreateFileSys: "
            L"FAT file system driver initialization failed!\r\n"
            ));
        goto cleanUp;
        }

cleanUp:
    return hDriver;
}
//------------------------------------------------------------------------------

handle_t
OEMBootCreateDriver(
    void *pContext,
    enum_t classId,
    enum_t index
    )
{
    handle_t hDriver = NULL;
    BootLoader_t *pLoader = pContext;


    BOOTMSG(ZONE_FUNC, (
        L"+OEMBootCreateDriver(0x%08x, 0x%08x, %d)\r\n", pContext,
        classId, index
        ));

    switch (classId)
        {
        case BOOT_DRIVER_CLASS_TERMINAL:
            hDriver = CreateTerminal(pLoader, index);
            break;
        case BOOT_DRIVER_CLASS_TRANSPORT:
            hDriver = CreateTransport(pLoader, index);
            break;
        case BOOT_DRIVER_CLASS_DOWNLOAD:
            hDriver = CreateDownload(pLoader, index);
            break;
        case BOOT_DRIVER_CLASS_DISPLAY:
            hDriver = CreateDisplay(pLoader, index);
            break;
        case BOOT_DRIVER_CLASS_BLOCK:
            hDriver = CreateBlock(pLoader, index);
            break;
        case BOOT_DRIVER_CLASS_FILESYSTEM:
            hDriver = CreateFileSys(pLoader, index);
            break;
        }

    BOOTMSG(ZONE_FUNC, (L"-OEMBootCreateDriver = 0x%08x\r\n", hDriver));
    return hDriver;
}

//------------------------------------------------------------------------------

