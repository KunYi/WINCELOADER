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

#ifndef _INI_H
#define _INI_H

// Maximum file length
#define MAX_FILE_NAME_LEN           12              // TODO: reslove FAT32 question

// Maximum IP address length
#define MAX_IP_ADDRESS_LEN          15              


// Stores non-BOOT_ARGS-related INI data
typedef struct _INI_PARAMS
{
    UCHAR szUserSelectableImage[MAX_FILE_NAME_LEN + 1];
    UCHAR szImage[MAX_FILE_NAME_LEN + 1];
    UCHAR szBackupImage[MAX_FILE_NAME_LEN + 1];
    UCHAR szSplashFile[MAX_FILE_NAME_LEN + 1];
    LONG  Delay;
} INI_PARAMS;

extern INI_PARAMS IniParams;

int BootArgsLoadIniFileData(BootLoader_t *pLoader);

// Public function prototypes
//int InitBootArgs(BOOT_ARGS * pBootArgs);
//BLSTATUS SetUserSelectableBinFile(LONG delay);

#endif
