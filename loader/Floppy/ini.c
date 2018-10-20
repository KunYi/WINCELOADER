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
//------------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <bootarg.h>
#include <parser.h>
#include <util.h>
#include "bldr.h"
#include "debug.h"
#include "fs.h"
#include "ini.h"
#include "video.h"
#include "decoder.h"

// Boot parameters (from the .ini file)
INI_PARAMS IniParams;

// Private functions
BLSTATUS ParseIP(BOOT_ARGS * pBootArgs);

// BIOS functions
ULONG BIOS_ReadKeyboardCharacter();
ULONG BIOS_Delay();

// Baudrate to divisor conversion
typedef struct _BAUDRATE_TABLE_ENTRY
{
    ULONG   ulBaudrate;
    UCHAR   ucDivisor;
} BAUDRATE_TABLE_ENTRY;

#if 0   // No support in BOOT_ARGS for serial debugging setting so far...
BAUDRATE_TABLE_ENTRY BAUDRATE_TABLE [] =
{
    {  14400,   BD14400  },
    {  19200,   BD19200  },
    {  28800,   BD28800  },
    {  38400,   BD38400  },
    {  57600,   BD57600  },
    { 115200,   BD115200 }
};

#define BAUDRATE_TABLE_SIZE (sizeof(BAUDRATE_TABLE)/sizeof(BAUDRATE_TABLE_ENTRY))
#endif

// Private functions
BLSTATUS SetDefaultBootArgs(BOOT_ARGS * pBootArgs);
BLSTATUS SetUserBootArgs(BOOT_ARGS * pBootArgs);
BLSTATUS LoadIniFile();

/**
 * Initialize the shared memory used by the bootloader and the OS to
 * communicate settings (video mode, network settings, etc.).
 */
BLSTATUS InitBootArgs(BOOT_ARGS * pBootArgs)
{
    SetDefaultBootArgs(pBootArgs);

    if (LoadIniFile() == BLSTATUS_OK)
    {
        SetUserBootArgs(pBootArgs);
    }

    return BLSTATUS_OK;
}

/**
 * Set the default BIOS loader options
 */
BLSTATUS SetDefaultBootArgs(BOOT_ARGS * pBootArgs)
{
    // Check parameters
    if (pBootArgs == NULL)
        return TRUE;

    // Set the default INI options
    memset(pBootArgs, 0, sizeof(BOOT_ARGS));

    // Boot block signature and length
    pBootArgs->dwSig               = BOOTARG_SIG;
    pBootArgs->dwVersionSig        = BOOT_ARG_VERSION_SIG;
    pBootArgs->MajorVersion        = BOOT_ARG_MAJOR_VER;
    pBootArgs->MinorVersion        = BOOT_ARG_MINOR_VER;
    pBootArgs->dwLen               = sizeof(BOOT_ARGS);

    // Debug
    pBootArgs->ucEdbgAdapterType   = EDBG_ADAPTER_DEFAULT;
    pBootArgs->ucComPort           = DEFAULT_COMPORT;
    pBootArgs->ucBaudDivisor       = DEFAULT_BAUDRATE;
    dwZoneMask                     = DEFAULT_DEBUG_ZONE;

    // Video
    pBootArgs->cxDisplayScreen     = DEFAULT_DISPLAY_WIDTH;
    pBootArgs->cyDisplayScreen     = DEFAULT_DISPLAY_HEIGHT;
    pBootArgs->bppScreen           = DEFAULT_DISPLAY_DEPTH;

    // Boot
    StrNCpy(pBootArgs->szDeviceNameRoot, DEFAULT_DEVICE_NAME_ROOT, MAX_DEV_NAMELEN);
    StrNCpy(IniParams.szImage, DEFAULT_BIN_FILE_NAME, MAX_FILE_NAME_LEN);

    IniParams.Delay                = DEFAULT_DELAY;

    pBootArgs->ucLoaderFlags       = 0;         // Flags set by loader
    pBootArgs->ucPCIConfigType     = 1;

    return BLSTATUS_OK;
}

