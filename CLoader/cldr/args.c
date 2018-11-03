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
#include <oal_args.h>
#include <bootArg.h>
#include <bootlog.h> //bootlog.h needs to be after bootarg.h
#include <ceddk.h>

#include <parser.h>
#include <FileSysAPI.h>
#include <DisplayAPI.h>
#include "ini.h"

// Maximum file length
#define MAX_FILE_NAME_LEN           12              // TODO: reslove FAT32 question

// Maximum IP address length
#define MAX_IP_ADDRESS_LEN          15              


// INI defaults
#define DEFAULT_BIN_FILE_NAME       "nk.bin"		// Default BIN file name (to be loaded)
#define DEFAULT_BAK_FILE_NAME       "nk_bak.bin"
#define DEFAULT_SPLASH_NAME         "splash.bmp"
 
#define DEFAULT_DEBUG_ZONE          0x00000003      // DBGZONE_ERROR, DBGZONE_WARNING
#define DEFAULT_DEVICE_NAME_ROOT    "CEPC"


#define DEFAULT_COMPORT             1
#define DEFAULT_BAUDRATE            BD38400

#define DEFAULT_DISPLAY_WIDTH       1024
#define DEFAULT_DISPLAY_HEIGHT      768
#define DEFAULT_DISPLAY_DEPTH       16


// Baud divisors
#define BD14400     8
#define BD16457     7
#define BD19200     6
#define BD23040     5
#define BD28800     4
#define BD38400     3
#define BD57600     2
#define BD115200    1

// INI Parameter names
#define INIPARAM_BIN_FILE           (PCHAR)"BinFile"
#define INIPARAM_BAK_BIN_FILE       (PCHAR)"BakBinFile"
#define INIPARAM_DEVICE_NAME_ROOT   (PCHAR)"DeviceNameRoot"
#define INIPARAM_DEBUG_ZONE         (PCHAR)"DebugZone"
#define INIPARAM_VIDEO              (PCHAR)"Video"
#define INIPARAM_DISPLAY_WIDTH      (PCHAR)"DisplayWidth"
#define INIPARAM_DISPLAY_HEIGHT     (PCHAR)"DisplayHeight"
#define INIPARAM_PHYSICAL_WIDTH     (PCHAR)"PhysicalWidth"
#define INIPARAM_PHYSICAL_HEIGHT    (PCHAR)"PhysicalHeight"
#define INIPARAM_DISPLAY_DEPTH      (PCHAR)"DisplayDepth"
#define INIPARAM_SPLASH_FILE        (PCHAR)"SplashFile"
//#define INIPARAM_FLASH_BACKUP       (PCHAR)"FlashBackup"
#define INIPARAM_ETH_IRQ            (PCHAR)"EthIRQ"
#define INIPARAM_ETH_IO             (PCHAR)"EthIO"
#define INIPARAM_DBG_IP             (PCHAR)"DbgIP"
#define INIPARAM_COM_PORT           (PCHAR)"COMPort"
#define INIPARAM_BAUDRATE           (PCHAR)"Baudrate"
#define INIPARAM_VIDEO              (PCHAR)"Video"
//#define INIPARAM_DELAY              (PCHAR)"Delay"

int LoadIniFile(char *pszFileName, PBYTE *ppIniData);
int SetUserBootArgs(BootLoader_t *pLoader);
int ParseIP(BOOT_ARGS * pBootArgs);
ULONG ParseDec(PCHAR str, ULONG * pVal);

// Boot parameters (from the .ini file)
INI_PARAMS IniParams;

//
// Set the following vars as a const vol so that we can use FIXUP var in .bib file to set.
//
DWORD volatile const dwDefDebugSerialPort = (DWORD)-1; 
//DWORD volatile const dwDefDebugSerialBaud = (DWORD)-1; 
DWORD volatile const dwDefDebugZones = (DWORD)-1; 
DWORD volatile const dwDefVideoWidth = (DWORD)-1; 
DWORD volatile const dwDefVideoHeight = (DWORD)-1; 
DWORD volatile const dwDefVideoDepth   = (DWORD)-1; 
DWORD volatile const dwFlagIgnoreIniFile = (DWORD)-1; 

