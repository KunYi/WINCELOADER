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
#include <bootTerminalUtils.h>
#include <bootTransportEdbg.h>
#include <bootBios.h>
#include <bootPci.h>
#include <ceddk.h>
#include <bootDisplayBios.h>
#include <bootBlockBios.h>
#include <bootFileSystemFat.h>
#include <oal_args.h>

extern uint32_t g_dwOEMUnsupportedNICs[];
extern uint32_t g_dwOEMUnsupportedNICCount;

//------------------------------------------------------------------------------

__inline
static
uint8_t
SwapNibbles(
    uint8_t value
    )
{
    return (value >> 4)|(value << 4);
}

//------------------------------------------------------------------------------

static
void
MenuHeader(
    handle_t hTerminal,
    wcstring_t format,
    ...
    )
{
    va_list pArgList;
    wchar_t line[80];
    size_t ix;

    va_start(pArgList, format);
    BootTerminalPrintf(hTerminal, L"\r\n");
    for (ix = 0; ix < dimof(line) - 1; ix++) line[ix] = L'-';
    line[dimof(line) - 1] = L'\0';
    BootTerminalPrintf(hTerminal, L"%s\r\n", line);
    BootTerminalVPrintf(hTerminal, format, pArgList);
    BootTerminalPrintf(hTerminal, L"%s\r\n\r\n", line);
    va_end(pArgList);
}

//------------------------------------------------------------------------------

static
wcstring_t
DevLocToString(
    BootLoader_t *pLoader,
    const DeviceLocation_t *pDevLoc
    )
{
    wcstring_t info = NULL;
    static wchar_t buffer[64];


    switch (pDevLoc->type)
        {
        case DeviceTypeEdbg:
            switch (pDevLoc->ifc)
                {
                case IfcTypePci:
                    {
                    BootPciLocation_t pciLoc;
                    uint32_t id;
                    enum_t ix;

                    pciLoc.logicalLoc = pDevLoc->location;
                    id = BootPciGetId(pciLoc);
                    for (ix = 0; ix < pLoader->devices; ix++)
                        {
                        const Device_t *pDevice = &pLoader->pDevices[ix];
                        if (pDevLoc->ifc != pDevice->ifc) continue;
                        if (id == pDevice->id)
                            {
                            BootLogSPrintf(
                                buffer, dimof(buffer),
                                L"%s at PCI bus %d dev %d fnc %d",
                                pDevice->name, pciLoc.bus, pciLoc.dev,
                                pciLoc.fnc
                                );
                            info = buffer;
                            break;
                            }
                        }
                    }
                    break;
                }
            break;
        case DeviceTypeStore:
            info = L"Boot Drive";
            break;
        }

    return info;
}

//------------------------------------------------------------------------------

static
void
UpdateDisplayMode(
    BootLoader_t *pLoader,
    size_t width,
    size_t height,
    size_t bpp
    )
{
    handle_t hDisplay = NULL;
    enum_t mode = (enum_t)-1;


    pLoader->displayWidth = 0;
    pLoader->displayHeight = 0;
    pLoader->displayBpp = 0;

    hDisplay = OEMBootCreateDriver(pLoader, BOOT_DRIVER_CLASS_DISPLAY, 0);
    if (hDisplay == NULL) goto cleanUp;

    if (!BootDisplayModeQuery(
            hDisplay, &mode, &width, &height, &bpp
            )) goto cleanUp;

    pLoader->displayWidth = width;
    pLoader->displayHeight = height;
    pLoader->displayBpp = bpp;

cleanUp:
    BootDisplayDeinit(hDisplay);
}

//------------------------------------------------------------------------------

