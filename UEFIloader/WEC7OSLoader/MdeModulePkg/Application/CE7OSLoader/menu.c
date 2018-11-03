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
  BootTerminalMenu(), This function will get KEY pressed to determine into submenu or return
  to parant menu.
      
  @param  pContext      Point to a BootLoader_t structure that temporary store config value.
  
  @param  pActionMenu   Point to a submenu for display on terminal.

**/

void
BootTerminalMenu(
    void *pContext,
    void *pActionContext
    )
{
    BootLoader_t *pLoader       = pContext;
    BootTerminalMenu_t *pMenu   = pActionContext;
    BootTerminalMenuEntry_t     *pEntry;
    wchar_t                     key, line[80];
    enum_t                      ix;

    if (pMenu == NULL) goto cleanUp;

    for (;;) {
        // Print header
        Print(L"\r\n");
        for (ix = 0; ix < dimof(line) - 1; ix++) line[ix] = L'-';
        line[dimof(line) - 1] = L'\0';
        Print(L"%s\r\n", line);
        Print(L" %s\r\n", pMenu->title);
        Print(L"%s\r\n\r\n", line);

        // Print menu items
        for (pEntry = pMenu->entries; pEntry->key != 0; pEntry++) {
            Print(L" [%c] %s\r\n", pEntry->key, pEntry->text);
        }
        Print(L"\r\n Selection: ");

        for (;;) {
            // Get key
            key = ReadKeyboardCharacter();
            if ((CHAR)key == L'\0') continue;

            // Look for key in menu
            for (pEntry = pMenu->entries; pEntry->key != L'\0'; pEntry++) {
                if (pEntry->key == key) break;
            }

            // If we find it, break loop
            if (pEntry->key != L'\0') break;
        }

        // Print out selection character
        Print(L"%c\r\n", key);

        // When action is NULL return back to parent menu
        if ((pEntry->pfnAction == NULL) && (pEntry->pActionContext == NULL))
            break;

        if (pEntry->pfnAction == NULL) {
            BootTerminalMenu(pLoader, pEntry->pActionContext);
        }
        else {
            pEntry->pfnAction(pLoader, pEntry->pActionContext);
        }
    }

cleanUp:
    return;
}

//------------------------------------------------------------------------------

