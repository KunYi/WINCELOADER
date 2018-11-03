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
//
// -- Intel Copyright Notice -- 
//  
// @par 
// Copyright (c) 2002-2011 Intel Corporation All Rights Reserved. 
//  
// @par 
// The source code contained or described herein and all documents 
// related to the source code ("Material") are owned by Intel Corporation 
// or its suppliers or licensors.  Title to the Material remains with 
// Intel Corporation or its suppliers and licensors. 
//  
// @par 
// The Material is protected by worldwide copyright and trade secret laws 
// and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, 
// distributed, or disclosed in any way except in accordance with the 
// applicable license agreement . 
//  
// @par 
// No license under any patent, copyright, trade secret or other 
// intellectual property right is granted to or conferred upon you by 
// disclosure or delivery of the Materials, either expressly, by 
// implication, inducement, estoppel, except in accordance with the 
// applicable license agreement. 
//  
// @par 
// Unless otherwise agreed by Intel in writing, you may not remove or 
// alter this notice or any other notice embedded in Materials by Intel 
// or Intel's suppliers or licensors in any way. 
//  
// @par 
// For further details, please see the file README.TXT distributed with 
// this software. 
//  
// @par 
// -- End Intel Copyright Notice -- 
//  
#include <Library/UefiLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Library/TimerLib.h>
#include <Protocol/SimpleTextInEx.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "include/bldr.h"
#include "include/bootterminalutils.h"
#include "include/globalvariable.h"
#include "include/oal_args.h"


// Control whether to let user access to boot menu
// By default, turn on for faster boot. 
#define FASTBOOT_NOMENU

//------------------------------------------------------------------------------
//  Global variables
EFI_LOADED_IMAGE                        *g_LoadedImage;
extern EFI_GRAPHICS_OUTPUT_PROTOCOL     *g_GraphicsOutput;
//------------------------------------------------------------------------------
// Local Function
static
void
TerminalMenu(
    );

bool_t
ReadConfigFromGlobalVar(
    BootLoader_t* pLoader
    );

static
void
ShowSettings(
    void *pContext,
    void * pActionMenu
    );

static
void
SelectDevice(
    BootLoader_t *pLoader,
    bool_t kitlDevice
    );

static
void
BootDevice(
    void *pContext,
    void * pActionMenu
    );

static
void
SaveSettings(
    void *pContext,
    void * pActionMenu
    );

static
bool_t
WriteConfigToDisk(
    );

//------------------------------------------------------------------------------

/**
  DisplaySettings(), This function will show current display resolution and physical size on display menu.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for display on terminal.

**/

static
void
DisplaySettings(
    void *pContext,
    void *pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;

    UNREFERENCED_PARAMETER(pActionMenu);

    Print( L"\r\n Display:\r\n");

    Print(L"  Resolution:    %d x %d x %d\r\n",
        pLoader->displayWidth, pLoader->displayHeight, pLoader->displayBpp
        );
    if (pLoader->displayLogicalWidth == 0) {
        Print(
            L"  Logical size:  Full Screen\r\n"
        );
    }
    else {
        Print(
        L"  Logical size:  %d x %d\r\n",
        pLoader->displayLogicalWidth, pLoader->displayLogicalHeight
        );
    }
}

/**
  DisplayResolution(), This function will show current display resolution and available
  resolution that get by GraphicsOutputon Protocol to display submenu, and then wait
  user to choose available item for set resolution value in boot config.
  Because the UEFI GraphicsOutput Protocol now only can obtian one mode from platform,
  so only one available item will present, otherwise, if user chose physical size is small
  than this available size, the physical size will show as default whatever user has been change
  physical size to other but not available size or default size.
  And the PixelFormat format is not identify as 8 ,16, 24 or 32, so Bpp setting will need to
  transfer from PixelFormat to set match value.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for display on terminal.

**/