static
void
SelectDevice(
    BootLoader_t *pLoader,
    handle_t hTerminal,
    bool_t kitlDevice
    )
{
    DeviceLocation_t devLoc[9], actualDevLoc;
    enum_t ix, iy;
    wchar_t key;
    wcstring_t name;


    MenuHeader(
        hTerminal, L" Select %s Device\r\n", kitlDevice ? L"KITL" : L"Boot"
        );

    actualDevLoc = kitlDevice ? pLoader->kitlDevice : pLoader->bootDevice;

    ix = 0;
    for (iy = 0; iy < pLoader->devices; iy++)
        {
        const Device_t *pDevice = &pLoader->pDevices[iy];
        switch (pDevice->type)
            {
            case DeviceTypeEdbg:
                switch (pDevice->ifc)
                    {
                    case IfcTypePci:
                        {
                        BootPciLocation_t pciLoc;
                        pciLoc.logicalLoc = 0;
                        do
                            {
                            if (BootPciGetId(pciLoc) == pDevice->id)
                                {
                                devLoc[ix].type = DeviceTypeEdbg;
                                devLoc[ix].ifc = IfcTypePci;
                                devLoc[ix].busNumber = 0;
                                devLoc[ix].location = pciLoc.logicalLoc;
                                name = DevLocToString(pLoader, &devLoc[ix]);
                                BootTerminalPrintf(
                                    hTerminal, L" [%d] %s\r\n", ix + 1, name
                                    );
                                ix++;
                                }
                            BootPciNextLocation(&pciLoc);
                            if (ix >= dimof(devLoc)) break;
                            }                            
                        while (pciLoc.bus <= pLoader->pciLastBus);
                        }
                        break;
                    }
                break;
            case DeviceTypeStore:
                if (kitlDevice) break;
                devLoc[ix].type = DeviceTypeStore;
                devLoc[ix].ifc = (enum_t)IfcTypeUndefined;
                devLoc[ix].busNumber = 0;
                devLoc[ix].location = 0;
                name = DevLocToString(pLoader, &devLoc[ix]);
                BootTerminalPrintf(hTerminal, L" [%d] %s\r\n", ix + 1, name);
                ix++;
                break;
            }
        if (ix >= dimof(devLoc)) break;
        }

    BootTerminalPrintf(
        hTerminal, L"\r\n Selection (actual %s): ",
        DevLocToString(pLoader, &actualDevLoc)
        );

    // Get selection
    do
        {
        key = BootTerminalReadChar(hTerminal);
        if (key == L'\0') continue;
        }
    while ((key < L'1') || (key >= (L'1' + ix)));

    // Print out selection character
    BootTerminalPrintf(hTerminal, L"%c\r\n", key);

    // Save new device
    actualDevLoc = devLoc[key - L'1'];
    if (kitlDevice)
        {
        pLoader->kitlDevice = actualDevLoc;
        }
    else
        {
        pLoader->bootDevice = actualDevLoc;
        }

    // Print info...
    BootTerminalPrintf(
        hTerminal, L" %s Device is set to %s.\r\n",
        kitlDevice ? L"KITL" : L"Boot", DevLocToString(pLoader, &actualDevLoc)
        );
}

//------------------------------------------------------------------------------

void
DefaultConfig(
    BootLoader_t *pLoader
    )
{
    BootPciLocation_t pciLoc;
    bool_t found = false;


    found = false;
    pciLoc.logicalLoc = 0;
    while (pciLoc.bus <= pLoader->pciLastBus)
        {
        uint32_t id = BootPciGetId(pciLoc);
        enum_t ix;
        uint32_t classcode = BootPciGetClassCodeAndRevId(pciLoc);
        classcode >>= 8; // discard revision id

        for (ix = 0; ix < pLoader->devices; ix++)
        {
            const Device_t *pDevice = &pLoader->pDevices[ix];
            if (pDevice->type != DeviceTypeEdbg) continue;
            if (pDevice->ifc != IfcTypePci) continue;
            if (id == pDevice->id)
            {
                found = true;
                break;
            }
        }
        if (found) break;
        if (classcode == CLASS_CODE_ETHERNET) {
            if (g_dwOEMUnsupportedNICCount < MAX_OEM_UNSUPPORTED_NICS)
            {
                // save id to display later.
                g_dwOEMUnsupportedNICs[g_dwOEMUnsupportedNICCount++] = id;
            }
        }

        BootPciNextLocation(&pciLoc);
        }

    if (found)
    {
        pLoader->bootDevice.type = DeviceTypeEdbg;
        pLoader->bootDevice.ifc = IfcTypePci;
        pLoader->bootDevice.busNumber = 0;
        pLoader->bootDevice.location = pciLoc.logicalLoc;
    }

    pLoader->formatUserStore = false;
    pLoader->ipAddress = 0;
    pLoader->kitlFlags  = BOOT_TRANSPORT_EDBG_FLAG_ENABLED | BOOT_TRANSPORT_EDBG_FLAG_VMINI;
    if (pLoader->ipAddress == 0)
        {
        pLoader->kitlFlags |= BOOT_TRANSPORT_EDBG_FLAG_DHCP;
        }
    pLoader->kitlDevice = pLoader->bootDevice;

    pLoader->comPort = 1;
    pLoader->baudDivisor = 3;

    UpdateDisplayMode(pLoader, 640, 480, 16);
}

//------------------------------------------------------------------------------