BOOT_ARGS *pBootArgs;

//------------------------------------------------------------------------------
//
//
void BootArgsInit(bool_t force)
{
    BOOT_ARGS **ppBootArgs = (BOOT_ARGS **)IMAGE_SHARE_BOOT_ARGS_PTR_PA;
    pBootArgs = (BOOT_ARGS *)IMAGE_SHARE_BOOT_ARGS_PA;
	
    if ((pBootArgs->dwSig != BOOTARG_SIG) || (pBootArgs->dwLen < sizeof(BOOT_ARGS)) ||
        (*ppBootArgs != pBootArgs))
    {
        force = true;
    }

    if (force)
    {
        *ppBootArgs = pBootArgs;
        memset(pBootArgs, 0, sizeof(*pBootArgs));
        pBootArgs->dwSig = BOOTARG_SIG;
        pBootArgs->dwLen = sizeof(BOOT_ARGS);
        pBootArgs->dwVersionSig = BOOT_ARG_VERSION_SIG;
        pBootArgs->MajorVersion = BOOT_ARG_MAJOR_VER;
        pBootArgs->MinorVersion = BOOT_ARG_MINOR_VER;

//    // Debug
//    pBootArgs->ucEdbgAdapterType   = EDBG_ADAPTER_DEFAULT;
		if (dwDefDebugSerialPort != -1)
			pBootArgs->ucComPort           = (BYTE)dwDefDebugSerialPort;
		else
			pBootArgs->ucComPort           = DEFAULT_COMPORT;

		//if (dwDefDebugSerialBaud != -1)
  //          pBootArgs->ucBaudDivisor       = dwDefDebugSerialBaud;
		//else
  //          pBootArgs->ucBaudDivisor       = DEFAULT_BAUDRATE;

		// If the default var was set in .bib file, use it.
		if (dwDefDebugZones != -1)
			dpCurSettings.ulZoneMask       = dwDefDebugZones;
		else
			dpCurSettings.ulZoneMask       = DEFAULT_DEBUG_ZONE;

		//
		// Video settings. The defaults can be overridden by fixup vars.
		//
		if (dwDefVideoWidth != 0xffffffff)
			pBootArgs->cxDisplayScreen     = (WORD)dwDefVideoWidth;
		else
			pBootArgs->cxDisplayScreen     = DEFAULT_DISPLAY_WIDTH;

		if (dwDefVideoHeight != 0xffffffff)
			pBootArgs->cyDisplayScreen     = (WORD)dwDefVideoHeight;
		else
			pBootArgs->cyDisplayScreen     = DEFAULT_DISPLAY_HEIGHT;

		if (dwDefVideoDepth != 0xffffffff)
			pBootArgs->bppScreen           = (WORD)dwDefVideoDepth;
		else
			pBootArgs->bppScreen           = DEFAULT_DISPLAY_DEPTH;

	}
	// Default file names
	strcpy (IniParams.szImage, DEFAULT_BIN_FILE_NAME);
	strcpy (IniParams.szBackupImage, DEFAULT_BAK_FILE_NAME);
	strcpy (IniParams.szSplashFile, DEFAULT_SPLASH_NAME);
}
//------------------------------------------------------------------------------