//
static
void
DisplayResolution(
    void *pContext,
    void *pActionMenu
    )
{
    BootLoader_t                            *pLoader = pContext;
    enum_t                                  uMode, bppMode, resMode, uMaxMode;
    wchar_t                                 key;
    uint32_t                                bpp = 8, bppOptions[] = {0,0,0,0,};
    enum_t                                  resOptions[20];
    EFI_STATUS                              Status;
    UINTN                                   uSizeOfInfo;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION    *Info;

    UNREFERENCED_PARAMETER(pActionMenu);

    if (g_GraphicsOutput == NULL)
        return;

    Print(L" Select Desired Bits Per Pixel\r\n");

    uMaxMode = g_GraphicsOutput->Mode->MaxMode;

    uMode = bppMode = 0;
    while (uMode < uMaxMode) {
        Status = g_GraphicsOutput->QueryMode(
                                            g_GraphicsOutput,
                                            uMode,
                                            &uSizeOfInfo,
                                            &Info
                                            );

        if (EFI_ERROR(Status)) {
            WARNMSG(TRUE, (L"Cannot get mode info (status=0x%x)\n", Status));
            break;
        }
        // Only support four bpp modes (8, 16, 24, 32) currently, and 20 resolutions for each
        if ((Info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor) ||
            (Info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor))
              bpp = 24;

        if (!((bpp == bppOptions[0]) ||
              (bpp == bppOptions[1]) ||
              (bpp == bppOptions[2]) ||
              (bpp == bppOptions[3]))) {
            key = L'a' + (wchar_t) uMode;
            Print(L" [%c] %2d\r\n",
                key, bpp
                );

            bppOptions[bppMode++] = bpp;
        }

        uMode++;
    }

    Print(
        L"\r\n Selection (actual %d bpp): ",
        pLoader->displayBpp
        );

    // Get selection
    do
        {
        key = ReadKeyboardCharacter();
        if (key == L'\0') continue;
        }
    while ((key < L'a') || (key >= (L'a' + bppMode)));

    // Print out selection character
    Print(L"%c\r\n", key);

    // User choosed this mode...
    bppMode = key - 'a';


    Print(L" Select Display Resolution\r\n");

    uMaxMode = g_GraphicsOutput->Mode->MaxMode;
    uMode = resMode = 0;
    uSizeOfInfo = 0;
    while (uMode < uMaxMode) {
        Status = g_GraphicsOutput->QueryMode(
                                            g_GraphicsOutput,
                                            uMode,
                                            &uSizeOfInfo,
                                            &Info
                                            );
        if (EFI_ERROR(Status)) {
            WARNMSG(TRUE, (L"Cannot get mode info (status=0x%x)\n", Status));
            break;
        }

        if ((Info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor) ||
            (Info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor))
              bpp = 24;

        if (bpp == bppOptions[bppMode]) {
            key = L'a' + (wchar_t)resMode;
            Print(
                L" [%c] %4d x %4d x %2d\r\n",
                key, Info->HorizontalResolution, Info->VerticalResolution,
                bpp
                );

            resOptions[resMode++] = uMode;
        }

        uMode++;
    }

    Print(
        L"\r\n Selection (actual %d x %d x %d): ",
        pLoader->displayWidth, pLoader->displayHeight, pLoader->displayBpp
        );

    // Get selection
    do
        {
        key = ReadKeyboardCharacter();
        if (key == L'\0') continue;
        }
    while ((key < L'a') || (key >= (L'a' + resMode)));

    // Print out selection character
    Print(L"%c\r\n", key);

    // Use choosed this mode...
    resMode = key - 'a';
    uMode = resOptions[resMode];
    Status = g_GraphicsOutput->QueryMode(
                                        g_GraphicsOutput,
                                        uMode,
                                        &uSizeOfInfo,
                                        &Info
                                        );
    if (EFI_ERROR(Status)) {
            WARNMSG(TRUE, (L"Cannot get mode info (status=0x%x)\n", Status));
            bpp = 8;
    }

    if ((Info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor) ||
        (Info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor))
        bpp = 24;

    pLoader->displayWidth = Info->HorizontalResolution;
    pLoader->displayHeight = Info->VerticalResolution;
    pLoader->displayBpp = bpp;
    Print(
        L" Display resolution set to %d x %d x %d\r\n",
        pLoader->displayWidth, pLoader->displayHeight, pLoader->displayBpp
        );
}

/**
  DisplayMode(), This function will show current physical resolution and list
  resolution on display submenu, meanwhile, wait user to choose available items for set
  resolution value in boot config.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for display on terminal.

**/
static
void
DisplayMode(
    void *pContext,
    void *pActionMenu
    )
{
    BootLoader_t    *pLoader = pContext;
    enum_t          ix;
    wchar_t         key;
    static struct {
        size_t width;
        size_t height;
    } const displayMode[] = {
        { 0, 0 }, { 176, 220 }, { 240, 240 }, { 240, 320 }, { 320, 240 },
        { 320, 320 }, { 400, 240 }, { 480, 640 }, { 480, 800 }, { 640, 480 },
        { 640, 640 }, { 800, 480 }, { 800, 600}, { 1024, 768 }, { 1366, 768}
    };

    UNREFERENCED_PARAMETER(pActionMenu);

    Print(L" Select Display Mode\r\n");

    for (ix = 0; ix < dimof(displayMode); ix++) {
        if (displayMode[ix].width == 0) {
            Print(L" [a] Full Screen\r\n");
        }
        else {
            key = L'a' + (wchar_t)ix;
            Print(
                L" [%c] %d x %d\r\n", ix + L'a', displayMode[ix].width,
                displayMode[ix].height
                );
        }
    }

    if (pLoader->displayLogicalWidth == 0) {
        Print(
            L"\r\n Selection (actual Full Screen): "
            );
    }
    else {
        Print(
            L"\r\n Selection (actual %d x %d): ",
            pLoader->displayLogicalWidth, pLoader->displayLogicalHeight
            );
    }

    // Get selection
    do {
        key = ReadKeyboardCharacter();
        if (key == L'\0') continue;
    }while ((key < L'a') || (key > (L'a' + dimof(displayMode) - 1)));

    // Print out selection character
    Print( L"%c\r\n", key);

    ix = key - L'a';
    pLoader->displayLogicalWidth = displayMode[ix].width;
    pLoader->displayLogicalHeight = displayMode[ix].height;

    if (pLoader->displayLogicalWidth == 0) {
        Print(
            L" Display mode set to Full Screen.\r\n"
            );
    }
    else{
        Print(
            L" Display mode set to %d x %d.\r\n",
            pLoader->displayLogicalWidth, pLoader->displayLogicalHeight
            );
    }

}

/**
  g_menuDisplay(), a submenu for Display setting, include menu title, resolution
  setting submenu and relation function.

**/
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

