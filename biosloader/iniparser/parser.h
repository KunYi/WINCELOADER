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

#ifndef __BOOT_INI_PARSER_H
#define __BOOT_INI_PARSER_H

// Parser status code type
typedef ULONG PARSESTATUS;

// Parser status code values
#define INIPARSE_OK                 0   // Requested value parsed correctly
#define INIPARSE_PARAM_NOT_FOUND    1   // Requested parameter not found (spelling/case mistake?)
#define INIPARSE_PARSE_ERROR        2   // Parameter found, unable to parse the value (bad format?)
#define INIPARSE_NOT_INITED         3   // IniParserInit not called
#define INIPARSE_ARG_ERROR          4   // Bad arguments

// API function prototypes
void IniParserInit(PCHAR pIniRawData, ULONG ulLength);
ULONG GetIniStringLen(PCHAR pParameter);
PARSESTATUS GetIniString(PCHAR pParameter, PCHAR pBuf, ULONG ulLength);
PARSESTATUS GetIniByte(PCHAR pParameter, PBYTE pBuf);
PARSESTATUS GetIniWord(PCHAR pParameter, PWORD pBuf);
PARSESTATUS GetIniDword(PCHAR pParameter, PDWORD pBuf);
PARSESTATUS GetIniBool(PCHAR pParameter, PBOOL pBuf);

#endif