int BootArgsLoadIniFileData(BootLoader_t *pLoader)
{
	int rc;
	PBYTE pIniData = 0;
    BOOT_ARGS* pBootArgs = (BOOT_ARGS *)IMAGE_SHARE_BOOT_ARGS_PA;
	BYTE ucCurrDebugPort;

	//DEBUGMSG(ZONE_FUNC,((L"BootArgsLoadIniFileData++\r\n")));

	// 
	// See if the fixup var is set to ignore the INI file.
	//
	if (dwFlagIgnoreIniFile == 1)
	{
		RETAILMSG (ZONE_INFO, (L"Ignore INI file fixup var set. Not loading INI file.\r\n"));
		goto error_return;
	}

	// Save the debug port.
	ucCurrDebugPort = pLoader->comPort;

	// Read the INI file into a buffer.
	rc = LoadIniFile("BOOT.INI", &pIniData);  
	if ((rc != 0) || (pIniData == NULL))
	{
		DEBUGMSG(ZONE_ERROR,((L"BootArgsLoadIniFileData:: Failed to LoadIniFile rc %d\r\n"), rc));
		goto error_return;
	}

	// Parse the loaded INI data for tokens.
	rc = SetUserBootArgs(pLoader);
	if (rc != 0)
	{
		DEBUGMSG(ZONE_ERROR,((L"BootArgsLoadIniFileData:: Failed in SetUserBootArgs rc %d\r\n"), rc));
	}

	// See if debug port changed. If so, reinit the port.
	if (ucCurrDebugPort != pLoader->comPort)
	{
	    InitSerialEcho(pLoader->comPort);
		pBootArgs->ucComPort = pLoader->comPort;
	}

error_return:
	if (pIniData)
		BootFree (pIniData);

	DEBUGMSG(ZONE_FUNC,((L"BootArgsLoadIniFileData-- rc = %d\r\n"), rc));
    return rc;
}

//------------------------------------------------------------------------------
//
// Final stuffing of bootargs just before jumping to image.
//
void FinalBootArgsSet(BootLoader_t *pLoader)
{
	int nVidWidth, nVidHeight, nVidColors;
	BYTE rs, gs, bs, rp, gp, bp;
	PBYTE pVidImageBase;
	DWORD dwStride;
	WORD wMode;

	DEBUGMSG(ZONE_FUNC,((L"BootArgsLoadIniFileData++\r\n")));

	DEBUGMSG(1,((L"BootArgsLoadIniFileData++  %x\r\n"), pBootArgs));
	// Get the video info.
	GetVidModeInfo(&nVidWidth, &nVidHeight, &nVidColors, &dwStride, 
	               &pVidImageBase, &rp, &rs, &gp, &gs, &bp, &bs, &wMode);

	DEBUGMSG(1,((L"[%x]  %d %d %d %d %x\r\n"), wMode, nVidWidth, nVidHeight, nVidColors, dwStride, pVidImageBase));

	pBootArgs->pvFlatFrameBuffer = (DWORD)pVidImageBase;
	pBootArgs->vesaMode = (WORD)wMode;
	pBootArgs->cxDisplayScreen = (WORD)nVidWidth;
	pBootArgs->cyDisplayScreen = (WORD)nVidHeight;
	pBootArgs->cxPhysicalScreen = (WORD)nVidWidth;
	pBootArgs->cyPhysicalScreen = (WORD)nVidHeight;
	pBootArgs->cbScanLineLength = (WORD)dwStride;
	pBootArgs->bppScreen = (WORD)nVidColors;
	pBootArgs->RedMaskSize = (UCHAR)rs;
	pBootArgs->RedMaskPosition = (UCHAR)rp;
	pBootArgs->GreenMaskSize = (UCHAR)gs;
	pBootArgs->GreenMaskPosition = (UCHAR)gp;
	pBootArgs->BlueMaskSize = (UCHAR)bs;
	pBootArgs->BlueMaskPosition = (UCHAR)bp;
	
	//(db) supported in CE7 pBootArgs->VideoRam.QuadPart = pLoader->videoRam;
    
	//if (pLoader->displayWidth == 0)
	//{
	//	pBootArgs->cxDisplayScreen = (WORD)nVidWidth;
	//	pBootArgs->cyDisplayScreen = (WORD)nVidHeight;
	//}
	//else
	//{
	//	pBootArgs->cxDisplayScreen = (WORD)pLoader->displayWidth;
	//	pBootArgs->cyDisplayScreen = (WORD)pLoader->displayHeight;
	//}

	DEBUGMSG(ZONE_FUNC,((L"BootArgsLoadIniFileData--\r\n")));
    return;
}

