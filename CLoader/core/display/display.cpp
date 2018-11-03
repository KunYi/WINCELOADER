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
//#include <bootDisplayBios.h>
#include <bootBios.h>
#include <bootMemory.h>
#include <bootLog.h>
#include <display.h>

#include <DisplayAPI.h>
//------------------------------------------------------------------------------


//static
//Display_t
//s_display;
//
//------------------------------------------------------------------------------


typedef struct {
	int nHeight;
	int nWidth;
	int nColors;
	WORD wMode;
	BYTE ucRPos;
	BYTE ucRSize;
	BYTE ucGPos;
	BYTE ucGSize;
	BYTE ucBPos;
	BYTE ucBSize;
	DWORD dwStride;
	PBYTE pFrameBuff;
}VIDINFO, *PVIDINFO;

VIDINFO vidCurrMode;

//------------------------------------------------------------------------------
//
//handle_t
//BootDisplayBiosInit(
//    )
//{
//    void *pContext = NULL;
//    uint32_t eax, ebx, ecx, edx, esi, edi;
//    VbeInfoBlock_t *pInfo = BootBiosBuffer();
//    VbeModeInfoBlock_t *pModeInfo = (VbeModeInfoBlock_t *)&pInfo[1];
//    uint16_t *pModes;
//    enum_t modes, idx;
//    bool_t vesa3;
//
//
//    // Create store context
//    memset(&s_display, 0, sizeof(Display_t));
//    s_display.pVTable = &s_displayVTable;
//
//    // Find if there is VBE support (VESA)
//    pInfo = BootBiosBuffer();
//    memset(pInfo, 0, sizeof(*pInfo));
//    pInfo->signature = '2EBV';
//    eax = 0x4F00;
//    edi = (uint16_t)((uint32_t)pInfo);
//    BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
//    if (((eax & 0xFFFF) != 0x004F) || (pInfo->version < 0x0200))
//        {
//        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosInit: "
//            L"BIOS doesn't support VESA 2.0+ Extension!\r\n"
//            ));
//        goto cleanUp;
//        }
//    vesa3 = pInfo->version >= 0x0300;
//
//    // Look how many video modes are usefull
//    pModes = BootBiosPtr2Void(pInfo->videoModesPtr);
//
//    idx = modes = 0;
//    while (pModes[idx] != 0xFFFF)
//        {
//        eax = 0x4F01;
//        ecx = pModes[idx++];
//        edi = (uint32_t)pModeInfo;
//        BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
//        if ((eax & 0xFFFF) != 0x004F) continue;
//        // We need at least 176 x 220
//        if (pModeInfo->xResolution < 176) continue;
//        if (pModeInfo->yResolution < 220) continue;
//        // Linear frame buffer must be avaiable
//        if ((pModeInfo->modeAttributes & (1 << 7)) == 0) continue;
//        // Do not support 15bpp configs
//        if (pModeInfo->bitsPerPixel == 15) continue;
//        // Support only RGB 5/6/5 mode for 16bpp configs
//        if (pModeInfo->bitsPerPixel == 16)
//            {
//            if (vesa3)
//                {
//                if (pModeInfo->linRedMaskSize != 5) continue;
//                if (pModeInfo->linGreenMaskSize != 6) continue;
//                if (pModeInfo->linBlueMaskSize != 5) continue;
//                }
//            else
//                {
//                if (pModeInfo->redMaskSize != 5) continue;
//                if (pModeInfo->greenMaskSize != 6) continue;
//                if (pModeInfo->blueMaskSize != 5) continue;
//                }
//            }
//        // One more video mode we can use
//        modes++;        
//        }
//    if (modes == 0)
//        {
//        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosInit: "
//            L"No usable VESA mode find!\r\n"
//            ));
//        goto cleanUp;
//        }
//
//    // Allocate structure to save display mode info
//    s_display.modes = modes;
//    s_display.aMode = BootAlloc(sizeof(DisplayMode_t) * modes);
//    if (s_display.aMode == NULL)
//        {
//        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosInit: "
//            L"Memory allocation failed!\r\n"
//            ));
//        goto cleanUp;
//        }
//
//    // Save mode information
//    pModes = BootBiosPtr2Void(pInfo->videoModesPtr);
//    idx = modes = 0;
//    while (pModes[idx] != 0xFFFF)
//        {
//        DisplayMode_t *pMode = &s_display.aMode[modes];
//        
//        eax = 0x4F01;
//        ecx = pModes[idx++];
//        edi = (uint32_t)pModeInfo;
//        BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
//        if ((eax & 0xFFFF) != 0x004F) continue;
//        // We need at least 176 x 220
//        if (pModeInfo->xResolution < 176) continue;
//        if (pModeInfo->yResolution < 220) continue;
//        // Linear frame buffer must be avaiable
//        if ((pModeInfo->modeAttributes & (1 << 7)) == 0) continue;
//        // Do not support 15bpp configs
//        if (pModeInfo->bitsPerPixel == 15) continue;
//        // Support only RGB 5/6/5 mode for 16bpp configs
//        if (pModeInfo->bitsPerPixel == 16)
//            {
//            if (vesa3)
//                {
//                if (pModeInfo->linRedMaskSize != 5) continue;
//                if (pModeInfo->linGreenMaskSize != 6) continue;
//                if (pModeInfo->linBlueMaskSize != 5) continue;
//                }
//            else
//                {
//                if (pModeInfo->redMaskSize != 5) continue;
//                if (pModeInfo->greenMaskSize != 6) continue;
//                if (pModeInfo->blueMaskSize != 5) continue;
//                }
//            }
//        // Save info
//        pMode->vbeMode = pModes[idx - 1];
//        pMode->width = pModeInfo->xResolution;
//        pMode->height = pModeInfo->yResolution;
//        pMode->bpp = pModeInfo->bitsPerPixel;
//        pMode->phFrame = pModeInfo->physBasePtr;
//        if (vesa3)
//            {
//            pMode->redSize = pModeInfo->linRedMaskSize;
//            pMode->redPos = pModeInfo->linRedFieldPosition;
//            pMode->greenSize = pModeInfo->linGreenMaskSize;
//            pMode->greenPos = pModeInfo->linGreenFieldPosition;
//            pMode->blueSize = pModeInfo->linBlueMaskSize;
//            pMode->bluePos = pModeInfo->linBlueFieldPosition;
//            pMode->stride = pModeInfo->linBytesPerScanLine;
//            }
//        else
//            {
//            pMode->redSize = pModeInfo->redMaskSize;
//            pMode->redPos = pModeInfo->redFieldPosition;
//            pMode->greenSize = pModeInfo->greenMaskSize;
//            pMode->greenPos = pModeInfo->greenFieldPosition;
//            pMode->blueSize = pModeInfo->blueMaskSize;
//            pMode->bluePos = pModeInfo->blueFieldPosition;
//            pMode->stride = pModeInfo->bytesPerScanLine;
//            }
//
//        // Move to next mode
//        modes++;        
//        }
//    
//    s_display.mode = (enum_t)-1;
//    
//    // Done
//    pContext = &s_display;
//
//cleanUp:
//    if (pContext == NULL) BootDisplayBiosDeinit(&s_display);
//    return pContext;
//}
//
////------------------------------------------------------------------------------
//
//bool_t
//BootDisplayBiosDeinit(
//    void *pContext
//    )
//{
//    bool_t rc = false;
//    Display_t *pDisplay = pContext;
//
//
//    // Check display handle
//    if (pDisplay != &s_display)
//        {
//        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosDeinit: "
//            L"Invalid display handle!\r\n"
//            ));
//        goto cleanUp;
//        }
//
//    // Release allocated mode table
//    BootFree(pDisplay->aMode);
//    
//    // Delete context (oops, clear it only)
//    memset(pDisplay, 0, sizeof(Display_t));
//
//    // Done
//    rc = true;
//
//cleanUp:
//    return rc;
//}
//
//------------------------------------------------------------------------------