bool_t
ReadConfigFromDisk(
    BootLoader_t *pLoader
    )
{
    bool_t rc = false;
    handle_t hBlock = NULL, hSection = NULL;
    size_t sectorSize;
    size_t sector, sectors;
    uint8_t *pSector = NULL;
    BootConfig_t *pBootConfig;

    // Open boot block driver
    hBlock = OEMBootCreateDriver(pLoader, BOOT_DRIVER_CLASS_BLOCK, 0);
    if (hBlock == NULL) goto cleanUp;

    sectorSize = BootBlockSectorSize(hBlock);
    if (sectorSize == 0) goto cleanUp;

    // Open active partition
    hSection = BootBlockOpenPartition(hBlock, 0, 0);
    if (hSection == NULL) goto cleanUp;

    // Get data sector & number of data sectors
    if (!BootBlockDataSectors(hSection, &sector, &sectors)) goto cleanUp;

    // Allocate memory for sector
    pSector = BootAlloc(sectorSize);
    if (pSector == NULL) goto cleanUp;
    pBootConfig = (BootConfig_t*)pSector;

    // Find sector with parameters
    while (sectors > 0)
        {
        if (!BootBlockRead(hSection, sector, 1, pSector)) goto cleanUp;
        // Is this configuration sector?
        if (memcmp(pSector, "BLDRCFG\x0A", 8) == 0) break;
        // Is this boot loader image sector?
        if (memcmp(pSector, "B000FF\x0A", 7) == 0) goto cleanUp;
        // Move to next sector
        sector++;
        sectors--;
        }
    if (sectors == 0) goto cleanUp;

    // Check if this is consistent version
    if (pBootConfig->version != BOOT_CONFIG_VERSION) goto cleanUp;

    // Update loader context
    pLoader->ipAddress = pBootConfig->ipAddress;
    pLoader->kitlFlags = pBootConfig->kitlFlags;
    pLoader->bootDevice = pBootConfig->bootDevice;
    pLoader->kitlDevice = pBootConfig->kitlDevice;

    UpdateDisplayMode(
        pLoader, pBootConfig->displayWidth, pBootConfig->displayHeight,
        pBootConfig->displayBpp
        );
    pLoader->displayLogicalWidth = pBootConfig->displayLogicalWidth;
    pLoader->displayLogicalHeight = pBootConfig->displayLogicalHeight;

    pLoader->comPort = pBootConfig->comPort;
    pLoader->baudDivisor = pBootConfig->baudDivisor;

    pLoader->imageUpdateFlags = pBootConfig->imageUpdateFlags;

    // Done
    rc = true;

cleanUp:
    BootBlockClose(hSection);
    BootBlockDeinit(hBlock);
    BootFree(pSector);
    return rc;
}

//------------------------------------------------------------------------------

bool_t
WriteConfigToDisk(
    BootLoader_t *pLoader
    )
{
    bool_t rc = false;
    handle_t hBlock = NULL, hSection = NULL;
    size_t sectorSize;
    size_t sector, sectors;
    uint8_t *pSector = NULL;
    BootConfig_t *pBootConfig;


    // Open boot block driver
    hBlock = OEMBootCreateDriver(pLoader, BOOT_DRIVER_CLASS_BLOCK, 0);
    if (hBlock == NULL) goto cleanUp;

    sectorSize = BootBlockSectorSize(hBlock);
    if (sectorSize == 0) goto cleanUp;

    // Open active partition
    hSection = BootBlockOpenPartition(hBlock, 0, 0);
    if (hSection == NULL) goto cleanUp;

    // Get data sector & number of data sectors
    if (!BootBlockDataSectors(hSection, &sector, &sectors)) goto cleanUp;

    // Allocate memory for sector
    pSector = BootAlloc(sectorSize);
    if (pSector == NULL) goto cleanUp;
    pBootConfig = (BootConfig_t*)pSector;

    // Find sector with parameters
    while (sectors > 0)
        {
        if (!BootBlockRead(hSection, sector, 1, pSector)) goto cleanUp;
        // Is this configuration sector?
        if (memcmp(pSector, "BLDRCFG\x0A", 8) == 0) break;
        // Is this boot loader image sector?
        if (memcmp(pSector, "B000FF\x0A", 7) == 0) goto cleanUp;
        // Move to next sector
        sector++;
        sectors--;
        }

    // Set configuration
    pBootConfig->version = BOOT_CONFIG_VERSION;
    pBootConfig->ipAddress = pLoader->ipAddress;
    pBootConfig->kitlFlags = pLoader->kitlFlags;
    pBootConfig->bootDevice = pLoader->bootDevice;
    pBootConfig->kitlDevice = pLoader->kitlDevice;
    pBootConfig->displayWidth = pLoader->displayWidth;
    pBootConfig->displayHeight = pLoader->displayHeight;
    pBootConfig->displayBpp = pLoader->displayBpp;
    pBootConfig->displayLogicalWidth = pLoader->displayLogicalWidth;
    pBootConfig->displayLogicalHeight = pLoader->displayLogicalHeight;
    pBootConfig->comPort = pLoader->comPort;
    pBootConfig->baudDivisor = pLoader->baudDivisor;
    pBootConfig->imageUpdateFlags = pLoader->imageUpdateFlags & ~OAL_ARGS_UPDATEMODE;
    
    // Write sector to disk
    if (!BootBlockWrite(hSection, sector, 1, pSector)) goto cleanUp;

    // Done
    rc = true;

cleanUp:
    BootBlockClose(hSection);
    BootBlockDeinit(hBlock);
    BootFree(pSector);
    return rc;
}

//------------------------------------------------------------------------------