//------------------------------------------------------------------------------
//
// Load INI file and initialize the parser
//
int LoadIniFile(char *pszFileName, PBYTE *ppIniData)
{
	int rc = 0;
    DWORD dwIniFileSize;
    PBYTE pIniData;

	DEBUGMSG(ZONE_FUNC,((L"LoadIniFile++  File: %S\r\n"), pszFileName));

	*ppIniData = NULL;

    // Open the BMP
	dwIniFileSize = FSOpenFile (pszFileName);
    if (dwIniFileSize == 0)
    {
        DEBUGMSG(ZONE_ERROR,(L"Failed to INI file. %S\r\n", pszFileName));
		rc = -1;
		goto error_return;
    }

	// Allocate a buffer to store the INI data
	pIniData = (PBYTE)BootAlloc (dwIniFileSize + 2);
	if (pIniData == 0)
	{
		DEBUGMSG(ZONE_ERROR,(L"Failed to alloc space for INI file. File size %d \r\n", dwIniFileSize));
		rc = -7;
		goto error_return;
	}

    // A boot.ini exists - extract options to override defaults.
	DEBUGMSG(ZONE_INFO,((L"LoadIniFile: Reading INI file. Size: %d\r\n"), dwIniFileSize));
	if (!FSReadFile ((PBYTE)pIniData, dwIniFileSize))
    {
        DEBUGMSG(ZONE_ERROR,(L"Failed to read INI file.\r\n"));
		rc = -2;
		goto error_return;
    }
	// Terminate the data
	*(pIniData + dwIniFileSize) = '\0';   
            
    // Prepare the parser 
    IniParserInit((PCHAR)pIniData, dwIniFileSize);

    *ppIniData = pIniData;

    // Close the file now that we're done with it
error_return:
	FSCloseFile ();

	DEBUGMSG(ZONE_FUNC,((L"LoadIniFile--  rc: %d\r\n"), rc));
    return rc;
}

