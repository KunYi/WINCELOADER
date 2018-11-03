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
#include <oal_args.h>

#include <DisplayAPI.h>
#include <ini.h>

BOOL LoadBINFile(PCHAR pFileName, PULONG pImageLoc);
void FinalBootArgsSet(BootLoader_t *pLoader);

void BootJumpTo(uint32_t address);
void ScreenLog(wcstring_t format, ...);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Function:  OEMBootMain - Called from BootMain in <bsp>\src\boot\core\common\main.c
//
void OEMBootMain(   )
{
	BootLoader_t *pContext = NULL;
	ULONG address;
	int rc;
	int BestWidth, BestHeight, BestColor;

    BootLog(
        L"Microsoft Windows CE CLoader Version %d.%d (Built %S %S)\r\n",
        VERSION_MAJOR, VERSION_MINOR, __DATE__, __TIME__
        );
    

	// Initialize the default boot arg settings.
	BootArgsInit(TRUE);

	//
    // Call OEMBootInit to query hardware, set default args and init the file system.
	//
    pContext = OEMBootInit();   
	if (pContext == NULL) goto powerOff;

	//
	// Now, we can read the INI file.
	//
	rc = BootArgsLoadIniFileData(pContext);

	//
	// Time to initialize the display.
	//
	if ((pContext->displayWidth != 0) && (pContext->displayHeight != 0))
	{
		DEBUGMSG (ZONE_INFO, (L"OEMBootInit:: Calling SetDisplayMode.  (%d x %d x %d)\r\n",
				  pContext->displayWidth, pContext->displayHeight, pContext->displayBpp));

		// Init the display

//		rc = SetDisplayMode (pContext->displayWidth, pContext->displayHeight, pContext->displayBpp, 
//		                     &pContext->wVideoMode, FALSE);
		rc = SetDisplayMode (pContext->displayWidth, pContext->displayHeight, pContext->displayBpp, 
		                     &pContext->wVideoMode, &BestWidth, &BestHeight, &BestColor, FALSE);

		if (rc == 0)
		{
			// Display the splash screen.
			rc = DrawSplashImage(IniParams.szSplashFile);
		}
		else
		{
			DEBUGMSG (ZONE_ERROR, (L"OEMBootInit:: SetDisplayMode failed for resolution H:%d W:%d D:%d. rc = %d\r\n",
			          pContext->displayWidth, pContext->displayHeight, pContext->displayBpp, rc));

#ifdef WCELDRS
			// If this is the serial loader, a fail of setting the video causes a blank screen. 
			// At least tells the user why.
			ScreenLog (L"Failed to set resolution H:%d W:%d D:%d. rc = %d\r\n",
					  pContext->displayWidth, pContext->displayHeight, pContext->displayBpp, rc);//
//			ScreenLog (L"Check output on COM%d for supported modes.\r\n", pContext->comPort);
#endif
			// This will dump the available video modes. Use BootLog so that the data will always be printed.
			BootLog (L"\r\nThe BIOS reports the following video modes.\r\n");
			DumpDisplayModes ();

			// Try to set best resolution possible.
			pContext->displayWidth = BestWidth;
			pContext->displayHeight = BestHeight;
			pContext->displayBpp = BestColor;

			BootLog (L"Attempting to set resolution H:%d W:%d D:%d.\r\n",
					  pContext->displayWidth, pContext->displayHeight, pContext->displayBpp);

			rc = SetDisplayMode (pContext->displayWidth, pContext->displayHeight, pContext->displayBpp, 
			                     &pContext->wVideoMode, &BestWidth, &BestHeight, &BestColor, FALSE);
			if (rc == 0)
			{
				// Display the splash screen.
				rc = DrawSplashImage(IniParams.szSplashFile);
			}
			else
			{
				DEBUGMSG (ZONE_ERROR, (L"OEMBootInit:: SetDisplayMode failed for backup resolution H:%d W:%d D:%d. rc = %d\r\n",
						  BestWidth, BestHeight, BestColor, rc));
			}
		}
	}
	else
	{
			DEBUGMSG (ZONE_ERROR, (L"OEMBootInit:: Headless mode. One H:W:D value not specified.  (%d x %d x %d)\r\n",
				pContext->displayWidth, pContext->displayHeight, pContext->displayBpp));
	}

	//
	// Load the image.
	//
	if (!LoadBINFile(IniParams.szImage, &address))
	{
		DEBUGMSG (ZONE_ERROR, (L"OEMBootInit:: Failed to load image file %S\r\n",IniParams.szImage));
		goto powerOff;
	}
	//
	// Final init of the BootArgs
	//
	DEBUGMSG (ZONE_INFO, (L"Calling FinalBootArgsSet pContext: %x \r\n", pContext));
	FinalBootArgsSet(pContext);

	//		
	// Jump to physical address
	//
	DEBUGMSG (ZONE_INFO, (L"Jumping to address: %x\r\n", address));
    BootJumpTo(address);

    BootLog(L"Bootloader returned from image jump!!!\r\n");

powerOff:
    BootLog(L"Bootloader terminating in file:%s  line:%d.\r\nHALT\r\nHALT\r\nHALT\r\n", __FILE__, __LINE__);
	OEMBootPowerOff(pContext);
    for(;;);
}