static
void
NetworkSettings(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    flags_t flags = pLoader->kitlFlags;

    UNREFERENCED_PARAMETER(pActionMenu);

    BootTerminalPrintf(hTerminal, L"\r\n Network:\r\n");
    BootTerminalPrintf(
        hTerminal, L"  KITL state:    %s\r\n",
        ((flags & BOOT_TRANSPORT_EDBG_FLAG_ENABLED) != 0) ?
        L"enabled" : L"disabled"
        );
    BootTerminalPrintf(
        hTerminal, L"  KITL mode:     %s\r\n",
        ((flags & BOOT_TRANSPORT_EDBG_FLAG_POLL) != 0) ? L"poll" : L"interrupt"
        );
    BootTerminalPrintf(
        hTerminal, L"  DHCP:          %s\r\n",
        ((flags & BOOT_TRANSPORT_EDBG_FLAG_DHCP) != 0) ?
        L"enabled" : L"disabled"
        );
    BootTerminalPrintf(
        hTerminal, L"  IP address:    %d.%d.%d.%d\r\n",
        ((uint8_t*)&pLoader->ipAddress)[0], ((uint8_t*)&pLoader->ipAddress)[1],
        ((uint8_t*)&pLoader->ipAddress)[2], ((uint8_t*)&pLoader->ipAddress)[3]
        );
    BootTerminalPrintf(
        hTerminal, L"  VMINI:         %s\r\n",
        ((flags & BOOT_TRANSPORT_EDBG_FLAG_VMINI) != 0) ?
        L"enabled" : L"disabled"
        );
}

//------------------------------------------------------------------------------

static
void
EnableKitl(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    bool_t enabled;

    UNREFERENCED_PARAMETER(pActionMenu);

    enabled = (pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_ENABLED) != 0;
    BootTerminalReadEnable(hTerminal, &enabled, L"KITL");
    if (enabled)
        pLoader->kitlFlags |= BOOT_TRANSPORT_EDBG_FLAG_ENABLED;
    else
        pLoader->kitlFlags &= ~BOOT_TRANSPORT_EDBG_FLAG_ENABLED;
}

//------------------------------------------------------------------------------

static
void
SetKitlMode(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    wchar_t key;

    bool_t poll = (pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_POLL) != 0;
    UNREFERENCED_PARAMETER(pActionMenu);

    if (poll)
        {
        BootTerminalWriteString(
            hTerminal, L" Set KITL to interrupt mode [y/-]: "
            );
        }
    else
        {
        BootTerminalWriteString(
            hTerminal, L" Set KITL to poll mode [y/-]: "
            );
        }

    do
        {
        key = BootTerminalReadChar(hTerminal);
        }
    while (key == '\0');
    BootTerminalPrintf(hTerminal, L"%c\r\n", key);

    if ((key == L'y') || (key == L'Y'))
        {
        if (poll)
            {
            pLoader->kitlFlags &= ~BOOT_TRANSPORT_EDBG_FLAG_POLL;
            BootTerminalWriteString(
                hTerminal, L" KITL set to interrupt mode\r\n"
                );
            }
        else
            {
            pLoader->kitlFlags |= BOOT_TRANSPORT_EDBG_FLAG_POLL;
            BootTerminalWriteString(
                hTerminal, L" KITL set to poll mode\r\n"
                );
        }
    }
}

//------------------------------------------------------------------------------

static
void
EnableDHCP(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    bool_t enabled = pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_DHCP;
    UNREFERENCED_PARAMETER(pActionMenu);
    BootTerminalReadEnable(hTerminal, &enabled, L"DHCP");
    if (enabled)
        pLoader->kitlFlags |= BOOT_TRANSPORT_EDBG_FLAG_DHCP;
    else
        pLoader->kitlFlags &= ~BOOT_TRANSPORT_EDBG_FLAG_DHCP;
}

//------------------------------------------------------------------------------

static
void
DeviceIp(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    uint32_t ip = pLoader->ipAddress;
    UNREFERENCED_PARAMETER(pActionMenu);
    if (BootTerminalReadIp4(hTerminal, &ip, L"Device"))
        {
        pLoader->ipAddress = ip;
        }
}

//------------------------------------------------------------------------------

static
void
EnableVMINI(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    bool_t enabled = pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_VMINI;
    UNREFERENCED_PARAMETER(pActionMenu);
    BootTerminalReadEnable(hTerminal, &enabled, L"VMINI");
    if (enabled)
        pLoader->kitlFlags |= BOOT_TRANSPORT_EDBG_FLAG_VMINI;
    else
        pLoader->kitlFlags &= ~BOOT_TRANSPORT_EDBG_FLAG_VMINI;
}

//------------------------------------------------------------------------------

static
void
BootDevice(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    UNREFERENCED_PARAMETER(pActionMenu);
    SelectDevice(pContext, hTerminal, false);
}

//------------------------------------------------------------------------------

static
void
KitlDevice(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    UNREFERENCED_PARAMETER(pActionMenu);
    SelectDevice(pContext, hTerminal, true);
}

//------------------------------------------------------------------------------