int FillRect16(int x, int y, int cx, int cy, DWORD Color);

// Save the best mode we can find if the requested one isn't supported.
VbeModeInfoBlock_t BestModeInfo;

int DumpDisplayModes (void)
{
	WORD wMode;
	int a, b, c;
	SetDisplayMode (0xffff, 0xffff, 32, &wMode, &a, &b, &c, TRUE);
	return 0;
}

//------------------------------------------------------------------------------
//
//
int SetDisplayMode (int nWidth, int nHeight, int nColors, WORD *pwMode, 
					int *pBestWidth, int *pBestHeight, int *pBestColor, BOOL fDump)
{
    uint32_t eax, ebx, ecx, edx, esi, edi;
    VbeInfoBlock_t *pInfo;
    VbeModeInfoBlock_t *pModeInfo;
    uint16_t *pModes;
    enum_t modes, idx;
    bool_t vesa3;
	*pBestWidth = 0;
	*pBestHeight = 0;
	*pBestColor = 0; 

	int rc = -1;
	BYTE rs = 0, gs = 0, bs = 0, rp = 0, gp = 0, bp = 0;
	WORD wStride = 1;
	BOOL fFound = FALSE;

    // Find if there is VBE support (VESA)
    pInfo = (VbeInfoBlock_t *)BootBiosBuffer();
    pModeInfo = (VbeModeInfoBlock_t *)&pInfo[1];

	memset(pInfo, 0, sizeof(*pInfo));
    pInfo->signature = '2EBV';
    eax = 0x4F00;
    edi = (uint16_t)((uint32_t)pInfo);
    BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if (((eax & 0xFFFF) != 0x004F) || (pInfo->version < 0x0200))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosInit: "
            L"BIOS doesn't support VESA 2.0+ Extension!\r\n"
            ));
        goto cleanUp;
        }
    vesa3 = pInfo->version >= 0x0300;

    // Look how many video modes are usefull
    pModes = (uint16_t *)BootBiosPtr2Void(pInfo->videoModesPtr);

    idx = modes = 0;
    while (pModes[idx] != 0xFFFF)
        {
        eax = 0x4F01;
        ecx = pModes[idx++];
        edi = (uint32_t)pModeInfo;
        BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
        if ((eax & 0xFFFF) != 0x004F) continue;

        // Linear frame buffer must be avaiable
        if ((pModeInfo->modeAttributes & (1 << 7)) == 0) continue;

		if (vesa3)
        {
                rs = pModeInfo->linRedMaskSize;
                rp = pModeInfo->linRedFieldPosition;
                gs = pModeInfo->linGreenMaskSize;
				gp = pModeInfo->linGreenFieldPosition; 
                bs = pModeInfo->linBlueMaskSize;
				bp = pModeInfo->linBlueFieldPosition;
				wStride = pModeInfo->linBytesPerScanLine;
        }
        else
        {
                rs = pModeInfo->redMaskSize;
				rp = pModeInfo->redFieldPosition;
                gs = pModeInfo->greenMaskSize;
				gp = pModeInfo->greenFieldPosition;
                bs = pModeInfo->blueMaskSize;
				bp = pModeInfo->blueFieldPosition;
				wStride = pModeInfo->bytesPerScanLine;
        }
		if (fDump)
		{

			// Using BootLog so that the info will always print.
			BootLog (L"   Mode %x %dx%d  %d bpp.   color format %d:%d:%d\r\n", pModes[idx-1],
				pModeInfo->xResolution, pModeInfo->yResolution, pModeInfo->bitsPerPixel,
				rs, gs, bs);
#ifdef WCELDRS
			ScreenLog (L"   Mode %x %dx%d  %d bpp.   color format %d:%d:%d\r\n", pModes[idx-1],
				pModeInfo->xResolution, pModeInfo->yResolution, pModeInfo->bitsPerPixel,
				rs, gs, bs);
#endif
		}
        // Do not support 15bpp configs
        if (pModeInfo->bitsPerPixel == 15) continue;
        // Support only RGB 5/6/5 mode for 16bpp configs
        if (pModeInfo->bitsPerPixel == 16)
        {
            if (vesa3)
            {
                if (pModeInfo->linRedMaskSize != 5) continue;
                if (pModeInfo->linGreenMaskSize != 6) continue;
                if (pModeInfo->linBlueMaskSize != 5) continue;
            }
            else
            {
                if (pModeInfo->redMaskSize != 5) continue;
                if (pModeInfo->greenMaskSize != 6) continue;
                if (pModeInfo->blueMaskSize != 5) continue;
            }
        }
        // One more video mode we can use
        modes++;        

		// See if this mode matches the requested resolution.
		if ((nWidth == pModeInfo->xResolution) &&
			(nHeight == pModeInfo->yResolution) &&
			(nColors == pModeInfo->bitsPerPixel))
		{
			fFound = TRUE;
			break;
		}
		else
		{
			// See if this mode better than the last
			if (*pBestWidth < pModeInfo->xResolution)
			{
				*pBestWidth = pModeInfo->xResolution;
				*pBestHeight = pModeInfo->yResolution;
				*pBestColor = pModeInfo->bitsPerPixel; 
			}
			else if (*pBestWidth == pModeInfo->xResolution)
			{
				if ((pModeInfo->bitsPerPixel < 32) &&
					 (*pBestColor < pModeInfo->bitsPerPixel))
				{
					*pBestWidth = pModeInfo->xResolution;
					*pBestHeight = pModeInfo->yResolution;
					*pBestColor = pModeInfo->bitsPerPixel; 
				}
				else if (*pBestColor < 16)
				{
					*pBestWidth = pModeInfo->xResolution;
					*pBestHeight = pModeInfo->yResolution;
					*pBestColor = pModeInfo->bitsPerPixel; 
				}
			}
		}
	}
	if (fDump)
		BootLog (L"%d valid modes found.\r\n", modes);

	if (fFound)
	{
		// Save video information.
		vidCurrMode.nHeight = pModeInfo->yResolution;
		vidCurrMode.nWidth = pModeInfo->xResolution;
		vidCurrMode.nColors = pModeInfo->bitsPerPixel;
		vidCurrMode.dwStride = wStride;
		vidCurrMode.pFrameBuff = (PBYTE)pModeInfo->physBasePtr;
		vidCurrMode.wMode = pModes[idx-1];
		vidCurrMode.ucRPos = rp;
		vidCurrMode.ucRSize = rs;
		vidCurrMode.ucGPos = gp;
		vidCurrMode.ucGSize = gs;
		vidCurrMode.ucBPos = bp;
		vidCurrMode.ucBSize = bs;

		// Return the mode
		*pwMode = pModes[idx-1];

		// Set mode.
		eax = 0x4F02;
		ebx = pModes[idx-1] | (1 << 14);
		edi = 0;
		BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
		if ((eax & 0xFFFF) != 0x004F) 
		{
			DEBUGMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosInit: "
				L"Failed to set VESA mode %x!  ax %x\r\n", pModes[idx-1], eax));
		}
		rc = 0;
	}
	else
	{
		*pwMode = 0xffff;
		rc = -2;
	}
    if (modes == 0)
	{
        DEBUGMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosInit: "
            L"No usable VESA mode find!\r\n"
            ));
		*pwMode = 0xffff;
		rc = -3;
        goto cleanUp;
     }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//
