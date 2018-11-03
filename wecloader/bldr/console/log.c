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
#include "bldr.h"
#include <bootBios.h>
#include <bootSerial.h>

//------------------------------------------------------------------------------
static uint16_t ioPortBase;

void
InitSerialEcho(uint8_t comPort)
{
    switch (comPort)
        {
        case 1: 
            ioPortBase = COM1_BASE;
            break;
        case 2: 
            ioPortBase = COM2_BASE;
            break;
        case 3: 
            ioPortBase = COM3_BASE;
            break;
        case 4: 
            ioPortBase = COM4_BASE;
            break;
        default:
            ioPortBase = 0;
            break;
        }

    if (ioPortBase) 
        {
        BootBiosInitializeSerialPort(ioPortBase);
        }
}


void
OEMBootLogWrite(
    wcstring_t string
    )
{
    uint32_t eax, ebx, ecx, edx, esi, edi;

    while (string[0] != L'\0')
        {
        eax = 0x0E00 | (uint8_t)string[0];
        ebx = 0x0007;
        BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
        if (ioPortBase)
            {
            BootBiosWriteByteToSerialPort(ioPortBase, (uint8_t) (*string));
            }
        string++;
        }
}

//------------------------------------------------------------------------------

wchar_t
OEMBootLogReadChar(
    )
{
    wchar_t ch = L'\0';
    uint32_t eax, ebx, ecx, edx, esi, edi;

    eax = 0x0100;
    BootBiosInt16(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if (eax != 0 && eax != 0x0100)
        {
        eax = 0x0000;
        BootBiosInt16(&eax, &ebx, &ecx, &edx, &esi, &edi);
        ch = (uint16_t)(eax & 0xFF);
        }
    return ch;

}

//------------------------------------------------------------------------------