/**
 * Load INI file and initialize the parser
 */
BLSTATUS LoadIniFile()
{
    BLSTATUS status = BLSTATUS_NO_INI_FILE;
    ULONG IniFileLen = 0;

    // Look for an optional boot.ini file to feed us user options
    if ((IniFileLen = BINCOMPRESS_OPEN_FILE(OPTIONS_FILE_NAME)) != 0)
    {
        PUCHAR pTmp = NULL;
        USHORT nCount = 0;

        // We don't support arbitrarily long ini files
        if (IniFileLen > INI_RAW_DATA_LENGTH - 1)
        {
            WARNMSG(MSG_TOO_LONG_INI_FILE, ("INI file longer than %d bytes\n", INI_RAW_DATA_LENGTH));
            IniFileLen = INI_RAW_DATA_LENGTH;
        }

        // A boot.ini exists - extract options to override defaults.
        if (FSReadFile(INI_RAW_DATA_BUFFER, IniFileLen))
        {
            // Put '\0' at the end of the INI data
            INI_RAW_DATA_BUFFER[IniFileLen] = '\0';

            // Prepare the parser
            IniParserInit(INI_RAW_DATA_BUFFER, IniFileLen);

            status = BLSTATUS_OK;
        }

        // Close the file now that we're done with it
        FSCloseFile();
    }
    else
    {
        WARNMSG(MSG_CANNOT_OPEN_INI_FILE, ("Cannot open INI file: %s\n", OPTIONS_FILE_NAME));
    }

    return status;
}

/**
 * Parses user settings into given BOOT_ARGS structure.
 */