int GetVidModeInfo(int *pnWidth, int *pnHeight, int *pnColors, DWORD *pdwStride,
				   PBYTE *ppFrameBuffer, PBYTE pucRPos, PBYTE pucRSiz, PBYTE pucGPos, 
				   PBYTE pucGSiz, PBYTE pucBPos, PBYTE pucBSiz, PWORD pwMode)
{
	int rc = 0;

	if ((pnWidth == 0) || (pnHeight == 0) || (pdwStride == 0) || 
		(pnColors == 0) || (ppFrameBuffer == 0))
		return -1;

	*pnHeight = vidCurrMode.nHeight;
	*pnWidth = vidCurrMode.nWidth;
	*pnColors = vidCurrMode.nColors;
	*pdwStride = vidCurrMode.dwStride;
	*ppFrameBuffer = vidCurrMode.pFrameBuff;
	if (pwMode) *pwMode  = vidCurrMode.wMode;
	if (pucRPos) *pucRPos = vidCurrMode.ucRPos; 
	if (pucRSiz) *pucRSiz = vidCurrMode.ucRSize;
	if (pucGPos) *pucGPos = vidCurrMode.ucGPos;
	if (pucGSiz) *pucGSiz = vidCurrMode.ucGSize;
	if (pucBPos) *pucBPos = vidCurrMode.ucBPos; 
	if (pucBSiz) *pucBSiz = vidCurrMode.ucBSize;

	if ((*pnWidth == 0) || (*pnHeight == 0) || (*pnColors == 0) || (*ppFrameBuffer == 0))
		return -2;

	return rc;
}