static
BootTerminalMenu_t
g_menuNetwork = {
    L"Network Settings", {
        { L'1', L"Show Current Settings", NetworkSettings, NULL },
        { L'2', L"Enable/disable KITL", EnableKitl, NULL },
        { L'3', L"KITL interrupt/poll mode", SetKitlMode, NULL },
        { L'4', L"Enable/disable DHCP", EnableDHCP, NULL },
        { L'5', L"Set IP address", DeviceIp, NULL },
        { L'6', L"Enable/disable VMINI", EnableVMINI, NULL },
        { L'0', L"Exit and Continue", NULL, NULL },
        { 0, NULL, NULL, NULL }
    }
};

//------------------------------------------------------------------------------

static
void
DisplaySettings(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    UNREFERENCED_PARAMETER(pActionMenu);


    BootTerminalPrintf(hTerminal, L"\r\n Display:\r\n");

    BootTerminalPrintf(
        hTerminal, L"  Resolution:    %d x %d x %d\r\n",
        pLoader->displayWidth, pLoader->displayHeight, pLoader->displayBpp
        );
    if (pLoader->displayLogicalWidth == 0)
        {
        BootTerminalPrintf(
            hTerminal, L"  Logical size:  Full Screen\r\n"
            );
        }
    else
        {
        BootTerminalPrintf(
            hTerminal, L"  Logical size:  %d x %d\r\n",
            pLoader->displayLogicalWidth, pLoader->displayLogicalHeight
            );
        }
}

//------------------------------------------------------------------------------

static
void
DisplayResolution(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    handle_t hDisplay;
    enum_t mode, bppMode, resMode;
    wchar_t key;
    uint32_t width, height, bpp;
    uint32_t bppOptions[4] = {0,0,0,0};
    enum_t resOptions[20];
    UNREFERENCED_PARAMETER(pActionMenu);


    hDisplay = OEMBootCreateDriver(pContext, BOOT_DRIVER_CLASS_DISPLAY, 0);
    if (hDisplay == NULL) goto cleanUp;

    MenuHeader(hTerminal, L" Select Desired Bits Per Pixel\r\n");

    mode = bppMode = 0;
    while (BootDisplayModeQuery(hDisplay, &mode, &width, &height, &bpp) && (bppMode < 4))
        {
        // Only support four bpp modes (8, 16, 24, 32) currently, and 20 resolutions for each
        if (!((bpp == bppOptions[0]) || 
              (bpp == bppOptions[1]) || 
              (bpp == bppOptions[2]) || 
              (bpp == bppOptions[3])))
            {
            key = L'a' + (wchar_t)bppMode;
            BootTerminalPrintf(
                hTerminal, L" [%c] %2d\r\n",
                key, bpp
                );

            bppOptions[bppMode++] = bpp;
            }

        mode++;
        }

    BootTerminalPrintf(
        hTerminal, L"\r\n Selection (actual %d bpp): ",
        pLoader->displayBpp
        );

    // Get selection
    do
        {
        key = BootTerminalReadChar(hTerminal);
        if (key == L'\0') continue;
        }
    while ((key < L'a') || (key >= (L'a' + bppMode)));

    // Print out selection character
    BootTerminalPrintf(hTerminal, L"%c\r\n", key);

    // User choosed this mode...
    bppMode = key - 'a';


    MenuHeader(hTerminal, L" Select Display Resolution\r\n");

    mode = resMode = 0;
    while (BootDisplayModeQuery(hDisplay, &mode, &width, &height, &bpp) && (resMode < 12))
        {

        if (bpp == bppOptions[bppMode])
            {
            key = L'a' + (wchar_t)resMode;
            BootTerminalPrintf(
                hTerminal, L" [%c] %4d x %4d x %2d\r\n",
                key, width, height, bpp
                );

            resOptions[resMode++] = mode;
            }

        mode++;
        }

    BootTerminalPrintf(
        hTerminal, L"\r\n Selection (actual %d x %d x %d): ",
        pLoader->displayWidth, pLoader->displayHeight, pLoader->displayBpp
        );

    // Get selection
    do
        {
        key = BootTerminalReadChar(hTerminal);
        if (key == L'\0') continue;
        }
    while ((key < L'a') || (key >= (L'a' + resMode)));

    // Print out selection character
    BootTerminalPrintf(hTerminal, L"%c\r\n", key);

    // Use choosed this mode...
    resMode = key - 'a';
    mode = resOptions[resMode];
    BootDisplayModeQuery(hDisplay, &mode, &width, &height, &bpp);
    pLoader->displayWidth = width;
    pLoader->displayHeight = height;
    pLoader->displayBpp = bpp;
    BootTerminalPrintf(
        hTerminal, L" Display resolution set to %d x %d x %d\r\n",
        width, height, bpp
        );

cleanUp:
    BootDisplayDeinit(hDisplay);
}

//------------------------------------------------------------------------------