BLSTATUS SetUserBootArgs(BOOT_ARGS * pBootArgs)
{
    BOOL bTmp = FALSE;
    DWORD dwTmp = 0;

    // Parse file names
    if (GetIniString(INIPARAM_BIN_FILE, IniParams.szImage, MAX_FILE_NAME_LEN + 1) == INIPARSE_PARSE_ERROR)
    {
        WARNMSG(MSG_INIWARN_BIN_FILE, (("Cannot parse parameter: %s\n"), INIPARAM_BIN_FILE));
    }
    if (GetIniString(INIPARAM_BAK_BIN_FILE, IniParams.szBackupImage, MAX_FILE_NAME_LEN + 1) == INIPARSE_PARSE_ERROR)
    {
        WARNMSG(MSG_INIWARN_BAK_BIN_FILE, (("Cannot parse parameter: %s\n"), INIPARAM_BAK_BIN_FILE));
    }

    // Parse CEPC name
    if (GetIniString(INIPARAM_DEVICE_NAME_ROOT, pBootArgs->szDeviceNameRoot, MAX_DEV_NAMELEN) == INIPARSE_PARSE_ERROR)
    {
       WARNMSG(MSG_INIWARN_DEVICE_NAME_ROOT, (("Cannot parse parameter: %s\n"), INIPARAM_DEVICE_NAME_ROOT));
    }

    // Parse DebugMask
    if (GetIniDword(INIPARAM_DEBUG_ZONE, &dwZoneMask) == INIPARSE_PARSE_ERROR)
    {
        WARNMSG(MSG_INIWARN_DEBUG_ZONE, (("Cannot parse parameter: %s\n"), INIPARAM_DEBUG_ZONE));
    }

    // Parse Delay
    if (GetIniDword(INIPARAM_DELAY, &IniParams.Delay) == INIPARSE_PARSE_ERROR)
    {
        WARNMSG(MSG_INIWARN_DELAY, (("Cannot parse parameter: %s\n"), INIPARAM_DELAY));
    }

    // Parse video settings
    if (GetIniBool(INIPARAM_VIDEO, &bTmp) == INIPARSE_PARSE_ERROR)
    {
        WARNMSG(MSG_INIWARN_VIDEO, (("Cannot parse parameter: %s\n"), INIPARAM_VIDEO));
    }
    else
    {
        if (bTmp)
        {
            // Video "on"
            if (GetIniWord(INIPARAM_DISPLAY_WIDTH, &pBootArgs->cxDisplayScreen) == INIPARSE_PARSE_ERROR)
            {
                WARNMSG(MSG_INIWARN_DISPLAY_WIDTH, (("Cannot parse parameter: %s\n"), INIPARAM_DISPLAY_WIDTH));
            }
            if (GetIniWord(INIPARAM_DISPLAY_HEIGHT, &pBootArgs->cyDisplayScreen) == INIPARSE_PARSE_ERROR)
            {
                WARNMSG(MSG_INIWARN_DISPLAY_HEIGHT, (("Cannot parse parameter: %s\n"), INIPARAM_DISPLAY_HEIGHT));
            }
            if (GetIniWord(INIPARAM_PHYSICAL_WIDTH, &pBootArgs->cxPhysicalScreen) == INIPARSE_PARSE_ERROR)
            {
                WARNMSG(MSG_INIWARN_PHYSICAL_WIDTH, (("Cannot parse parameter: %s\n"), INIPARAM_PHYSICAL_WIDTH));
            }
            if (GetIniWord(INIPARAM_PHYSICAL_HEIGHT, &pBootArgs->cyPhysicalScreen) == INIPARSE_PARSE_ERROR)
            {
                WARNMSG(MSG_INIWARN_PHYSICAL_HEIGHT, (("Cannot parse parameter: %s\n"), INIPARAM_PHYSICAL_HEIGHT));
            }
            if (GetIniWord(INIPARAM_DISPLAY_DEPTH, &pBootArgs->bppScreen) == INIPARSE_PARSE_ERROR)
            {
                WARNMSG(MSG_INIWARN_DISPLAY_DEPTH, (("Cannot parse parameter: %s\n"), INIPARAM_DISPLAY_DEPTH));
            }

            if (pBootArgs->cxPhysicalScreen > 0 &&
                pBootArgs->cxPhysicalScreen < pBootArgs->cxDisplayScreen)
            {
                WARNMSG(MSG_PHYS_WIDTH_CORRECTED, ("Setting DisplayWidth = PhysicalWidth; PhysicalWidth = 0\n"));
                pBootArgs->cxDisplayScreen = pBootArgs->cxPhysicalScreen;
                pBootArgs->cxPhysicalScreen = 0;
            }

            if (pBootArgs->cyPhysicalScreen > 0 &&
                pBootArgs->cyPhysicalScreen < pBootArgs->cyDisplayScreen)
            {
                WARNMSG(MSG_PHYS_HEIGHT_CORRECTED, ("Setting DisplayHeight = PhysicalHeight; PhysicalHeight = 0\n"));
                pBootArgs->cyDisplayScreen = pBootArgs->cyPhysicalScreen;
                pBootArgs->cyPhysicalScreen = 0;
            }
        }
        else
        {
            // Let the main() know we don't want video
            pBootArgs->ucVideoMode         = 0xFF;
        }
    }

    // Parse FlashBackup
    if (GetIniBool(INIPARAM_FLASH_BACKUP, &bTmp) == INIPARSE_PARSE_ERROR)
    {
        WARNMSG(MSG_INIWARN_FLASH_BACKUP, (("Cannot parse parameter: %s\n"), INIPARAM_FLASH_BACKUP));
    }
    else if (bTmp)
    {
        pBootArgs->ucLoaderFlags |= LDRFL_FLASH_BACKUP;
    }

    // Parse Ethernet settings
    if (GetIniByte(INIPARAM_ETH_IRQ, &pBootArgs->ucEdbgIRQ) == INIPARSE_PARSE_ERROR)
    {
        WARNMSG(MSG_INIWARN_ETH_IRQ, (("Cannot parse parameter: %s\n"), INIPARAM_ETH_IRQ));
    }

    if (GetIniDword(INIPARAM_ETH_IO, &pBootArgs->dwEdbgBaseAddr) == INIPARSE_PARSE_ERROR)
    {
        WARNMSG(MSG_INIWARN_ETH_IO, (("Cannot parse parameter: %s\n"), INIPARAM_ETH_IO));
    }

    if (ParseIP(pBootArgs) == INIPARSE_PARSE_ERROR)
    {
        WARNMSG(MSG_INIWARN_DBG_IP, (("Cannot parse parameter: %s\n"), INIPARAM_DBG_IP));
    }
    else
    {
        pBootArgs->ucEdbgAdapterType = EDBG_ADAPTER_DEFAULT;
    }

    if (&pBootArgs->ucEdbgIRQ != 0 || &pBootArgs->dwEdbgBaseAddr != 0)
    {
        pBootArgs->ucLoaderFlags |= LDRFL_USE_EDBG;
    }

    // Parse serial port settings
    if (GetIniByte(INIPARAM_COM_PORT, &pBootArgs->ucComPort) == INIPARSE_PARSE_ERROR)
    {
        WARNMSG(MSG_INIWARN_COM_PORT, (("Cannot parse parameter: %s\n"), INIPARAM_COM_PORT));
    }
    else
    {
        // Ensure COM port is valid.  We support up to COM 4.
    // If invalid then we will revert to default

    if( pBootArgs->ucComPort > 4 )
    {
            WARNMSG(MSG_INVALID_COMPORT, (("Invalid COM port specified in BOOT.INI.\n")));
            pBootArgs->ucComPort = DEFAULT_COMPORT;
        }
    }

#if 0   // No support in BOOT_ARGS for serial debugging setting so far...
    if (GetIniDword(INIPARAM_BAUDRATE, &dwTmp) == BLSTATUS_PARSE_ERROR)
    {
        WARNMSG(MSG_INIWARN_BAUDRATE, (("Cannot parse parameter: %s\n"), INIPARAM_BAUDRATE));
    }
    else
    {
        // Convert the baudrate to divisor
        for (i = 0; i < BAUDRATE_TABLE_SIZE; i++)
        {
            if (dwTmp == BAUDRATE_TABLE[i].ulBaudrate)
            {
                pBootArgs->ucBaudDivisor = BAUDRATE_TABLE[i].ucDivisor;
                break;
            }
        }
    }
#endif

#if 0
    INFOMSG(MSG_UNDEFINED, ("DisplayWidth.....: %d\n",    pBootArgs->cxDisplayScreen));
    INFOMSG(MSG_UNDEFINED, ("DisplayHeight....: %d\n",    pBootArgs->cyDisplayScreen));
    INFOMSG(MSG_UNDEFINED, ("DisplayDepth.....: %d\n",    pBootArgs->bppScreen));
    INFOMSG(MSG_UNDEFINED, ("PhysicalWidth....: %d\n",    pBootArgs->cxPhysicalScreen));
    INFOMSG(MSG_UNDEFINED, ("PhysicalHeight...: %d\n",    pBootArgs->cyPhysicalScreen));
    INFOMSG(MSG_UNDEFINED, ("SerialPort.......: COM%d\n", pBootArgs->ucComPort));
    INFOMSG(MSG_UNDEFINED, ("Baudrate divisor.: %d\n",    pBootArgs->ucBaudDivisor));
    INFOMSG(MSG_UNDEFINED, ("Ethernet IRQ.....: 0x%x\n",  pBootArgs->ucEdbgIRQ));
    INFOMSG(MSG_UNDEFINED, ("Ethernet base....: 0x%x\n",  pBootArgs->dwEdbgBaseAddr));
    INFOMSG(MSG_UNDEFINED, ("IP...............: %d.%d.%d.%d\n",
        pBootArgs->EdbgAddr.dwIP & 0xFF,
        (pBootArgs->EdbgAddr.dwIP >>  8) & 0xFF,
        (pBootArgs->EdbgAddr.dwIP >> 16) & 0xFF,
        (pBootArgs->EdbgAddr.dwIP >> 24) & 0xFF));
    INFOMSG(MSG_UNDEFINED, ("Image file.......: %s\n",    IniParams.szImage));
    INFOMSG(MSG_UNDEFINED, ("Backup image.....: %s\n",    IniParams.szBackupImage));
    INFOMSG(MSG_UNDEFINED, ("DeviceNameRoot...: %s\n",    pBootArgs->szDeviceNameRoot));
    INFOMSG(MSG_UNDEFINED, ("DebugZone........: 0x%x\n",  dwZoneMask));
    INFOMSG(MSG_UNDEFINED, ("LoaderFlags......: 0x%x\n",  pBootArgs->ucLoaderFlags));
#endif

    return BLSTATUS_OK;
}