//------------------------------------------------------------------------------
// Parses user settings into given BOOT_ARGS structure.
//
int SetUserBootArgs(BootLoader_t *pLoader)
{
    BOOL bTmp = FALSE;
    DWORD dwTmp = 0;

    // Parse file names
    if (GetIniString(INIPARAM_BIN_FILE, (PCHAR)IniParams.szImage, MAX_FILE_NAME_LEN + 1) == INIPARSE_PARSE_ERROR)
    {
        DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_BIN_FILE));
    }
    if (GetIniString(INIPARAM_BAK_BIN_FILE, (PCHAR)IniParams.szBackupImage, MAX_FILE_NAME_LEN + 1) == INIPARSE_PARSE_ERROR)
    {
        DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_BAK_BIN_FILE));
    }

    // Parse CEPC name
    if (GetIniString(INIPARAM_DEVICE_NAME_ROOT, (PCHAR)pBootArgs->szDeviceNameRoot, MAX_DEV_NAMELEN) == INIPARSE_PARSE_ERROR)
    {
       DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_DEVICE_NAME_ROOT));
    }

    // Parse DebugMask
    if (GetIniDword(INIPARAM_DEBUG_ZONE, &dpCurSettings.ulZoneMask) == INIPARSE_PARSE_ERROR)
    {
        DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_DEBUG_ZONE));
    }
    
    //// Parse Delay
    //if (GetIniDword(INIPARAM_DELAY, (PDWORD)&IniParams.Delay) == INIPARSE_PARSE_ERROR)
    //{
    //    DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_DELAY));
    //}
    
    // Parse video settings
    if (GetIniBool(INIPARAM_VIDEO, &bTmp) == INIPARSE_PARSE_ERROR)
    {
        DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_VIDEO));
    }
    else
    {
        if (bTmp)
        {
            pLoader->wVideoMode         = 0;  //We'll get the actual mode later.

            // Video "on"
            if (GetIniDword(INIPARAM_DISPLAY_WIDTH, &pLoader->displayWidth) == INIPARSE_PARSE_ERROR)
            {
                DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_DISPLAY_WIDTH));
            }
            if (GetIniDword(INIPARAM_DISPLAY_HEIGHT, &pLoader->displayHeight) == INIPARSE_PARSE_ERROR)
            {
                DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_DISPLAY_HEIGHT));
            }
            if (GetIniDword(INIPARAM_PHYSICAL_WIDTH, &pLoader->displayPhysicalWidth) == INIPARSE_PARSE_ERROR)
            {
                DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_PHYSICAL_WIDTH));
            }
            if (GetIniDword(INIPARAM_PHYSICAL_HEIGHT, &pLoader->displayPhysicalHeight) == INIPARSE_PARSE_ERROR)
            {
                DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_PHYSICAL_HEIGHT));
            }
            if (pLoader->displayPhysicalWidth > 0 &&
                pLoader->displayPhysicalWidth < pLoader->displayWidth)
            {
                DEBUGMSG(ZONE_WARN,(L"Setting DisplayWidth = PhysicalWidth; PhysicalWidth = 0\r\n"));
                pLoader->displayWidth = pLoader->displayPhysicalWidth;
                //pLoader->displayPhysicalWidth = 0;
            }
			if ((pLoader->displayPhysicalWidth > 0) && (pLoader->displayWidth == 0))
				pLoader->displayWidth = pLoader->displayPhysicalWidth;

            if (pLoader->displayPhysicalHeight > 0 &&
                pLoader->displayPhysicalHeight < pLoader->displayHeight)
            {
                DEBUGMSG(ZONE_WARN,((L"Setting DisplayHeight = PhysicalHeight; PhysicalHeight = 0\r\n")));
                pLoader->displayHeight = pLoader->displayPhysicalHeight;
                //pLoader->displayPhysicalHeight = 0;
            }
			if ((pLoader->displayPhysicalHeight > 0) && (pLoader->displayHeight == 0))
				pLoader->displayHeight = pLoader->displayPhysicalHeight;

            if (GetIniDword(INIPARAM_DISPLAY_DEPTH, &pLoader->displayBpp) == INIPARSE_PARSE_ERROR)
            {
                DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_DISPLAY_DEPTH));
            }
			pBootArgs->bppScreen = pLoader->displayBpp;
			
			if (GetIniString(INIPARAM_SPLASH_FILE, (PCHAR)IniParams.szSplashFile, MAX_FILE_NAME_LEN + 1) == INIPARSE_PARSE_ERROR)
			{
				DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_SPLASH_FILE));
			}
        }
        else
        {
            // Let the main() know we don't want video
            pLoader->wVideoMode         = 0xFFFF;

            pLoader->displayWidth = 0;
            pLoader->displayPhysicalWidth = 0;
            pLoader->displayHeight = 0;
            pLoader->displayPhysicalHeight = 0;
		
		}
    }

    // Parse Ethernet settings
    if (GetIniByte(INIPARAM_ETH_IRQ, &pBootArgs->ucEdbgIRQ) == INIPARSE_PARSE_ERROR)
    {
        DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\n"), INIPARAM_ETH_IRQ));
    }
    
    if (GetIniDword(INIPARAM_ETH_IO, &pBootArgs->dwEdbgBaseAddr) == INIPARSE_PARSE_ERROR)
    {
        DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\n"), INIPARAM_ETH_IO));
    }
    
    if (ParseIP(pBootArgs) == INIPARSE_PARSE_ERROR)
    {
        DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\n"), INIPARAM_DBG_IP));
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
    if (GetIniByte(INIPARAM_COM_PORT, &pLoader->comPort) == INIPARSE_PARSE_ERROR)
    {
        DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_COM_PORT));
    }
    else
    {
        // Ensure COM port is valid.  We support up to COM 4.
	// If invalid then we will revert to default
	    
	if( pLoader->comPort > 4 )
	{
            DEBUGMSG(ZONE_WARN,((L"Invalid COM port specified in BOOT.INI.\n")));
            pLoader->comPort = DEFAULT_COMPORT;
        }
    }
    