/**
  DebugSettingsEnablePort(), This function will show current debug port state,
  if it is enable request, the code will sset default value in port number and
  baudrate, that will be COM1 and 38400bps, if it is disable, the port number
  will set to "0".

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for Debug port.

**/
static
void
DebugSettingsEnablePort(
    void *pContext,
    void *pActionMenu
    )
{
    BootLoader_t    *pLoader = pContext;
    bool_t          enabled;

    UNREFERENCED_PARAMETER(pActionMenu);

    enabled = (pLoader->comPort != 0);
    BootTerminalReadEnable(&enabled, L"Serial Debug Port");

    if (enabled) {
        pLoader->comPort = 1; // default to COM1 - 38400 baud
        pLoader->baudDivisor = 3;
    }
    else {
        pLoader->comPort = 0; // disabled
    }

}

/**
  DebugSettingsComPort(), This function will show current debug port state
  and port number, then wait user choose used COM port for set in boot config.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for Debug port.

**/
static
void
DebugSettingsComPort(
    void *pContext,
    void *pActionMenu
    )
{
    BootLoader_t    *pLoader = pContext;
    enum_t          ix;
    wchar_t         key;

    UNREFERENCED_PARAMETER(pActionMenu);

    Print(L" Select Com Port\r\n");

    for(ix = 0; ix < 4; ix++)
    {
        Print(L" [%d] COM%d\r\n", ix + 1, ix + 1);
    }

    if (pLoader->comPort == 0) {
        Print(
            L"\r\n Selection (Disabled): "
            );
    }
    else {
        Print(
            L"\r\n Selection (actual COM%d): ",
            pLoader->comPort
            );
    }

    // Get selection
    do{
        key = ReadKeyboardCharacter();
        if (key == L'\0') continue;
    }while ((key < L'1') || (key > (L'4')));

    // Print out selection character
    Print(L"%c\r\n", key);

    ix = key - L'0';
    pLoader->comPort = (uint8_t)ix;

    if (pLoader->comPort == 0) {
        Print(
            L" Debug port disabled.\r\n"
            );
    }
    else {
        Print(
            L" Debug port set to COM%d.\r\n",
            pLoader->comPort
            );
    }

}

/**
  DebugSettingsBaudRate(), This function will show current baudrate that debug port using,
  and 5 available baudrates for user choose, after user select one of them, the value will
  set in boot config.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for Debug port.

**/
static
void
DebugSettingsBaudRate(
    void *pContext,
    void *pActionMenu
    )
{
    BootLoader_t    *pLoader = pContext;
    enum_t          ix;
    wchar_t         key;

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

    Print(L" Set Baud Rate\r\n");

    for(ix = 0; ix < numEntries; ix++) {
        Print(L" [%d] %d\r\n", ix + 1, BaudTable[ix]);
    }

    for (ix = 0; ix < numEntries; ix++) {
        if (pLoader->baudDivisor == BaudTable[ix].ucDivisor) {
            Print(
                L"\r\n Selection (%d): ", BaudTable[ix].ulRate
                );
        }
    }

    // Get selection
    do {
        key = ReadKeyboardCharacter();
        if (key == L'\0') continue;
    }
    while ((key < L'1') || (key > (L'0' + numEntries)));

    // Print out selection character
    Print(L"%c\r\n", key);

    ix = key - L'1';

    pLoader->baudDivisor = BaudTable[ix].ucDivisor;

    for (ix = 0; ix < numEntries; ix++) {
        if (pLoader->baudDivisor == BaudTable[ix].ucDivisor) {
            Print(
                L" Baud rate set to %d.\r\n",
                BaudTable[ix].ulRate
                );
        }
    }
}


/**
  g_menuDebugPort(), a submenu for Debug port setting, include menu title, port enable
  setting submenu, port number setting submenu and relation function.

**/
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

/**
  NetworkSettings(), This function will show current KITL information.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for Debug port.

**/
static
void
NetworkSettings(
    void *pContext,
    void *pActionMenu
    )
{
    BootLoader_t    *pLoader = pContext;
    flags_t         flags = pLoader->kitlFlags;

    UNREFERENCED_PARAMETER(pActionMenu);

    Print(L"\r\n Network:\r\n");
    Print(
        L"  KITL state:    %s\r\n",
        ((flags & BOOT_TRANSPORT_EDBG_FLAG_ENABLED) != 0) ?
        L"enabled" : L"disabled"
        );
    Print(
        L"  KITL mode:     %s\r\n",
        ((flags & BOOT_TRANSPORT_EDBG_FLAG_POLL) != 0) ? L"poll" : L"interrupt"
        );
    Print(
        L"  DHCP:          %s\r\n",
        ((flags & BOOT_TRANSPORT_EDBG_FLAG_DHCP) != 0) ?
        L"enabled" : L"disabled"
        );
    Print(
        L"  IP address:    %d.%d.%d.%d\r\n",
        ((uint8_t*)&pLoader->ipAddress)[0], ((uint8_t*)&pLoader->ipAddress)[1],
        ((uint8_t*)&pLoader->ipAddress)[2], ((uint8_t*)&pLoader->ipAddress)[3]
        );
    Print(
        L"  VMINI:         %s\r\n",
        ((flags & BOOT_TRANSPORT_EDBG_FLAG_VMINI) != 0) ?
        L"enabled" : L"disabled"
        );
}

/**
  EnableKitl(), This function will show current KITL enable/disable state.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for Debug port.

**/
static
void
EnableKitl(
    void *pContext,
    void *pActionMenu
    )
{
    BootLoader_t    *pLoader = pContext;
    bool_t          enabled;

    UNREFERENCED_PARAMETER(pActionMenu);

    enabled = (pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_ENABLED) != 0;
    BootTerminalReadEnable(&enabled, L"KITL");

    if (enabled)
        pLoader->kitlFlags |= BOOT_TRANSPORT_EDBG_FLAG_ENABLED;
    else
        pLoader->kitlFlags &= ~BOOT_TRANSPORT_EDBG_FLAG_ENABLED;
}

