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
#ifndef _FAT32_H_
#define _FAT32_H_

#ifdef __cplusplus
extern "C" {
#endif

//
// Function prototypes
//
BOOL FAT32_FSInit(void);
ULONG FAT32_FSOpenFile(PCHAR pFileName);
void FAT32_FSCloseFile(void);
BOOL FAT32_FSReadFile(PUCHAR pAddress, ULONG Length);
BOOL FAT32_FSRewind(ULONG offset);


#ifdef __cplusplus
}
#endif

#endif	// _FAT32_H_
