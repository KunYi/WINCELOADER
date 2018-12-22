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
#include <bootLog.h>

VOID
BootLogDumpMemory(
    const void* pBuffer,
    uint32_t base,
    size_t size
    )
{
    wchar_t line[128];
    uint32_t pos, ix, size0;
    size_t col;


    size0 = base & 0xF;
    base &= ~0x000F;

    pos = 0;
    while (pos < size)
        {
        col = BootLogSPrintf(line, dimof(line), L"%08x", base);
        for (ix = 0; ix < 16; ix++)
            {
            if ((ix < size0) || ((pos + ix - size0) >= size))
                {
                col += BootLogSPrintf(&line[col], dimof(line) - col, L"   ");
                }
            else
                {
                col += BootLogSPrintf(
                    &line[col], dimof(line) - col, L" %02x", 
                    ((uint8_t*)pBuffer)[pos + ix - size0]
                    );
                }
            }
        
        col += BootLogSPrintf(&line[col], dimof(line) - col, L"   ");

        for (ix = 0; ix < 16; ix++)
            {
            if ((ix < size0) || ((pos + ix - size0) >= size))
                {
                col += BootLogSPrintf(&line[col], dimof(line) - col, L" ");
                }
            else
                {
                uint8_t c = ((uint8_t *)pBuffer)[pos + ix - size0];
                if ((c < ' ') || (c >= 0x80)) c = '.';
                col += BootLogSPrintf(&line[col], dimof(line) - col, L"%c", c);
                }
            }
        
        col += BootLogSPrintf(&line[col], dimof(line) - col, L"\r\n");
        BootLog(line);

        pos += (16 - size0);
        base += 16;
        size0 = 0;            
        }
}

//------------------------------------------------------------------------------

