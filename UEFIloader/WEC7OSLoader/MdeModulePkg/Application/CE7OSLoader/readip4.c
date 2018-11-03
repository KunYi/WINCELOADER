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

/**
  In StringToIp4(), it's used convert user input IP address string to
  physical address.

  @param  szIp4     IP string for convert.

  @param  pIp4      To point a address to store converted IP address.

  @return pIp4 and BOOL state.

**/

static
bool_t
StringToIp4(
    wcstring_t  szIp4,
    uint32_t    *pIp4
    )
{
    bool_t      rc = false;
    uint32_t    ip4 = 0;
    wcstring_t  psz = szIp4;
    enum_t      count = 0, part = 0;

    // Replace the dots with NULL terminators
    while (count < 4) {
        if ((*psz == L'.') || (*psz == L'\0')) {
            ip4 |= part << (count << 3);
            part = 0;
            count++;
        }
        else if ((*psz >= L'0') && (*psz <= L'9')) {
            part = part * 10 + (*psz - L'0');
            if (part > 255) break;
        }
        else {
            break;
        }
        if (*psz == L'\0') break;
        psz++;
    }

    rc = (count >= 4);
    if (rc) *pIp4 = ip4;
    return rc;
}

/**
  In StringToIp4, it's used to show corrent IP address and calls BootTerminalReadLine()
  for user input new IP address.

  @param  pIp4      A pointer that store converted IP address.

  @param  prompt    A string that indicate item title.

  @return pIp4 and BOOL state.

**/

bool_t
BootTerminalReadIp4(
    uint32_t    *pIp4,
    wcstring_t  prompt
    )
{
    bool_t      rc = false;
    uint32_t    ip4 = *pIp4;
    wchar_t     buffer[16];

    // Print prompt
    Print(
        L" Enter %s IP address (actual %d.%d.%d.%d): ", prompt,
        ((uint8_t*)&ip4)[0], ((uint8_t*)&ip4)[1], ((uint8_t*)&ip4)[2],
        ((uint8_t*)&ip4)[3]
        );

    // Read input line
    if (BootTerminalReadLine(buffer, dimof(buffer)) == 0)
        goto cleanUp;

    // Convert string to IP address
    if (!StringToIp4(buffer, &ip4)) {
        Print(
            L" '%s' isn't valid IP address\r\n", buffer
            );
        goto cleanUp;
    }

    // Print final IP address
    Print(
        L" %s IP address set to %d.%d.%d.%d\r\n", prompt,
        ((uint8_t*)&ip4)[0], ((uint8_t*)&ip4)[1], ((uint8_t*)&ip4)[2],
        ((uint8_t*)&ip4)[3]
        );

    // Save new setting
    *pIp4 = ip4;
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

