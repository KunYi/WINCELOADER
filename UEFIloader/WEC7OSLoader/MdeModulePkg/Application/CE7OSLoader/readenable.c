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
#include "include/bldr.h"

/**
  In BootTerminalReadEnable(), it's used for Boot Menu to present corrently state
  by pEnable value, and waiting for user input for set function Enable/Disable.

  @param  pEnable   To point a value that indicate corrent state for
                    user decide to change state or not.

  @param  prompt    A string that indicate item title.

**/

void
BootTerminalReadEnable(
    bool_t      *pEnable,
    wcstring_t  prompt
    )
{
    wchar_t key = L'\0';

    Print(
        L" %s %s (actually %s) [y/-]: ",
        *pEnable ? L"Disable" : L"Enable", prompt,
        *pEnable ? L"enabled" : L"disabled"
        );

    do {
        key = ReadKeyboardCharacter();
    }while (key == '\0');

    Print(L"%c\r\n", key);

    if ((key == L'y') || (key == L'Y')) {
        *pEnable = !*pEnable;
        Print(
            L" %s %s\r\n", prompt,
            *pEnable ? L"enabled" : L"disabled"
            );
    }
    else {
        Print(
            L" %s stays %s\r\n", prompt,
            *pEnable ? L"enabled" : L"disabled"
            );
    }
}

//------------------------------------------------------------------------------

