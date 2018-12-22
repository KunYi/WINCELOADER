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

#ifndef _SPLASH_H
#define _SPLASH_H

// NOTE: A 24-bit screen will be requested
#define SPLASH_SCREEN_WIDTH                 640
#define SPLASH_SCREEN_HEIGHT                480

#define PROGRESS_BAR_WIDTH                  100
#define PROGRESS_BAR_HEIGHT                 10
#define PROGRESS_BAR_TOP_MARGIN             5
#define PROGRESS_BAR_COLOUR                 0x00383838      // 6bits per primary
#define PROGRESS_BAR_UPDATE_THRESHOLD       25

#define SPLASH_IMAGE_FILE_NAME              "splash.bmp"

// Public function prototypes
int InitSplashScreen();
void SetProgressValue(int progress);

#endif
