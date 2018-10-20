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

//
// BIOS loader debug support
//

#ifndef _DBG_H
#define _DBG_H

// Which function to use to output debug information
#define SERPRINT      KITLOutputDebugString

// Debug mask global variable
extern DWORD dwZoneMask;

// Helps define debug masks
#define DBGZONE(n)    (dwZoneMask&(0x00000001<<(n)))

// Default debug zones
#define DBGZONE_ERROR      DBGZONE(0)
#define DBGZONE_WARNING    DBGZONE(1)
#define DBGZONE_INFO       DBGZONE(2)

// In case someone combined SHIP and DEBUG options
#ifdef DEBUG
#ifdef SHIP_BUILD
#undef SHIP_BUILD
#pragma message (__FILE__ ":WARNING: SHIP_BUILD turned off since DEBUG defined")
#endif
#endif

// Define SHIP_BUILD to turn off output at all (except for RTLMSG)
#ifdef SHIP_BUILD

#define RTLMSG(cond,printf_exp)   \
   ((void)((cond)?(SERPRINT printf_exp),1:0))

#define DBGMSG(cond,printf_exp) ((void)0)

#define INFOMSG(code, printf_exp) ((void)0)
#define WARNMSG(code, printf_exp) ((void)0)
#define ERRMSG(code, printf_exp) ((void)0)
#endif

// Define DEBUG to turn on the debug output
#ifdef DEBUG

#define DBGMSG(cond,printf_exp)   \
   ((void)((cond)?(SERPRINT printf_exp),1:0))
#define RTLMSG(cond,printf_exp)   \
   ((void)((cond)?(SERPRINT printf_exp),1:0))

//
// Define FULLMESSAGES to include full messages
// in the binary file.
//
// If FULLMESSAGES is not defined only message
// codes are included in the binary file.
//
#ifdef FULLMESSAGES

#define INFOMSG(code, printf_exp) \
   ((void)((DBGZONE_INFO)?(SERPRINT("INFO: "), SERPRINT printf_exp),1:0))
#define WARNMSG(code, printf_exp) \
   ((void)((DBGZONE_WARNING)?(SERPRINT("WARNING: "), SERPRINT printf_exp),1:0))
#define ERRMSG(code, printf_exp) \
   ((void)((DBGZONE_ERROR)?(SERPRINT("ERROR: "), SERPRINT printf_exp),1:0))

#else   // FULLMESSAGES

#define INFOMSG(code, printf_exp) \
   ((void)((DBGZONE_INFO)?(SERPRINT("INFO: code=0x%x.\n", code)),1:0))
#define WARNMSG(code, printf_exp) \
   ((void)((DBGZONE_WARNING)?(SERPRINT("WARNING: code=0x%x.\n", code)),1:0))
#define ERRMSG(code, printf_exp) \
   ((void)((DBGZONE_ERROR)?(SERPRINT("ERROR: code=0x%x.\n", code)),1:0))

#endif // FULLMESSAGES
#endif // DEBUG

//
// Debug message codes
//
#define MSG_UNDEFINED                   0xFFFFFFFF