static
void
DisplayMode(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    enum_t ix;
    wchar_t key;
    static struct {
        size_t width;
        size_t height;
    } const displayMode[] = {
        { 0, 0 }, { 176, 220 }, { 240, 240 }, { 240, 320 }, { 320, 240 },
        { 320, 320 }, { 400, 240 }, { 480, 640 }, { 480, 800 }, { 640, 480 }, { 640, 640 }, { 800, 480 }
    };
    UNREFERENCED_PARAMETER(pActionMenu);


    MenuHeader(hTerminal, L" Select Display Mode\r\n");

    for (ix = 0; ix < dimof(displayMode); ix++)
        {
        if (displayMode[ix].width == 0)
            {
            BootTerminalPrintf(hTerminal, L" [a] Full Screen\r\n");
            }
        else
            {
            key = L'a' + (wchar_t)ix;
            BootTerminalPrintf(
                hTerminal, L" [%c] %d x %d\r\n", ix + L'a', displayMode[ix].width,
                displayMode[ix].height
                );
            }
        }

    if (pLoader->displayLogicalWidth == 0)
        {
        BootTerminalPrintf(
            hTerminal, L"\r\n Selection (actual Full Screen): "
            );
        }
    else
        {
        BootTerminalPrintf(
            hTerminal, L"\r\n Selection (actual %d x %d): ",
            pLoader->displayLogicalWidth, pLoader->displayLogicalHeight
            );
        }

    // Get selection
    do
    {
        key = BootTerminalReadChar(hTerminal);
        if (key == L'\0') continue;
    }
    while ((key < L'a') || (key > (L'a' + dimof(displayMode) - 1)));

    // Print out selection character
    BootTerminalPrintf(hTerminal, L"%c\r\n", key);

    ix = key - L'a';
    pLoader->displayLogicalWidth = displayMode[ix].width;
    pLoader->displayLogicalHeight = displayMode[ix].height;

    if (pLoader->displayLogicalWidth == 0)
        {
        BootTerminalPrintf(
            hTerminal, L" Display mode set to Full Screen.\r\n"
            );
        }
    else
        {
        BootTerminalPrintf(
            hTerminal, L" Display mode set to %d x %d.\r\n",
            pLoader->displayLogicalWidth, pLoader->displayLogicalHeight
            );
        }

}

//------------------------------------------------------------------------------

static
BootTerminalMenu_t
g_menuDisplay = {
    L"Display Settings", {
        { L'1', L"Show Current Settings", DisplaySettings, NULL },
        { L'2', L"Change Display Resolution", DisplayResolution, NULL },
        { L'3', L"Change Viewable Display Region", DisplayMode, NULL },
        { L'0', L"Exit and Continue", NULL, NULL },
        { 0, NULL, NULL, NULL }
    }
};

//------------------------------------------------------------------------------

static
void
DebugSettingsEnablePort(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    bool_t enabled;
    UNREFERENCED_PARAMETER(pActionMenu);

    enabled = (pLoader->comPort != 0);
    BootTerminalReadEnable(hTerminal, &enabled, L"Serial Debug Port");
    if (enabled)
    {
        pLoader->comPort = 1; // default to COM1 - 38400 baud
        pLoader->baudDivisor = 3;
    }
    else
    {
        pLoader->comPort = 0; // disabled
    }

}

//------------------------------------------------------------------------------

static
void
DebugSettingsComPort(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    enum_t ix;
    wchar_t key;
    UNREFERENCED_PARAMETER(pActionMenu);

    MenuHeader(hTerminal, L" Select Com Port\r\n");

    for(ix = 0; ix < 4; ix++)
    {
        BootTerminalPrintf(hTerminal, L" [%d] COM%d\r\n", ix + 1, ix + 1);
    }

    if (pLoader->comPort == 0)
    {
        BootTerminalPrintf(
            hTerminal, L"\r\n Selection (Disabled): "
            );
    }
    else
    {
        BootTerminalPrintf(
            hTerminal, L"\r\n Selection (actual COM%d): ",
            pLoader->comPort
            );
    }

    // Get selection
    do
    {
        key = BootTerminalReadChar(hTerminal);
        if (key == L'\0') continue;
    }
    while ((key < L'1') || (key > (L'4')));

    // Print out selection character
    BootTerminalPrintf(hTerminal, L"%c\r\n", key);

    ix = key - L'0';
    pLoader->comPort = (uint8_t)ix;

    if (pLoader->comPort == 0)
    {
        BootTerminalPrintf(
            hTerminal, L" Debug port disabled.\r\n"
            );
    }
    else
    {
        BootTerminalPrintf(
            hTerminal, L" Debug port set to COM%d.\r\n",
            pLoader->comPort
            );
    }

}

//------------------------------------------------------------------------------

