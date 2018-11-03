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
// BIOS loader constants and support macros
//
// Revision History
//
//   Major  Minor       Comments
//   -----  -----       ------------------------------------------------
//     1      0         Initial revision.
//     1      1         Relocated loader to 0000:1000 and disabled PIC.
//     2      0         INI file support
//                      Splash screen and progress bar support
//                      More flexible debug message mechanisms
//     2      1         decompression support added
//                      User-selectable image
//                      Minor progress bar bugfix
//                      Prefix-related fixes in the assembler code
//     2      2         Cleaned up compression code and memory map.
//

#ifndef _BLDR_H_
#define _BLDR_H_

#define BLDR_SPLASH_MESSAGE        "\n\rMicrosoft Windows CE BIOS Bootloader Version"
#define BLDR_SPLASH_MESSAGE_LENGTH sizeof(BLDR_SPLASH_MESSAGE)
#define BLDR_VERSION_MAJOR         "2"
#define BLDR_VERSION_MINOR         "2"

//
// Memory locations
//
// When adding a new memory location care must be taken not to overlap the
// already existing ones (watch start address and length of the buffers).
//

#define VESA_GENINFO_START                  ((VESA_GENERAL_INFO *)0x8000)

#define BOOT_ARG_PTR_LOCATION_NP            0x001FFFFC
#define BOOT_ARG_LOCATION_NP                0x001FFF00

// BMP file raw data memory buffer
#define BMP_RAW_DATA_START                  0x00020000
#define BMP_RAW_DATA_LENGTH                 0x00010000
#define BMP_RAW_DATA_BUFFER                 ((PCHAR)BMP_RAW_DATA_START)

// Decompressor original data buffer
#define BINCOMPRESS_ORIGINAL_DATA_START     0x00030000
#define BINCOMPRESS_ORIGINAL_DATA_LENGTH    0x00010000
#define BINCOMPRESS_ORIGINAL_DATA_BUFFER    ((PCHAR)BINCOMPRESS_ORIGINAL_DATA_START)

// Decompressor encoded data buffer
#define BINCOMPRESS_ENCODED_DATA_START      0x00040000
#define BINCOMPRESS_ENCODED_DATA_LENGTH     0x00010000
#define BINCOMPRESS_ENCODED_DATA_BUFFER     ((PCHAR)BINCOMPRESS_ENCODED_DATA_START)

// INI file raw data memory buffer
#define INI_RAW_DATA_START                  0x00050000
#define INI_RAW_DATA_LENGTH                 0x00001000
#define INI_RAW_DATA_BUFFER                 ((PCHAR)INI_RAW_DATA_START)

// Compression algorithms work buffers
// As only one compression method is selected at compile time
// the buffers may overlap

//
// work memory
//
// Work buffer. As we don't know how much memory cecompress will request -
// we need to put the buffer at the very end. In this way cecompress can grab as
// much as it wants.  Problems will arise when we need another buffer of
// dynamic size...
#define BINCOMPRESS_WORK_AREA_START         0x00051000
#define BINCOMPRESS_WORK_AREA_BUFFER        ((PCHAR)BINCOMPRESS_WORK_AREA_START)

//
// Constants
//
//

// BIOS loader status codes
#define BLSTATUS_OK                     0
#define BLSTATUS_ERROR                  1
#define BLSTATUS_NO_INI_FILE            2
#define BLSTATUS_NO_VIDEO_MODE_FOUND    3
#define BLSTATUS_NO_USER_SELECTION      4

// Number of times to retry an IO operation
#define FLOPPY_IO_RETRIES           4
#define FIXEDDISK_IO_RETRIES        0

// Default INI file name (used for user options)
#define OPTIONS_FILE_NAME           "boot.ini"

// Helper macros
#define TO_UPPER(a)     ((a >= 0x61) && (a <=0x7A) ? a - 0x20 : a)
#define MIN(a, b)       (a < b ? a : b)

// Typedefs
typedef void (*PFN_LAUNCH)();
typedef ULONG BLSTATUS;

#endif // _BLDR_H_
