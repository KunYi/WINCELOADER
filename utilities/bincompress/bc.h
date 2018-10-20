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
// Defines constants used by bincompress. Include this file
// in the BIOS loader decompression module to keep the constants in sync.
//

#ifndef _BC_H
#define _BC_H

#include <cecompress.h>

// max block size
#define BINCOMPRESS_MAX_BLOCK           CECOMPRESS_MAX_BLOCK

// signature lets us recognize compressed files
#define BINCOMPRESS_SIGNATURE_SIZE          4
#define BINCOMPRESS_SIGNATURE_VALUE         "XPRS"
extern CHAR BINCOMPRESS_SIGNATURE [BINCOMPRESS_SIGNATURE_SIZE];

// For storing compressed/uncompressed block lengths
typedef DWORD BLOCKLENGTH;

// Indicates noncompressed blocks (XPRESS only)
#define NO_COMPRESSION_BIT              (1L << 31)

// Indicates the last encoded block if it's original size 
// is smaller than CECOMPRESS_MAX_BLOCK (XPRESS only)
#define PARTIAL_BLOCK_BIT               (1L << 30)

#endif