/**
  SetKitlMode(), This function will show current KITL operation mode, it will be
  Interrupt or polling mode.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for Debug port.

**/
static
void
SetKitlMode(
    void *pContext,
    void *pActionMenu
    )
{
    BootLoader_t    *pLoader = pContext;
    wchar_t         key;
    bool_t          poll;

    UNREFERENCED_PARAMETER(pActionMenu);

    poll = (pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_POLL) != 0;


    if (poll) {
        Print(
            L" Set KITL to interrupt mode [y/-]: "
            );
    }
    else {
        Print(
            L" Set KITL to poll mode [y/-]: "
            );
    }

    do {
        key = ReadKeyboardCharacter();
    }while (key == '\0');

    Print(L"%c\r\n", key);

    if ((key == L'y') || (key == L'Y')) {
        if (poll) {
            pLoader->kitlFlags &= ~BOOT_TRANSPORT_EDBG_FLAG_POLL;
            Print(
                 L" KITL set to interrupt mode\r\n"
                 );
        }
        else {
            pLoader->kitlFlags |= BOOT_TRANSPORT_EDBG_FLAG_POLL;
            Print(
                 L" KITL set to poll mode\r\n"
                 );
        }
    }
}

/**
  EnableDHCP(), This function will show current DHCP enable state, and get
  request by BootTerminalReadEnable() to decide enable or disable DHCP feature,
  and set flag to Boot Config

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for Debug port.

**/

static
void
EnableDHCP(
    void *pContext,
    void *pActionMenu
    )
{
    BootLoader_t    *pLoader = pContext;
    bool_t          enabled = pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_DHCP;

    UNREFERENCED_PARAMETER(pActionMenu);

    BootTerminalReadEnable(&enabled, L"DHCP");
    if (enabled)
        pLoader->kitlFlags |= BOOT_TRANSPORT_EDBG_FLAG_DHCP;
    else
        pLoader->kitlFlags &= ~BOOT_TRANSPORT_EDBG_FLAG_DHCP;
}

/**
  DeviceIp(), This function will show current IP addrss, and get user input
  IP address by BootTerminalReadIp4(), and then set IP address value to Boot Config.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for Debug port.

**/
static
void
DeviceIp(
    void *pContext,
    void *pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;
    uint32_t ip = pLoader->ipAddress;

    UNREFERENCED_PARAMETER(pActionMenu);

    if (BootTerminalReadIp4(&ip, L"Device")) {
        pLoader->ipAddress = ip;
    }
}

/**
  EnableVMINI(), This function will show current VMINI enable state, and get user
  selection by BootTerminalReadEnable(), and then set VMINI state value to Boot Config.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for Debug port.

**/
static
void
EnableVMINI(
    void *pContext,
    void * pActionMenu
    )
{
    BootLoader_t    *pLoader = pContext;
    bool_t          enabled = pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_VMINI;
    
    UNREFERENCED_PARAMETER(pActionMenu);

    BootTerminalReadEnable(&enabled, L"VMINI");
    if (enabled)
        pLoader->kitlFlags |= BOOT_TRANSPORT_EDBG_FLAG_VMINI;
    else
        pLoader->kitlFlags &= ~BOOT_TRANSPORT_EDBG_FLAG_VMINI;
}

/**
  KitlDevice(), This function will call SelectDevice() to get NIC config via PCI,
  and list available NIC for user choose.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for Debug port.

**/
static
void
KitlDevice(
    void *pContext,
    void * pActionMenu
    )
{
    UNREFERENCED_PARAMETER(pActionMenu);
    SelectDevice(pContext, true);
}

/**
  g_menuNetwork(), a submenu for KITL setting , include menu title, KITL setting submenu
  in KITL enable, operation mode, DHCP enable, IP address config, VMINI enable
  and relation function.

**/
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

/**
  g_menuMain(), a Main menu for Boot Config, include menu title, current setting,
  Boot device selection, KITL device selection, Network setting, Display setting,
  Debug port setting, and save Boot config to EFI Gloabl variable for next booting
  and relation function.

**/
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