//------------------------------------------------------------------------------
//
//
#if 0
static bool_t DisplayModeQuery(
    Display_t *pDisplay,
    enum_t *pMode,
    uint32_t *pWidth,
    uint32_t *pHeight,
    uint32_t *pBpp,
    uint32_t *pVesaMode,
    uint32_t *pPhFrame,
    uint32_t *pStride,
    uint32_t *pRedSize,
    uint32_t *pRedPos,
    uint32_t *pGreenSize,
    uint32_t *pGreenPos,
    uint32_t *pBlueSize,
    uint32_t *pBluePos
    )
{
    bool_t rc = false;
    enum_t mode = (enum_t)-1;
    DisplayMode_t *pModeInfo;
    
    if (*pMode == -1)
        {
        if ((*pWidth != 0) || (*pHeight != 0))
            {
            enum_t bestMode = (enum_t)-1;
            uint32_t bestDelta = (uint32_t)-1;
        
            for (mode = 0; mode < pDisplay->modes; mode++)
                {
                uint32_t delta;
                pModeInfo = &pDisplay->aMode[mode];

                // Not best mode if it is less than 16 bpp
                if (pModeInfo->bpp < 16) continue;
                if (*pWidth > pModeInfo->width) continue;
                if (*pHeight > pModeInfo->height) continue;
                delta  = pModeInfo->width - *pWidth;
                delta += pModeInfo->height - *pHeight;
                if (delta < bestDelta)
                    {
                    bestDelta = delta;
                    bestMode = mode;
                    }
                if (bestDelta == 0) break;
                }

            // If we didn't find reasonable mode, fail call...
            if (bestMode == -1) goto cleanUp;

            // This is our mode
            mode = bestMode;
            
            }
        else 
            {
            uint32_t eax, ebx, ecx, edx, esi, edi;

            // Get actual VESA mode            
            eax = 0x4F03;
            BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
            if ((eax & 0xFFFF) != 0x004F) goto cleanUp;
            ebx &= 0x3FFF;

            // Look if it is one from supported        
            for (mode = 0; mode < pDisplay->modes; mode++)
                {
                pModeInfo = &pDisplay->aMode[mode];
                if (pModeInfo->vbeMode == ebx) break;
                }

            // Fail if we don't support it...
            if (mode >= pDisplay->modes) goto cleanUp;
            }
        }
    else if (*pMode < pDisplay->modes)
        {
        mode = *pMode;
        }
    else
        {
        goto cleanUp;
        }

    // Results
    pModeInfo = &pDisplay->aMode[mode];

    *pMode = mode;
    *pVesaMode = pModeInfo->vbeMode;
    *pWidth = pModeInfo->width;
    *pHeight = pModeInfo->height;
    *pBpp = pModeInfo->bpp;
    *pPhFrame = pModeInfo->phFrame;
    *pStride = pModeInfo->stride;
    *pRedSize = pModeInfo->redSize;
    *pRedPos = pModeInfo->redPos;
    *pGreenSize = pModeInfo->greenSize;
    *pGreenPos = pModeInfo->greenPos;
    *pBlueSize = pModeInfo->blueSize;
    *pBluePos = pModeInfo->bluePos;
        
    // Done
    rc = true;
    
cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------
//
//
static bool_t DisplayModeSet(
    Display_t *pDisplay,
    enum_t mode
    )
{
    bool_t rc = false;
    uint32_t eax, ebx, ecx, edx, esi, edi;

    BootBiosBuffer();
    if (mode >= pDisplay->modes) goto cleanUp;

    eax = 0x4F02;
    ebx = pDisplay->aMode[mode].vbeMode | (1 <<   14);
    edi = 0;
    BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if ((eax & 0xFFFF) != 0x004F) goto cleanUp;
    
    // Set actual mode
    pDisplay->mode = mode;

    // Map frame buffer
    pDisplay->pFrame = (void *)BootPAtoUA(pDisplay->aMode[mode].phFrame);

    // Done
    rc = true;

cleanUp:    
    return rc;
}
#endif
//------------------------------------------------------------------------------
//
//
int FillRect16(int x, int y, int cx, int cy, DWORD dwColor)
{
	int rc = 0;

	int i, j;
	PBYTE pPxl = (PBYTE)vidCurrMode.pFrameBuff;
	PWORD pRow;

	WORD pxl;
	pxl = (WORD)( (dwColor & 0x0000001f)       |
	             ((dwColor & 0x00003f00) >> 3) |
	             ((dwColor & 0x001f0000) >> 5));

	for (i = 0; i < cx; i++)
	{
		pRow = (PWORD)(pPxl + ((i+y) * vidCurrMode.dwStride) + (x * 2));
		for (j = 0; j < cy; j++)
		{
			*((WORD *)pRow)++ = pxl;
		}
	}

    return rc;
}

//
////------------------------------------------------------------------------------
////
////
//static bool_t DisplayFillRect16(
//    Display_t* pDisplay,
//    RECT *pRect,
//    uint32_t color
//    )
//{
//    bool_t rc = false;
//    volatile uint16_t *pFrame = (uint16_t *)pDisplay->pFrame;
//    DisplayMode_t *pMode;
//    long xt, yt;
//
//    if (pDisplay->mode >= pDisplay->modes) goto cleanUp;
//    pMode = &pDisplay->aMode[pDisplay->mode];
//
//    if (pRect->right < pRect->left) goto cleanUp;
//    if (pRect->top > pRect->bottom) goto cleanUp;
//
//    for (yt = pRect->top; yt < pRect->bottom; yt++)
//        {
//        for (xt = pRect->left; xt < pRect->right; xt++)
//            {
//            pFrame[yt * (pMode->stride >> 1) + xt] = (uint16_t)color;
//            }
//        }
//
//    // Done
//    rc = true;
//
//cleanUp:    
//    return rc;
//}
//
//------------------------------------------------------------------------------
//
//
#if 0
static bool_t DisplayBltRect16(
    Display_t* pDisplay,
    RECT* pRect,
    void *pBuffer
    )
{
    bool_t rc = false;
    volatile uint16_t *pFrame = (uint16_t *)pDisplay->pFrame;
    DisplayMode_t *pMode;
    uint16_t *pData = (uint16_t*)pBuffer;
    long stride, pos, xt, yt;

    if (pDisplay->mode >= pDisplay->modes) goto cleanUp;
    pMode = &pDisplay->aMode[pDisplay->mode];

    if (pRect->right < pRect->left) goto cleanUp;
    if (pRect->top > pRect->bottom) goto cleanUp;
    
    // Convert stride from bytes to words
    stride = pRect->right - pRect->left;
    pos = (pRect->bottom - pRect->top) * stride;
    for (yt = pRect->top; yt < pRect->bottom; yt++)
        {
        for (xt = pRect->right - 1; xt >= pRect->left; xt--)
            {
            pFrame[yt * (pMode->stride >> 1) + xt] = pData[--pos];
            }
        }

    // Done
    rc = true;

cleanUp:
    return rc;
}
#endif
//------------------------------------------------------------------------------