static
void
DebugSettingsBaudRate(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    enum_t ix;
    wchar_t key;

    static struct
    {
        ULONG   ulRate;
        UCHAR   ucDivisor;
    }   
    const BaudTable[] = {
        { 9600,  12},
        { 19200,  6},
        { 38400,  3},
        { 57600,  2},
        { 115200, 1}
    };
    static enum_t numEntries = (sizeof(BaudTable) / sizeof(BaudTable[0]));

    UNREFERENCED_PARAMETER(pActionMenu);

    MenuHeader(hTerminal, L" Set Baud Rate\r\n");

    for(ix = 0; ix < numEntries; ix++)
    {
        BootTerminalPrintf(hTerminal, L" [%d] %d\r\n", ix + 1, BaudTable[ix]);
    }

    for (ix = 0; ix < numEntries; ix++)
    {
        if (pLoader->baudDivisor == BaudTable[ix].ucDivisor)
        {
            BootTerminalPrintf(
                hTerminal, L"\r\n Selection (%d): ", BaudTable[ix].ulRate
                );
        }
    }

    // Get selection
    do            
    {
        key = BootTerminalReadChar(hTerminal);
        if (key == L'\0') continue;
    }
    while ((key < L'1') || (key > (L'0' + numEntries)));

    // Print out selection character
    BootTerminalPrintf(hTerminal, L"%c\r\n", key);

    ix = key - L'1';

    pLoader->baudDivisor = BaudTable[ix].ucDivisor;
            
    for (ix = 0; ix < numEntries; ix++)
    {
        if (pLoader->baudDivisor == BaudTable[ix].ucDivisor)
        {
            BootTerminalPrintf(
                hTerminal, L" Baud rate set to %d.\r\n",
                BaudTable[ix].ulRate
                );
        }
    }
}


//------------------------------------------------------------------------------

static
BootTerminalMenu_t
g_menuDebugPort = {
    L"Debug Port Settings", {
        { L'1', L"Enable/disable debug port", DebugSettingsEnablePort, NULL },
        { L'2', L"Set COM port", DebugSettingsComPort, NULL },
        { L'3', L"Set baud rate", DebugSettingsBaudRate, NULL },
        { L'0', L"Exit and Continue", NULL, NULL },
        { 0, NULL, NULL, NULL }
    }
};

//------------------------------------------------------------------------------

static
void
ShowSettings(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    enum_t baudRate = 0;
    UNREFERENCED_PARAMETER(pActionMenu);

    BootTerminalPrintf(hTerminal, L"\r\n Main:\r\n");
    BootTerminalPrintf(
        hTerminal, L"  Boot source:   %s\r\n",
        DevLocToString(pLoader, &pLoader->bootDevice)
        );
    BootTerminalPrintf(
        hTerminal, L"  KITL device:   %s\r\n",
        DevLocToString(pLoader, &pLoader->kitlDevice)
        );
    BootTerminalPrintf(
        hTerminal, L"  KITL config:   %s, %s mode, VMINI %s\r\n",
        ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_ENABLED) != 0) ?
        L"enabled" : L"disabled",
        ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_POLL) != 0) ?
        L"poll" : L"interrupt",
        ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_VMINI) != 0) ?
        L"enabled" : L"disabled"
        );
    if ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_DHCP) != 0)
        {
        BootTerminalPrintf(
            hTerminal, L"  DHCP:          enabled\r\n"
            );
        }
    else
        {
        BootTerminalPrintf(
            hTerminal, L"  IP address:    %d.%d.%d.%d\r\n",
            ((uint8_t*)&pLoader->ipAddress)[0],
            ((uint8_t*)&pLoader->ipAddress)[1],
            ((uint8_t*)&pLoader->ipAddress)[2],
            ((uint8_t*)&pLoader->ipAddress)[3]
            );
        }

    if (pLoader->displayLogicalWidth > 0)
        {
        BootTerminalPrintf(
            hTerminal, L"  Display:       %d x %d x %d / %d x %d\r\n",
            pLoader->displayWidth, pLoader->displayHeight, pLoader->displayBpp,
            pLoader->displayLogicalWidth, pLoader->displayLogicalHeight
            );
        }
    else
        {
        BootTerminalPrintf(
            hTerminal, L"  Display:       %d x %d x %d / Full Screen\r\n",
            pLoader->displayWidth, pLoader->displayHeight, pLoader->displayBpp
            );
        }

    if (pLoader->comPort == 0)
    {
        BootTerminalPrintf(
            hTerminal, L"  Debug Port:    disabled\r\n",
            pLoader->comPort);
    }
    else
    {
        if (pLoader->baudDivisor)
        {
            baudRate = 115200 / pLoader->baudDivisor;
        }

        BootTerminalPrintf(
            hTerminal, L"  Debug Port:    COM%d %d baud\r\n",
            pLoader->comPort, baudRate);
    }
}

//------------------------------------------------------------------------------

static
void
SaveSettings(
    void *pContext,
    handle_t hTerminal,
    void * pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    UNREFERENCED_PARAMETER(pActionMenu);

    if (WriteConfigToDisk(pLoader))
        {
        BootTerminalWriteString(
            hTerminal, L" Current settings has been saved\r\n"
            );
        }
    else
        {
        BootTerminalWriteString(
            hTerminal, L" ERROR: Settings save failed!\r\n"
            );
        }
}

//------------------------------------------------------------------------------