/**
  DefaultConfig(), this function called by BootLoaderConfig() if it has not GB (Global Variables)
  available in UEFI, it will initial GB and save to UEFI BIOS, and then set default value
  to Boot Config.
  Because PCI function is not implement yet, so any PCI relation codes are comment out.

  @param  pLoader   A pointer to the BootLoader_t content.

**/
void
DefaultConfig(
    BootLoader_t *pLoader
    )
{

//******************************************************************************
// This function is to set a BootConfig_t type default value in Global Variable
// may it is not need in code, theose values will finn in by user while chosse
// "Save Setting" in Boot Menu.

    InitializeGlobalVariables();
//******************************************************************************


//******************************************************************************
// Because below code has use BIOS interrupt function, and now Pci function not
// acquire from EFI yet, so comment out and setting one default to it..
#if 0
    BootPciLocation_t   pciLoc;
    bool_t              found = false;


    found = false;
    pciLoc.logicalLoc = 0;
    while (pciLoc.bus <= pLoader->pciLastBus) {
        uint32_t id = BootPciGetId(pciLoc);
        enum_t ix;
        uint32_t classcode = BootPciGetClassCodeAndRevId(pciLoc);
        classcode >>= 8; // discard revision id

        for (ix = 0; ix < pLoader->devices; ix++) {
            const Device_t *pDevice = &pLoader->pDevices[ix];
            if (pDevice->type != DeviceTypeEdbg) continue;
            if (pDevice->ifc != IfcTypePci) continue;
            if (id == pDevice->id) {
                found = true;
                break;
            }
        }
        if (found) break;
        if (classcode == CLASS_CODE_ETHERNET) {
            if (g_dwOEMUnsupportedNICCount < MAX_OEM_UNSUPPORTED_NICS) {
                // save id to display later.
                g_dwOEMUnsupportedNICs[g_dwOEMUnsupportedNICCount++] = id;
            }
        }

        BootPciNextLocation(&pciLoc);
    }

    if (found) {
        pLoader->bootDevice.type = DeviceTypeEdbg;
        pLoader->bootDevice.ifc = IfcTypePci;
        pLoader->bootDevice.busNumber = 0;
        pLoader->bootDevice.location = pciLoc.logicalLoc;
    }
#endif
//******************************************************************************

    pLoader->bootDevice.type = DeviceTypeStore;
    pLoader->bootDevice.ifc = (enum_t)IfcTypeUndefined;
    pLoader->bootDevice.busNumber = 0;
    pLoader->bootDevice.location = 0;

    pLoader->formatUserStore = false;
    pLoader->ipAddress = 0;
    pLoader->kitlFlags = BOOT_TRANSPORT_EDBG_FLAG_ENABLED |
                         BOOT_TRANSPORT_EDBG_FLAG_VMINI;

    if (pLoader->ipAddress == 0) {
        pLoader->kitlFlags |= BOOT_TRANSPORT_EDBG_FLAG_DHCP;
    }

    pLoader->kitlDevice = pLoader->bootDevice;

    pLoader->comPort = 1;
    pLoader->baudDivisor = 1;

    pLoader->displayWidth = 640;            // Display mode for Windows CE
    pLoader->displayHeight = 480;
    pLoader->displayBpp = 16;
    pLoader->displayLogicalWidth = 640;
    pLoader->displayLogicalHeight = 480;

}

/**
  DevLocToString(), this function called for change Locgic device to a naming string
  for show on Boot Menu or submenu.
  Because PCI function is not implement yet, so any PCI relation codes are comment out.

  @param  pLoader       A pointer to the BootLoader_t content.

  @param  pDevLoc       A pointer to the device type for mapping name.

  @return wcstring_t    A converted string from Locgic device to a naming string .

**/
static
wcstring_t
DevLocToString(
    BootLoader_t            *pLoader,
    const DeviceLocation_t  *pDevLoc
    )
{
    wcstring_t      info = NULL;
    static wchar_t  buffer[64];


    switch (pDevLoc->type) {

        case DeviceTypeEdbg:
            switch (pDevLoc->ifc) {
                case IfcTypePci:
                    {
//******************************************************************************
// Because Pci function not acquire from EFI yet, so if pDevLoc->location == 0,
// just to print "No NIC in PCI now!"

                        if(pDevLoc->location == 0)
                            info = L"No NIC in PCI now!";
                        break;
//******************************************************************************
#if 0


                    BootPciLocation_t pciLoc;
                    uint32_t id;
                    enum_t ix;

                    pciLoc.logicalLoc = pDevLoc->location;
                    id = BootPciGetId(pciLoc);
                    for (ix = 0; ix < pLoader->devices; ix++) {
                        const Device_t *pDevice = &pLoader->pDevices[ix];
                        if (pDevLoc->ifc != pDevice->ifc) continue;
                        if (id == pDevice->id) {
                                Print(
                                L"%s at PCI bus %d dev %d fnc %d",
                                pDevice->name, pciLoc.bus, pciLoc.dev,
                                pciLoc.fnc
                                );
                            info = buffer;
                            break;
                        }
                    }
#endif
                }
                    break;
            }
            break;

        case DeviceTypeStore:
            info = L"Boot Disk";
            break;
    }

    return info;
}

/**
  BootLoaderConfig(), one of booting step, this function is to read Boot Config
  from GB, if it is not exist, it will call DefaultConfig() to initial GB and boot
  Config, before into Boot Menu, it will check update mode is enable or not, if so,
  it set Boot State as BOOT_STATE_LOAD_ULDR for next setp in Main(),if not, called
  TerminalMenu() to create boot menu for user to set boot configuration as he want.
  If user set image booting by ethernet, it will set Boot State as BOOT_STATE_DOWNLOAD
  for next step.
  If user set image booting by Store, it will set Boot State as BOOT_STATE_PRELOAD
  for next step.
  Because PCI function is not implement yet, so any PCI relation codes are comment out.

  @param  pLoader       A pointer to the BootLoader_t content.

  @return enum_t        A Boot State for next step, if this function failed, it will return
                        BOOT_STATE_FAILURE for stop booting procedure.

**/
enum_t
BootLoaderConfig(
    BootLoader_t *pLoader
    )
{
    enum_t  rc = (enum_t)BOOT_STATE_FAILURE;
    bool_t* pUpdateMode;

    if (!(ReadConfigFromGlobalVar(pLoader))) {
        ERRMSG(TRUE, (L"BootLoaderConfig: No Global Variable in UEFI\n"));
        DefaultConfig(pLoader);
    }

// If we boot in ULDR mode, don't show menu
    pUpdateMode = BootArgsQuery(pLoader, OAL_ARGS_QUERY_UPDATEMODE);
    if ((pUpdateMode != NULL) && (*pUpdateMode != 0)) {
        rc = BOOT_STATE_LOAD_ULDR;
    }
    else {
        // Run menu
        TerminalMenu(pLoader);

        switch (pLoader->bootDevice.type) {
//******************************************************************************
// Not NIC Support yet, so comment out
#if 0
            case DeviceTypeEdbg:
                rc = BOOT_STATE_DOWNLOAD;
                break;
#endif
//******************************************************************************
            case DeviceTypeStore:
                rc = BOOT_STATE_PRELOAD;
                break;
        }
    }

    // Done
    return rc;
}


