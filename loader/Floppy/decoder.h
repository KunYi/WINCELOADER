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

#ifndef _DECODER_H
#define _DECODER_H

#include <fs.h>

#ifdef CECOMPRESSION

// Use Compression

#include <bc.h>

BOOL CeCompressInit(void);
ULONG CeCompressOpenFile(PCHAR pFileName);
void CeCompressCloseFile(void);
BOOL CeCompressReadFile(PUCHAR pAddress, ULONG Length);


#define BINCOMPRESS_INIT                CeCompressInit
#define BINCOMPRESS_OPEN_FILE           CeCompressOpenFile
#define BINCOMPRESS_CLOSE_FILE          CeCompressCloseFile
#define BINCOMPRESS_READ_FILE           CeCompressReadFile

#else

// No compression
#define BINCOMPRESS_INIT                FSInit
#define BINCOMPRESS_OPEN_FILE           FSOpenFile
#define BINCOMPRESS_CLOSE_FILE          FSCloseFile
#define BINCOMPRESS_READ_FILE           FSReadFile

#endif


#endif
