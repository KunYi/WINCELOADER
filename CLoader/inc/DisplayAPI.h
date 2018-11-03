//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//------------------------------------------------------------------------------

//
// BIOS loader debug support
//

#ifndef _DISPLAYAPI_H
#define _DISPLAYAPI_H

#ifdef __cplusplus
extern "C" {
#endif

int InitializeDisplay (void);

//int DumpDisplayModes(void);

//static bool_t DisplayModeSet(
//    Display_t *pDisplay,
//    enum_t mode
//    );
//static bool_t DisplayFillRect16(
//    Display_t* pDisplay,
//    RECT *pRect,
//    uint32_t color
//    );
//
//static bool_t DisplayBltRect16(
//    Display_t* pDisplay,
//    RECT* pRect,
//    void *pBuffer
//    );

int DumpDisplayModes (void);
//int SetDisplayMode (int nWidth, int nHeight, int nColors, WORD *pwMode, BOOL fDump);
int SetDisplayMode (int nWidth, int nHeight, int nColors, WORD *pwMode, 
					int *pBestWidth, int *pBestHeight, int *pBestColor, BOOL fDump);

int DrawSplashImage(char *pszBmpFileName);

int GetVidModeInfo(int *pnWidth, int *pnHeight, int *pnColors, DWORD *pdwStride,
				   PBYTE *ppFrameBuffer, PBYTE pucRPos, PBYTE pucRSiz, PBYTE pucGPos, 
				   PBYTE pucGSiz, PBYTE pucBPos, PBYTE pucBSiz, PWORD pwMode);
 
#ifdef __cplusplus
}
#endif



#endif // _DISPLAYAPI_H