/**
 * Reads a character from keyboard and sets IniParams.szUserSelectableImage
 * accordingly.
 *
 * Returns BLSTATUS_OK when a key has been pressed and a corresponding INI entry is present.
 * Returns BLSTATUS_NO_USER_SELECTION when:
 *  - keyboard buffer was empty on function entry and
 *    no key has been pressed within <delay> seconds
 *    OR
 *  - no corresponding INI entry is present.
 */
BLSTATUS SetUserSelectableBinFile(LONG delay)
{
    CHAR iniParam[sizeof(INIPARAM_BIN_FILE) + 1] = INIPARAM_BIN_FILE;
    ULONG status;

    iniParam[sizeof(INIPARAM_BIN_FILE) - 1] = 0;
    iniParam[sizeof(INIPARAM_BIN_FILE)] = 0;

    delay *= 1000;
    do
    {
        // Read the last key from the buffer
        while( (status = BIOS_ReadKeyboardCharacter()) != 0)
        {
            iniParam[sizeof(INIPARAM_BIN_FILE) - 1] = (CHAR)(status & 0x000000FF);
        }

        if (iniParam[sizeof(INIPARAM_BIN_FILE) - 1] != 0)
        {
            break;
        }

        delay -= 100;
        BIOS_Delay();
    }
    while (delay > 0);

    // If no key or space bar has been hit - return at this point
    // and load the default image
    if (iniParam[sizeof(INIPARAM_BIN_FILE) - 1] == 0 ||
        iniParam[sizeof(INIPARAM_BIN_FILE) - 1] == 32)
    {
        return BLSTATUS_NO_USER_SELECTION;
    }

    // Check if we have corresponding INI entry
    if ((status = GetIniString(iniParam, IniParams.szUserSelectableImage, MAX_FILE_NAME_LEN + 1)) != BLSTATUS_OK)
    {
        WARNMSG(MSG_NO_BIN_FILE_ENTRY, ("%s INI entry not found\n", iniParam, status));
        return BLSTATUS_NO_USER_SELECTION;
    }

    return BLSTATUS_OK;
}