/**
  ReadConfigFromGlobalVar, this function read Boot Config fromm GB, and set them to
  local Boot Config, this local Boot Config of content finaly to set to BOOTARGS for
  pass to OS.

  @param  pLoader       A pointer to the BootLoader_t content.

  @return enum_t        A BOOL value to identify GB is exist or not.

**/
bool_t
ReadConfigFromGlobalVar(
    BootLoader_t *pLoader
    )
{
    bool_t          rc = true;
    BootConfig_t    BootConfig;

    // Check if Global Variable area has initial value or not
    if(!CheckIsGlobalVarVaild(&BootConfig)) {
        rc = false;
        goto cleanup;
    }


    DBGMSG(DBGZONE_INFO, (L"signature           = '%c%c%c%c%c%c%c'\n",
        BootConfig.signature[0], BootConfig.signature[1],
        BootConfig.signature[2], BootConfig.signature[3],
        BootConfig.signature[4], BootConfig.signature[5],
        BootConfig.signature[6]));
    DBGMSG(DBGZONE_INFO, (L"version              = 0x%X \n", BootConfig.version));
    DBGMSG(DBGZONE_INFO, (L"ipAddress            = 0x%X \n", BootConfig.ipAddress));
    DBGMSG(DBGZONE_INFO, (L"kitlFlags            = %d \n",   BootConfig.kitlFlags));
    DBGMSG(DBGZONE_INFO, (L"comPort              = COM%d \n",BootConfig.comPort));
    DBGMSG(DBGZONE_INFO, (L"baudDivisor          = %d \n",   BootConfig.baudDivisor));
    DBGMSG(DBGZONE_INFO, (L"imageUpdateFlags     = %d \n",   BootConfig.imageUpdateFlags));
    DBGMSG(DBGZONE_INFO, (L"displayWidth         = %d \n",   BootConfig.displayWidth));
    DBGMSG(DBGZONE_INFO, (L"displayHeight        = %d \n",   BootConfig.displayHeight));
    DBGMSG(DBGZONE_INFO, (L"displayBpp           = %d \n",   BootConfig.displayBpp));
    DBGMSG(DBGZONE_INFO, (L"displayLogicalWidth  = %d \n",   BootConfig.displayLogicalWidth));
    DBGMSG(DBGZONE_INFO, (L"displayLogicalHeight = %d \n",   BootConfig.displayLogicalHeight));
    DBGMSG(DBGZONE_INFO, (L"bootDevice.type      = 0x%X \n", BootConfig.bootDevice.type));
    DBGMSG(DBGZONE_INFO, (L"bootDevice.ifc       = 0x%X \n", BootConfig.bootDevice.ifc));
    DBGMSG(DBGZONE_INFO, (L"bootDevice.busNumber = 0x%X \n", BootConfig.bootDevice.busNumber));
    DBGMSG(DBGZONE_INFO, (L"bootDevice.location  = 0x%X \n", BootConfig.bootDevice.location));
    DBGMSG(DBGZONE_INFO, (L"kitlDevice.type      = 0x%X \n", BootConfig.kitlDevice.type));
    DBGMSG(DBGZONE_INFO, (L"kitlDevice.ifc       = 0x%X \n", BootConfig.kitlDevice.ifc));
    DBGMSG(DBGZONE_INFO, (L"kitlDevice.busNumber = 0x%X \n", BootConfig.kitlDevice.busNumber));
    DBGMSG(DBGZONE_INFO, (L"kitlDevice.location  = 0x%X \n", BootConfig.kitlDevice.location));


    // Check if this is consistent version
    if (BootConfig.version != BOOT_CONFIG_VERSION)
        goto cleanup;

    // Update loader context
    pLoader->ipAddress = BootConfig.ipAddress;
    pLoader->kitlFlags = BootConfig.kitlFlags;
    pLoader->bootDevice = BootConfig.bootDevice;
    pLoader->kitlDevice = BootConfig.kitlDevice;

    pLoader->displayWidth = BootConfig.displayWidth;
    pLoader->displayHeight = BootConfig.displayHeight;
    pLoader->displayBpp = BootConfig.displayBpp;

    pLoader->displayLogicalWidth = BootConfig.displayLogicalWidth;
    pLoader->displayLogicalHeight = BootConfig.displayLogicalHeight;

    pLoader->comPort = BootConfig.comPort;
    pLoader->baudDivisor = BootConfig.baudDivisor;

    pLoader->imageUpdateFlags = BootConfig.imageUpdateFlags;

cleanup:

    return rc;
}

