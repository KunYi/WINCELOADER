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

#ifndef _INI_H
#define _INI_H

// Maximum file length
#define MAX_FILE_NAME_LEN           12              // TODO: reslove FAT32 question

// Maximum IP address length
#define MAX_IP_ADDRESS_LEN          15

// INI defaults
#define DEFAULT_BIN_FILE_NAME       "eboot.bin"     // Default BIN file name (to be loaded)
#define DEFAULT_DEBUG_ZONE          0x00000007      // DBGZONE_ERROR, DBGZONE_WARNING, DBGZONE_INFO
#define DEFAULT_DEVICE_NAME_ROOT    "CEPC"

#define DEFAULT_COMPORT             1
#define DEFAULT_BAUDRATE            BD38400

#define DEFAULT_DISPLAY_WIDTH       640
#define DEFAULT_DISPLAY_HEIGHT      480
#define DEFAULT_DISPLAY_DEPTH       8

#define DEFAULT_DELAY               0

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
#define INIPARAM_BIN_FILE           "BinFile"
#define INIPARAM_BAK_BIN_FILE       "BakBinFile"
#define INIPARAM_DEVICE_NAME_ROOT   "DeviceNameRoot"
#define INIPARAM_DEBUG_ZONE         "DebugZone"
#define INIPARAM_VIDEO              "Video"
#define INIPARAM_DISPLAY_WIDTH      "DisplayWidth"
#define INIPARAM_DISPLAY_HEIGHT     "DisplayHeight"
#define INIPARAM_PHYSICAL_WIDTH     "PhysicalWidth"
#define INIPARAM_PHYSICAL_HEIGHT    "PhysicalHeight"
#define INIPARAM_DISPLAY_DEPTH      "DisplayDepth"
#define INIPARAM_FLASH_BACKUP       "FlashBackup"
#define INIPARAM_ETH_IRQ            "EthIRQ"
#define INIPARAM_ETH_IO             "EthIO"
#define INIPARAM_DBG_IP             "DbgIP"
#define INIPARAM_COM_PORT           "COMPort"
#define INIPARAM_BAUDRATE           "Baudrate"
#define INIPARAM_VIDEO              "Video"
#define INIPARAM_DELAY              "Delay"

// Stores non-BOOT_ARGS-related INI data
typedef struct _INI_PARAMS
{
    UCHAR szUserSelectableImage[MAX_FILE_NAME_LEN + 1];
    UCHAR szImage[MAX_FILE_NAME_LEN + 1];
    UCHAR szBackupImage[MAX_FILE_NAME_LEN + 1];
    LONG  Delay;
} INI_PARAMS;

extern INI_PARAMS IniParams;

// Public function prototypes
BLSTATUS InitBootArgs(BOOT_ARGS * pBootArgs);
BLSTATUS SetUserSelectableBinFile(LONG delay);

#endif