/**
 * Parse and set the debug IP addres
 */
BLSTATUS ParseIP(BOOT_ARGS * pBootArgs)
{
    ULONG i;
    CHAR Value[MAX_IP_ADDRESS_LEN + 1];
    PCHAR pValue = Value;
    ULONG charsProcessed = 0;
    DWORD dwIP;
    ULONG ulTmp;

    if ( (i = GetIniString(INIPARAM_DBG_IP, pValue, MAX_IP_ADDRESS_LEN + 1)) != INIPARSE_OK)
    {
        return i;
    }

    // Parse the first chunk
    if ((charsProcessed = ParseDec(pValue, &ulTmp)) == 0)
    {
        return INIPARSE_PARSE_ERROR;
    }
    dwIP = (ulTmp & 0xFF);

    // Parse the rest
    for(i = 1; i < 4; i++)
    {
        // Check for the dot
        pValue += charsProcessed;
        if (*pValue != '.')
        {
            return INIPARSE_PARSE_ERROR;
        }
        pValue += 1;

        // Parse the second chunk
        if ((charsProcessed = ParseDec(pValue, &ulTmp)) == 0)
        {
            return INIPARSE_PARSE_ERROR;
        }
        dwIP = dwIP | ((ulTmp & 0xFF) << (i*8));
    }

    pBootArgs->EdbgAddr.dwIP = dwIP;

    return INIPARSE_OK;
}