/**
  TerminalMenu, this function will show some loader information on terminal,
  and then going 5 seeconds count down to wait user keying " " to show Boot Menu
  up.

  @param  pLoader       A pointer to the BootLoader_t content.

**/
static
void
TerminalMenu(
    BootLoader_t *pLoader
    )
{
    wchar_t     key = L'\0';

#ifndef FASTBOOT_NOMENU
    uint32_t    count, PassedTime;
#endif

    Print(L"Microsoft Windows CE Boot Loader Version %d.%d (Built %S %S)\r\n",
            VERSION_MAJOR, VERSION_MINOR, __DATE__, __TIME__
            );

    Print(L"%dMB%s \r\n", pLoader->ramTop >> 20,
        pLoader->ramestimated ? L"(estimated)" :L"");

    // Notify about PCI BIOS support
    if (pLoader->pciBiosMajor != 0) {
        Print(
            L"PCI Extension %x.%02x, ",
            pLoader->pciBiosMajor, pLoader->pciBiosMinor
            );
    }
    else {
        Print(L"No PCI Extension, ");
    }

    // Notify about EDD BIOS support
    if (pLoader->eddBiosMajor != 0) {
        Print(
            L"EDD Services %d.%d, ",
            pLoader->eddBiosMajor, pLoader->eddBiosMinor
            );
    }
    else {
        Print(L"No EDD Services, ");
    }

    // Notify about VESA BIOS support
    if (g_GraphicsOutput) {
        Print(
            L"VESA version %d , ", g_GraphicsOutput->Mode->Info->Version
            );
    }
    else {
        Print(L"No VESA, ");
    }

    // Notify about APM BIOS support
    if (pLoader->apmBiosMajor != 0) {
        Print(
            L"APM Services %d.%d\r\n", pLoader->apmBiosMajor,
            pLoader->apmBiosMinor
            );
    }
    else {
        Print(L"No APM Services\r\n");
    }

    Print(L"\r\n");

#ifndef FASTBOOT_NOMENU
    // First let user break to menu
    count = 5;          // 5 seconds
    PassedTime = 0;

    while ((count > 0) && key != L' ') {
        Print(L"Hit space to enter configuration menu %d...\r\n", count);

        while(1) {
           key = ReadKeyboardCharacter();
           if (key == L' ') {
              DBGMSG(DBGZONE_INFO, (L"Get 'Spec'Key, key = %d\n",
                (CHAR)(key & 0x000000ff)));
              break;
            }

            MicroSecondDelay(100000);       // 100 mS
            PassedTime += 100000;
            if(PassedTime == 1000000)
                break;
        }
        count--;
        PassedTime = 0;
    }
#endif // FASTBOOT_NOMENU

    // Start menu when user hit space
    if (key == L' ')
        BootTerminalMenu(pLoader, &g_menuMain);

    Print(L"\r\n");

    return;
}

/**
  ReadKeyboardCharacter, this function read KEY value by calling RunTime Services
  of Console Input protocol of ReadKeyStroke(), it is be called in many Boot menu.

  @return wchar_t        A wide char key that user pressed.
**/
wchar_t
ReadKeyboardCharacter(
    )
{
    EFI_STATUS      Status;
    EFI_INPUT_KEY   inputKey;

    Status = gST->ConIn->ReadKeyStroke(
                                      gST->ConIn,
                                      &inputKey
                                      );

    if (!EFI_ERROR(Status)) {
        return (wchar_t)inputKey.UnicodeChar;
    }

    return 0;
}

/**
  ShowSettings(), This function will show current Main menu setting, for user know \
  the device how to booting at this time, or in every Main menu item, show it submenu
  setting and changed value confirm.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for Debug port.

**/
static
void
ShowSettings(
    void *pContext,
    void * pActionMenu
    )
{
    BootLoader_t    *pLoader = pContext;
    enum_t          baudRate = 0;

    UNREFERENCED_PARAMETER(pActionMenu);

    Print( L"\r\n ShowSetting:\r\n");
    Print(L"  Boot source:   %s\r\n",
        DevLocToString(pLoader, &pLoader->bootDevice)
        );
    Print(
         L"  KITL device:   %s\r\n",
        DevLocToString(pLoader, &pLoader->kitlDevice)
        );
    Print(
         L"  KITL config:   %s, %s mode, VMINI %s\r\n",
        ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_ENABLED) != 0) ?
        L"enabled" : L"disabled",
        ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_POLL) != 0) ?
        L"poll" : L"interrupt",
        ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_VMINI) != 0) ?
        L"enabled" : L"disabled"
        );
    if ((pLoader->kitlFlags & BOOT_TRANSPORT_EDBG_FLAG_DHCP) != 0) {
        Print(
             L"  DHCP:          enabled\r\n"
            );
    }
    else {
        Print(
             L"  IP address:    %d.%d.%d.%d\r\n",
            ((uint8_t*)&pLoader->ipAddress)[0],
            ((uint8_t*)&pLoader->ipAddress)[1],
            ((uint8_t*)&pLoader->ipAddress)[2],
            ((uint8_t*)&pLoader->ipAddress)[3]
            );
    }

    if (pLoader->displayLogicalWidth > 0) {
        Print(
             L"  Display:       %d x %d x %d / %d x %d\r\n",
            pLoader->displayWidth, pLoader->displayHeight, pLoader->displayBpp,
            pLoader->displayLogicalWidth, pLoader->displayLogicalHeight
            );
    }
    else {
        Print(
             L"  Display:       %d x %d x %d / Full Screen\r\n",
            pLoader->displayWidth, pLoader->displayHeight, pLoader->displayBpp
            );
    }

    if (pLoader->comPort == 0) {
        Print(
             L"  Debug Port:    disabled\r\n",
            pLoader->comPort);
    }
    else {
        if (pLoader->baudDivisor) {
            baudRate = 115200 / pLoader->baudDivisor;
        }

        Print(
             L"  Debug Port:    COM%d %d baud\r\n",
            pLoader->comPort, baudRate);
    }
}