#if 0   // No support in BOOT_ARGS for serial debugging setting so far...
    if (GetIniDword(INIPARAM_BAUDRATE, &dwTmp) == BLSTATUS_PARSE_ERROR)
    {
        DEBUGMSG(ZONE_WARN,((L"Cannot parse parameter: %s\r\n"), INIPARAM_BAUDRATE));
    }
    else
    {
        // Convert the baudrate to divisor
        for (i = 0; i < BAUDRATE_TABLE_SIZE; i++)
        {
            if (dwTmp == BAUDRATE_TABLE[i].ulBaudrate)
            {
                pLoader->baudDivisor = BAUDRATE_TABLE[i].ucDivisor;
                break;
            }
        }
    }
#endif

    DEBUGMSG(ZONE_INFO, (L"DisplayWidth.....: %d\r\n",    pLoader->displayWidth));
    DEBUGMSG(ZONE_INFO, (L"DisplayHeight....: %d\r\n",    pLoader->displayHeight));
    DEBUGMSG(ZONE_INFO, (L"DisplayDepth.....: %d\r\n",    pLoader->displayBpp));
    DEBUGMSG(ZONE_INFO, (L"PhysicalWidth....: %d\r\n",    pLoader->displayPhysicalWidth));
    DEBUGMSG(ZONE_INFO, (L"PhysicalHeight...: %d\r\n",    pLoader->displayPhysicalHeight));
    DEBUGMSG(ZONE_INFO, (L"SerialPort.......: COM%d\r\n", pLoader->comPort));
    //DEBUGMSG(ZONE_INFO, (L"Baudrate divisor.: %d\r\n",    pLoader->baudDivisor));
    DEBUGMSG(ZONE_INFO, (L"Ethernet IRQ.....: 0x%x\r\n",  pBootArgs->ucEdbgIRQ));
    DEBUGMSG(ZONE_INFO, (L"Ethernet base....: 0x%x\r\n",  pBootArgs->dwEdbgBaseAddr));
    DEBUGMSG(ZONE_INFO, (L"IP...............: %d.%d.%d.%d\r\n", 
              pBootArgs->EdbgAddr.dwIP & 0xFF,
             (pBootArgs->EdbgAddr.dwIP >>  8) & 0xFF,
             (pBootArgs->EdbgAddr.dwIP >> 16) & 0xFF,
             (pBootArgs->EdbgAddr.dwIP >> 24) & 0xFF));
    DEBUGMSG(ZONE_INFO, (L"Image file.......: %S\r\n",    IniParams.szImage));
    DEBUGMSG(ZONE_INFO, (L"Backup image.....: %S\r\n",    IniParams.szBackupImage));
    DEBUGMSG(ZONE_INFO, (L"Splash image.....: %S\r\n",    IniParams.szSplashFile));
    DEBUGMSG(ZONE_INFO, (L"DeviceNameRoot...: %s\r\n",    pBootArgs->szDeviceNameRoot));
    DEBUGMSG(ZONE_INFO, (L"DebugZone........: 0x%x\r\n",  dpCurSettings.ulZoneMask));
//    DEBUGMSG(ZONE_INFO, (L"LoaderFlags......: 0x%x\n",  pLoader->ucLoaderFlags));
    
    return 0;
}

//------------------------------------------------------------------------------
// Parse and set the debug IP addres
//
int ParseIP(BOOT_ARGS * pBootArgs)
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

    return 0;
}

//------------------------------------------------------------------------------


