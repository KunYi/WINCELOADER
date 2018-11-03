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
//
// -- Intel Copyright Notice -- 
//  
// @par 
// Copyright (c) 2002-2011 Intel Corporation All Rights Reserved. 
//  
// @par 
// The source code contained or described herein and all documents 
// related to the source code ("Material") are owned by Intel Corporation 
// or its suppliers or licensors.  Title to the Material remains with 
// Intel Corporation or its suppliers and licensors. 
//  
// @par 
// The Material is protected by worldwide copyright and trade secret laws 
// and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, 
// distributed, or disclosed in any way except in accordance with the 
// applicable license agreement . 
//  
// @par 
// No license under any patent, copyright, trade secret or other 
// intellectual property right is granted to or conferred upon you by 
// disclosure or delivery of the Materials, either expressly, by 
// implication, inducement, estoppel, except in accordance with the 
// applicable license agreement. 
//  
// @par 
// Unless otherwise agreed by Intel in writing, you may not remove or 
// alter this notice or any other notice embedded in Materials by Intel 
// or Intel's suppliers or licensors in any way. 
//  
// @par 
// For further details, please see the file README.TXT distributed with 
// this software. 
//  
// @par 
// -- End Intel Copyright Notice -- 
//  
#include "include/bootTerminalUtils.h"
#include "include/bldr.h"

/**
  In BootTerminalReadLine(), it's used for user input new IP address.

  @param  pBuffer       To point a fuffer to store input IP character.

  @param  bufferChars   Requested read size, the size is 16 character according to
                        IP format xxx.xxx.xxx.xxx

  @return pBuffer and data size stored in pBuffer.

**/

size_t
BootTerminalReadLine(
    wchar_t *pBuffer,
    size_t  bufferChars
    )
{
    size_t  count = 0;
    wchar_t key;

    while (count < bufferChars)
        {
        key = ReadKeyboardCharacter();
        if (key == L'\0') continue;
        if ((key == L'\r') || (key == L'\n')) {
            Print(L"\r\n");
            break;
        }
        if ((key == L'\b') && (count > 0)) {
            Print(L"\b \b");
            count--;
        }
        else if ((key >= L' ') && (key < 128) && (count < (bufferChars - 1))) {
            pBuffer[count++] = key;
            Print(L"%c", key);
        }
    }
    pBuffer[count] = L'\0';

    return count;
}

//------------------------------------------------------------------------------