#define MSG_UNKNOWN_FILE_SYSTEM         0x00000001  // Unknown file system
#define MSG_CANNOT_INIT_READ_BUFFER     0x00000002  // Failed to initialize read buffer
#define MSG_CANNOT_READ_ROOT_DIR        0x00000003  // Couldn't read root directory sector
#define MSG_FILE_FOUND                  0x00000004  // File found
#define MSG_CANNOT_RESET_DISK_CTRL      0x00000005  // Unable to reset disk controller
#define MSG_CANNOT_READ_SECTOR          0x00000006  // bits 31-24 contain the BIOS error code
#define MSG_FILE_NOT_FOUND              0x00000007  // File not found
#define MSG_CANNOT_LOAD_BIN             0x00000008  // Failed to load BIN image
#define MSG_CANNOT_READ_IMG_HEADER      0x00000009  // Cannot read image header
#define MSG_CANNOT_READ_REC_HEADER      0x0000000A  // Couldn't read record header
#define MSG_CANNOT_READ_REC_INFO        0x0000000B  // Cannot read record information
#define MSG_EOF_CLTR_IN_FAT_CHAIN       0x0000000C  // Reached EOF cluster in FAT file chain
#define MSG_CANNOT_READ_DATA            0x0000000D  // Failed to read data from storage
#define MSG_READ_BUFFER_TOO_SMALL       0x0000000E  // Read buffer should be at least a cluster size
#define MSG_RESERVED_CLTR_IN_CHAIN      0x0000000F  // Found reserved cluster in FAT file chain
#define MSG_BAD_CLTR_IN_CHAIN           0x00000010  // Found bad cluster in FAT file chain
#define MSG_CANNOT_GET_NEXT_CLTR        0x00000011  // Cannot find contiguous cluster grouping
#define MSG_TOO_LONG_INI_FILE           0x00000012  // INI file longer than expected
#define MSG_CANNOT_OPEN_INI_FILE        0x00000013  // Cannot open INI file
#define MSG_PHYS_WIDTH_CORRECTED        0x00000014  // Setting DisplayWidth = PhysicalWidth; PhysicalWidth = 0
#define MSG_PHYS_HEIGHT_CORRECTED       0x00000015  // Setting DisplayHeight = PhysicalHeight; PhysicalHeight = 0
#define MSG_CANNOT_COPY_DATA_SECTIONS   0x00000016  // Cannot copy initialized data section(s). Halting.
#define MSG_CANNOT_INIT_FAT             0x00000017  // FAT init failed. Halting.
#define MSG_FALLBACK_VIDEO_MODE         0x00000018  // Setting fallback video mode (320x200x256)
#define MSG_CANNOT_FIND_SPLASH_MODE     0x00000019  // Cannot find splash mode
#define MSG_CANNOT_SET_SPLASH_MODE      0x0000001a  // Cannot set splash mode
#define MSG_TOO_LONG_BMP_FILE           0x0000001b  // BMP longer than expected
#define MSG_CANNOT_LOAD_SPLASH_IMAGE    0x0000001c  // Cannot load splash image
#define MSG_CANNOT_FIND_SPLASH_IMAGE    0x0000001d  // Cannot find splash image
#define MSG_CANNOT_SET_PALETTE          0x0000001e  // Cannot set palette
#define MSG_NO_VESA                     0x0000001f  // No VESA-compliant video adapter
#define MSG_CANNOT_GET_MODE_INFO        0x00000020  // Cannot get mode info
#define MSG_CANNOT_FIND_DEFAULT_VIDEO   0x00000021  // Failed to find default video mode
#define MSG_TRYING_DEFAULT_VIDEO        0x00000022  // Requested video mode not found
#define MSG_CANNOT_SET_VIDEO_MODE       0x00000023  // Cannot set video mode
#define MSG_BAD_CLUSTER_NUMBER          0x00000024  // Bad cluster number
#define MSG_NO_FAT32_SUPPORT            0x00000025  // No FAT32 support yet
#define MSG_FAT_NOT_INITIALIZED         0x00000026  // FAT must be initialized before reading files
#define MSG_INIWARN_BIN_FILE            0x00000027  // Cannot find/parse parameter: BinFile
#define MSG_INIWARN_BAK_BIN_FILE        0x00000028  // Cannot find/parse parameter: BakBinFile
#define MSG_INIWARN_DEVICE_NAME_ROOT    0x00000029  // Cannot find/parse parameter: DeviceNameRoot
#define MSG_INIWARN_DEBUG_ZONE          0x0000002a  // Cannot find/parse parameter: DebugZone
#define MSG_INIWARN_VIDEO               0x0000002b  // Cannot find/parse parameter: Video
#define MSG_INIWARN_DISPLAY_WIDTH       0x0000002c  // Cannot find/parse parameter: DisplayWidth
#define MSG_INIWARN_DISPLAY_HEIGHT      0x0000002d  // Cannot find/parse parameter: DisplayHeight
#define MSG_INIWARN_PHYSICAL_WIDTH      0x0000002e  // Cannot find/parse parameter: PhysicalWidth
#define MSG_INIWARN_PHYSICAL_HEIGHT     0x0000002f  // Cannot find/parse parameter: PhysicalHeight
#define MSG_INIWARN_DISPLAY_DEPTH       0x00000030  // Cannot find/parse parameter: DisplayDepth
#define MSG_INIWARN_FLASH_BACKUP        0x00000031  // Cannot find/parse parameter: FlashBackup
#define MSG_INIWARN_ETH_IRQ             0x00000032  // Cannot find/parse parameter: EthIRQ
#define MSG_INIWARN_ETH_IO              0x00000033  // Cannot find/parse parameter: EthIO
#define MSG_INIWARN_DBG_IP              0x00000034  // Cannot find/parse parameter: DbgIP
#define MSG_INIWARN_COM_PORT            0x00000035  // Cannot find/parse parameter: COMPort
#define MSG_INIWARN_BAUDRATE            0x00000036  // Cannot find/parse parameter: Baudrate
#define MSG_TRYING_BAK_IMAGE            0x00000037  // Trying backup image file
#define MSG_DECOMPRESSION_ERROR         0x00000038  // Decompression error
#define MSG_DECOMPRESS_INIT_ERROR       0x00000039  // Cannot init decompressor
#define MSG_NO_BIN_FILE_ENTRY           0x0000003a  // BinFileX entry not found
#define MSG_INIWARN_DELAY               0x0000003b  // Cannot parse parameter: Delay
#define MSG_INVALID_COMPORT             0x0000003c  // COM port read from INI file is not in supported range [0,4]

#endif // _DBG_H
