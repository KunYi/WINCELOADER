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
#include "xldr.h"
#include <bootLog.h>
#include <bootBios.h>
#include <bootSerial.h>

//------------------------------------------------------------------------------

void
OEMBootLogWrite(
    wcstring_t string
    )
{
    static bool_t init = false;

    if (!init)
    {
        BootBiosInitializeSerialPort(COM1_BASE);
        init = true;
    }

    while (*string != L'\0')
    {
        BootBiosWriteByteToSerialPort(COM1_BASE, (uint8_t) (*string));
        string++;
    }
}

//------------------------------------------------------------------------------

void
BootLog(
    wcstring_t format,
    ...
    )
{
    va_list pArgList;
    wchar_t buffer[128];
    
    va_start(pArgList, format);
    BootLogVSPrintf(buffer, dimof(buffer), format, pArgList, TRUE);
    OEMBootLogWrite(buffer);
}

//------------------------------------------------------------------------------

