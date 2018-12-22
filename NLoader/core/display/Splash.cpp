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
//#include <windows.h>
#include <boot.h>
#include <bootBios.h>
#include <bootMemory.h>
#include <bootLog.h>
#include <display.h>
#include <splash.h>

#include <FileSysAPI.h>
#include <DisplayAPI.h>

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Draw splash bitmap (if available) and progress bar frame
//
int DrawSplashImage(char *pszBmpFileName)
{   
    // BMP file data
	BITMAPFILEHEADER bmphFile;
	BITMAPINFOHEADER bmpHeader;
	DWORD dwFileLen;
    DWORD *pColors = 0;
	PBYTE pImage = 0;
    int rc = 0, row, col, y;

	if (pszBmpFileName == 0)
		pszBmpFileName = SPLASH_IMAGE_FILE_NAME;

    // Open the BMP
	dwFileLen = FSOpenFile (pszBmpFileName);
    if (dwFileLen == 0)
    {
        DEBUGMSG(ZONE_ERROR,(L"Failed to open file. %S\r\n", pszBmpFileName));
		rc = -1;
		goto error_return;
    }

	// Read the Bitmap File header
	if (!FSReadFile ((PBYTE)&bmphFile, sizeof (BITMAPFILEHEADER)))
    {
        DEBUGMSG(ZONE_ERROR,(L"Failed to read bitmap file.\r\n"));
		rc = -2;
		goto error_return;
    }
    DEBUGMSG(ZONE_INFO, (L"Bitmap file header for %S\r\n",pszBmpFileName));
    DEBUGMSG(ZONE_INFO, (L"   .bfType            0x%x\r\n", bmphFile.bfType));
    DEBUGMSG(ZONE_INFO, (L"   .bfSize            0x%x\r\n", bmphFile.bfSize));
    DEBUGMSG(ZONE_INFO, (L"   .Reserved          0x%x\r\n", bmphFile.bfReserved1));
    DEBUGMSG(ZONE_INFO, (L"   .Reserved          0x%x\r\n", bmphFile.bfReserved2));
    DEBUGMSG(ZONE_INFO, (L"   .bfOffBits         0x%x\r\n", bmphFile.bfOffBits));

	// Verify the file type.
	if (bmphFile.bfType != 'MB')
    {
        DEBUGMSG(ZONE_ERROR,(L"Bitmap is unknown format. Expected %x, found %x.\r\n", 'MB', bmphFile.bfType));
		rc = -3;
		goto error_return;
    }

    // Read the BMP header
	if (!FSReadFile ((PBYTE)&bmpHeader, sizeof (BITMAPINFOHEADER)))
    {
        DEBUGMSG(ZONE_ERROR,(L"Failed to read bitmap header.\r\n"));
		rc = -2;
		goto error_return;
    }
    DEBUGMSG(ZONE_INFO, (L"Bitmap Header data\r\n"));
    DEBUGMSG(ZONE_INFO, (L"   .biSize           %d\r\n", bmpHeader.biSize));
    DEBUGMSG(ZONE_INFO, (L"   .biWidth          %d\r\n", bmpHeader.biWidth));
    DEBUGMSG(ZONE_INFO, (L"   .biHeight         %d\r\n", bmpHeader.biHeight));
    DEBUGMSG(ZONE_INFO, (L"   .biPlanes         %d\r\n", bmpHeader.biPlanes));
    DEBUGMSG(ZONE_INFO, (L"   .biBitCount       %d\r\n", bmpHeader.biBitCount));
    DEBUGMSG(ZONE_INFO, (L"   .biCompression    %d\r\n", bmpHeader.biCompression));
    DEBUGMSG(ZONE_INFO, (L"   .biSizeImage      %d\r\n", bmpHeader.biSizeImage));
    DEBUGMSG(ZONE_INFO, (L"   .biXPelsPerMeter  %d\r\n", bmpHeader.biXPelsPerMeter));
    DEBUGMSG(ZONE_INFO, (L"   .biYPelsPerMeter  %d\r\n", bmpHeader.biYPelsPerMeter));
    DEBUGMSG(ZONE_INFO, (L"   .biClrUsed        %d\r\n", bmpHeader.biClrUsed));
    DEBUGMSG(ZONE_INFO, (L"   .biClrImportant   %d\r\n", bmpHeader.biClrImportant));

	// Verify the header
    if (bmpHeader.biSize != sizeof(BITMAPINFOHEADER))
    {
        DEBUGMSG(ZONE_ERROR,(L"Bitmap header is incorrect size. Exp %d, fnd %d.\r\n", sizeof(BITMAPINFOHEADER), bmpHeader.biSize));
		rc = -4;
		goto error_return;
    }


    // Some BMPs which use 256 colours have 0 in ColorsUsed field.
    // Correct this here
    if (bmpHeader.biClrUsed == 0)
        bmpHeader.biClrUsed = 256;
    
	// Switch by bits per pixel
	if (bmpHeader.biBitCount == 8)
	{
		// Read the color table
		pColors = (DWORD *)BootAlloc (256 * sizeof (DWORD));
		if (pColors == 0)
		{
			DEBUGMSG(ZONE_ERROR,(L"Failed to alloc space for color table.\r\n"));
			rc = -5;
			goto error_return;
		}
		if (!FSReadFile ((PBYTE)pColors, 256 * sizeof (DWORD)))
		{
			DEBUGMSG(ZONE_ERROR,(L"Failed to read color table.\r\n"));
			rc = -6;
			goto error_return;
		}

        // Convert colour table from 8bit per primary to 6bit per primary
        for (y = 0; y < (int)bmpHeader.biClrUsed * 4; y++)
        {
            ((BYTE*)pColors)[y] = ((BYTE *)pColors)[y] >> 2;
        }
	}        

#define BITBUFSIZE   512 * 8
	pImage = (PBYTE)BootAlloc (BITBUFSIZE);
	if (pImage == 0)
	{
		DEBUGMSG(ZONE_ERROR,(L"Failed to alloc space for bitmap data.\r\n"));
		rc = -7;
		goto error_return;
	}
	PBYTE pVidImageBase, pVidBmpBase;
	int nVidWidth, nVidHeight, nVidColors;
	DWORD dwStride;
	int nOffset = 0, nOffset1, nBytesLeft = 0;
	WORD pxl;

	if (GetVidModeInfo(&nVidWidth, &nVidHeight, &nVidColors, 
		               &dwStride, &pVidImageBase, 0, 0, 0, 0, 0, 0, 0))
	{
		DEBUGMSG(ZONE_ERROR,(L"GetVidModeInfo Failed.\r\n"));
		rc = -7;
		goto error_return;
	}
	DEBUGMSG(ZONE_INFO, (L"Display Paramters:\r\n"));
    DEBUGMSG(ZONE_INFO, (L"   nVidWidth     %d\r\n", nVidWidth));
    DEBUGMSG(ZONE_INFO, (L"   nVidHeight    %d\r\n", nVidHeight));
    DEBUGMSG(ZONE_INFO, (L"   nVidColors    %d\r\n", nVidColors));
    DEBUGMSG(ZONE_INFO, (L"   dwStride      %x\r\n", dwStride));
    DEBUGMSG(ZONE_INFO, (L"   pVidImageBase %x\r\n", pVidImageBase));

	// Center bitmap on screen.
	int xPos, yPos;
	xPos = (nVidWidth - bmpHeader.biWidth) / 2;
	yPos = (nVidHeight - bmpHeader.biHeight) / 2;

	// Compute where in the Frame buffer the BMP will start.
	pVidBmpBase = pVidImageBase + (yPos * dwStride);
	switch (nVidColors)
	{
		case 8:
			pVidBmpBase += xPos;
			break;
		case 16:
			pVidBmpBase += (xPos * 2);
			break;
		case 24:
			pVidBmpBase += (xPos * 3);
			break;
		case 32:
			pVidBmpBase += (xPos * 4);
			break;
		default:
		    DEBUGMSG(ZONE_ERROR,(L"ERROR!  Unsupported bits per pixel in Splash draw routine bpp=%d\r\n", nVidColors));
			break;
	}

	// Process depending on BMP format.
	if (bmpHeader.biBitCount == 8)
	{
		// Got each row from bottom to top. (Bitmaps are inverted.)
		for (row = bmpHeader.biHeight-1; row >= 0; row--)
		{
			PWORD pVidRowStart = (PWORD)(pVidBmpBase + (dwStride * row));
			for (col = 0; col < bmpHeader.biWidth; col++)
			{
				// See if we need to read the next part of the file.
				if (nBytesLeft <= 0)
				{
					if (!FSReadFile ((PBYTE)pImage, BITBUFSIZE))
					{
						DEBUGMSG(ZONE_ERROR,(L"Failed to read bitmap bits.\r\n"));
						rc = -8;
						goto error_return;
					}
					nBytesLeft = BITBUFSIZE;
					nOffset = 0;
				}

				pxl = (WORD)((pColors[pImage[nOffset]] & 0x000000ff)         |
				 	   ((pColors[pImage[nOffset]] & 0x0000ff00) >> 3) |
					   ((pColors[pImage[nOffset]] & 0x00ff0000) >> 5));

				pVidRowStart[col] = pxl;
				nBytesLeft--;
				nOffset++;
			}
			// Correct for non-DWORD alignment at end of line.
			nOffset1 = nOffset;
			nOffset = ((nOffset + 4) & 0xfffffffc);
			nBytesLeft -= (nOffset - nOffset1);
		}
	}
	else if (bmpHeader.biBitCount == 24)
	{
		// Got each row from bottom to top. (Bitmaps are inverted.)
		for (row = bmpHeader.biHeight-1; row >= 0; row--)
		{
			PWORD pVidRowStart = (PWORD)(pVidBmpBase + (dwStride * row));
			WORD rgbClr[4];
			int k;
			for (col = 0; col < bmpHeader.biWidth; col++)
			{
				for (k = 0; k < 3; k++)
				{
					// See if we need to read the next part of the file.
					if (nBytesLeft <= 0)
					{
						if (!FSReadFile ((PBYTE)pImage, BITBUFSIZE))
						{
							DEBUGMSG(ZONE_ERROR,(L"Failed to read bitmap bits.\r\n"));
							rc = -8;
							goto error_return;
						}
						nBytesLeft = BITBUFSIZE;
						nOffset = 0;
					}
					rgbClr[k] = pImage[nOffset];
					nBytesLeft--;
					nOffset++;
				}

				pxl = (WORD)(((rgbClr[0] & 0xfc) >> 3)  |
				 	         ((rgbClr[1] & 0xfc) << 3)  |
					         ((rgbClr[2] & 0xf8) << 8));

				pVidRowStart[col] = pxl;
			}
			// Correct for non-DWORD alignment at end of line.
			nOffset1 = nOffset;
			nOffset = ((nOffset + 4) & 0xfffffffc);
			nBytesLeft -= (nOffset - nOffset1);
		}
	}

error_return:
	FSCloseFile ();
	if (pImage != 0) BootFree (pImage);
	if (pColors != 0) BootFree (pColors);
	return rc;
}