/**
  BootDevice(), This function will call SelectDevice() to get Store config, in this
  loader, only "Boot Disk" is selection item.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for Debug port.

**/
static
void
BootDevice(
    void  *pContext,
    void  *pActionMenu
    )
{
    UNREFERENCED_PARAMETER(pActionMenu);
    SelectDevice(pContext, false);
}

/**
  SelectDevice(), This function called by BootDevice() and KITLDevice()to get it
  available device for user choose, and then set to Boot Config.
  Because PCI function is not implement yet, so any PCI relation codes are comment out.

  @param  pLoader       A pointer to the BootLoader_t content.

  @param  kitlDevice    for Boot Device, it is FALSE, for KITL device, it is TRUE.

**/
static
void
SelectDevice(
    BootLoader_t    *pLoader,
    bool_t          kitlDevice
    )
{

    DeviceLocation_t    devLoc[9], actualDevLoc;
    enum_t              ix, iy;
    wchar_t             key;
    wcstring_t          name;

    UNREFERENCED_PARAMETER(kitlDevice);

    Print(L" Select %s Device\r\n", kitlDevice ? L"KITL" : L"Boot");

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
                            devLoc[ix].type = DeviceTypeEdbg;
                            devLoc[ix].ifc = IfcTypePci;
                            devLoc[ix].busNumber = 0;
                            devLoc[ix].location = 0;
                            name = pLoader->pDevices[iy].name;
                            if(kitlDevice)
                            Print(
                               L" [%d] %s\r\n", ix + 1, name
                               );
                            ix++;

//******************************************************************************
// Because Pci function not acquire from EFI yet, so just present default NIC
// name on list that while use choose KITL or BOOT device, but NIC is not
// functional until now
#if 0
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
#endif
//******************************************************************************
                        }
                       break;
                    }
                break;
            case DeviceTypeStore:
                if (kitlDevice)
                    break;
                devLoc[ix].type = DeviceTypeStore;
                devLoc[ix].ifc = (enum_t)IfcTypeUndefined;
                devLoc[ix].busNumber = 0;
                devLoc[ix].location = 0;
                name = DevLocToString(pLoader, &devLoc[ix]);
                Print(L" [%d] %s\r\n", ix + 1, name);
                ix++;
                break;
            }
        if (ix >= dimof(devLoc)) break;
        }

    Print(L"\r\n Selection (actual %s): ",
        DevLocToString(pLoader, &actualDevLoc)
        );

    // Get selection
    do
    {
        key = ReadKeyboardCharacter();
        if (key == L'\0') continue;
    }
    while ((key < L'1') || (key >= (L'1' + ix)));

    // Print out selection character
    Print(L"%c\r\n", key);

    // Save new device
    actualDevLoc = devLoc[key - L'1'];
    if (kitlDevice) {
        pLoader->kitlDevice = actualDevLoc;
    }
    else {
        pLoader->bootDevice = actualDevLoc;
    }

    // Print info...
    Print(L" %s Device is set to %s.\r\n",
        kitlDevice ? L"KITL" : L"Boot",
        kitlDevice ? pLoader->pDevices[key - L'0'].name : pLoader->pDevices[key - L'1'].name
        );

//******************************************************************************
// If Pci and NIC support, we should use formal tool to present device string

#if 0
    Print(L" %s Device is set to %s.\r\n",
        kitlDevice ? L"KITL" : L"Boot", DevLocToString(pLoader, &actualDevLoc)
        );
#endif
//******************************************************************************
}

/**
  SaveSettings(), This function called from "Save Settings" Main menu to save any change
  in local Boot Config value to GB for booting.

  @param  pContext      Point to a BootLoader_t structure that temporary store config value.

  @param  pActionMenu   Point to a submenu for Debug port.

**/
static
void
SaveSettings(
    void *pContext,
    void *pActionMenu
    )
{
    BootLoader_t *pLoader = pContext;

    UNREFERENCED_PARAMETER(pActionMenu);

    if (WriteConfigToDisk(pLoader)) {
        Print(L" Current settings has been saved\r\n");
    }
    else{
        Print(L" ERROR: Settings save failed!\r\n");
    }
}

/**
  WriteConfigToDisk(), This function called by SaveSettings() Main menu to save
  local Boot Config value to GB for booting.

  @param  pLoader   A pointer to the BootLoader_t content.

  @return bool_t    A BOOL value to indicate Save GB is success or not.

**/
static
bool_t
WriteConfigToDisk(
    BootLoader_t *pLoader
    )
{
    BootConfig_t    *pBootConfig;
    BootConfig_t    BootConfig;
    bool_t          rc = true;

    DBGMSG(DBGZONE_INFO, (L"+WriteConfigToDisk()\n"));

//******************************************************************************
// Not sure way WEC7 bldr code has to check BootConfig-> Signature to decide
// save value or not, if the Global Variable is never be using, this function
// will always skip save operation, so we do initial Global Variable in
// DefaultConfig() to avoid this issue.

    if(!CheckIsGlobalVarVaild(&BootConfig)) {
        rc = false;
        goto cleanup;
    }
//******************************************************************************
    pBootConfig = &BootConfig;

    // set change value for restore to GV
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

    if(EFI_ERROR(UpdateGlobalVariables(pBootConfig)))
        rc = false;

cleanup:
    DBGMSG(DBGZONE_INFO, (L"-WriteConfigToDisk(), rc = 0x%X\n", rc));
    // Done
    return rc;
}