static
BootTerminalMenu_t
g_menuMain = {
    L"Main Menu", {
        { L'1', L"Show Current Settings", ShowSettings, NULL },
        { L'2', L"Select Boot Source", BootDevice, NULL },
        { L'3', L"Select KITL Device", KitlDevice, NULL },
        { L'4', L"Network Settings", NULL, &g_menuNetwork },
        { L'5', L"Display Settings", NULL, &g_menuDisplay },
        { L'6', L"Debug Port Settings", NULL, &g_menuDebugPort },
        { L'7', L"Save Settings", SaveSettings, NULL },
        { L'0', L"Exit and Continue", NULL, NULL },
        { 0, NULL, NULL, NULL }
    }
};

//------------------------------------------------------------------------------

static
void
TerminalMenu(
    BootLoader_t *pLoader
    )
{
    handle_t hTerm;
    wchar_t key = L'\0';
    uint32_t count, time;


    // Get terminal driver handle
    hTerm = OEMBootCreateDriver(pLoader, BOOT_DRIVER_CLASS_TERMINAL, 0);
    if (hTerm == NULL) goto cleanUp;

    BootTerminalPrintf(
        hTerm,
        L"Microsoft Windows CE Boot Loader Version %d.%d (Built %S %S)\r\n",
        VERSION_MAJOR, VERSION_MINOR, __DATE__, __TIME__
        );

    BootTerminalPrintf(hTerm, L"%dMB%s, ", pLoader->ramTop >> 20, 
        pLoader->ramestimated?L"(estimated)":L"");

    BootTerminalPrintf(hTerm, L"VRAM %dMB, ", (pLoader->videoRam+512*1024)/(1024*1024));

    // Notify about PCI BIOS support
    if (pLoader->pciBiosMajor != 0)
        {
        BootTerminalPrintf(
            hTerm, L"PCI Extension %x.%02x, ",
            pLoader->pciBiosMajor, pLoader->pciBiosMinor
            );
        }
    else
        {
        BootTerminalPrintf(hTerm, L"No PCI Extension, ");
        }

    // Notify about EDD BIOS support
    if (pLoader->eddBiosMajor != 0)
        {
        BootTerminalPrintf(
            hTerm, L"EDD Services %d.%d, ",
            pLoader->eddBiosMajor, pLoader->eddBiosMinor
            );
        }
    else
        {
        BootTerminalPrintf(hTerm, L"No EDD Services, ");
        }

    // Notify about VESA BIOS support
    if (pLoader->vbeBiosMajor != 0)
        {
        BootTerminalPrintf(
            hTerm, L"VESA %d.%d, ", pLoader->vbeBiosMajor, pLoader->vbeBiosMinor
            );
        }
    else
        {
        BootTerminalPrintf(hTerm, L"No VESA, ");
        }

    // Notify about APM BIOS support
    if (pLoader->apmBiosMajor != 0)
        {
        BootTerminalPrintf(
            hTerm, L"APM Services %d.%d\r\n", pLoader->apmBiosMajor,
            pLoader->apmBiosMinor
            );
        }
    else
        {
        BootTerminalPrintf(hTerm, L"No APM Services\r\n");
        }

    BootTerminalPrintf(hTerm, L"\r\n");

    // First let user break to menu
    count = 5;
    while ((count > 0) && key != L' ')
        {
        BootTerminalPrintf(
            hTerm, L"Hit space to enter configuration menu %d...\r\n", count
            );
        time = OEMBootGetTickCount();
        while ((OEMBootGetTickCount() - time) < 1000)
            {
            key = BootTerminalReadChar(hTerm);
            if (key == L' ') break;
           }
        count--;
        }

    // Start menu when user hit space
    if (key == L' ') BootTerminalMenu(pLoader, hTerm, &g_menuMain);

    BootTerminalPrintf(hTerm, L"\r\n");
    BootTerminalDeinit(hTerm);

cleanUp:
    return;
}

//------------------------------------------------------------------------------

enum_t
BootLoaderConfig(
    BootLoader_t *pLoader
    )
{
    enum_t rc = (enum_t)BOOT_STATE_FAILURE;
    BOOL* pUpdateMode;
    
    // Get saved or default configuration
    if (!ReadConfigFromDisk(pLoader)) DefaultConfig(pLoader);

    // If we boot in ULDR mode, don't show menu
    pUpdateMode = BootArgsQuery(pLoader, OAL_ARGS_QUERY_UPDATEMODE);
    if ((pUpdateMode != NULL) && (*pUpdateMode != 0))
        {
        rc = BOOT_STATE_LOAD_ULDR;
        }
    else
        {
        // Run menu
        TerminalMenu(pLoader);

        switch (pLoader->bootDevice.type)
            {
            case DeviceTypeEdbg:
                rc = BOOT_STATE_DOWNLOAD;
                break;
            case DeviceTypeStore:
                rc = BOOT_STATE_PRELOAD;
                break;
            }            
        }
    
    // Done
    return rc;
}

//------------------------------------------------------------------------------

