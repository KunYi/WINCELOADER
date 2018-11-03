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

//------------------------------------------------------------------------------
#define APM_OFF 3
#define APM_SUSPEND 2
#define APM_STANDBY 1

void
OEMBootPowerOff(
    void * pContext
    )
{
    uint32_t eax, ebx, ecx, edx, esi, edi;
    uint32_t version;
    UNREFERENCED_PARAMETER(pContext);

    // Get APM version
    eax = 0x5300;
    ebx = 0x0000;
    BootBiosInt15(&eax, &ebx, &ecx, &edx, &esi, &edi);
    version = eax;

    // Print message 
    BootLog(L"\r\n*** Setting to low power state ***\r\n");
    
    // Connect to APM
    eax = 0x5301;
    ebx = 0x0000;
    BootBiosInt15(&eax, &ebx, &ecx, &edx, &esi, &edi);

    // Enable APM version bigger than 1.0 (1.0 doesn't power off)
    eax = 0x530E;
    ebx = 0x0000;
    ecx = version;
    BootBiosInt15(&eax, &ebx, &ecx, &edx, &esi, &edi);

    // Enable power management for all devices
    eax = 0x530D;
    ebx = 0x0001;
    ecx = 0x0001;
    BootBiosInt15(&eax, &ebx, &ecx, &edx, &esi, &edi);
    
    // Enagage power management for all devices
    eax = 0x530F;
    ebx = 0x0001;
    ecx = 0x0001;
    BootBiosInt15(&eax, &ebx, &ecx, &edx, &esi, &edi);
    
    // all devices
    eax = 0x5307;
    ebx = 0x0001;
    ecx = APM_STANDBY; // set this to APM_OFF to turn the CEPC off
    BootBiosInt15(&eax, &ebx, &ecx, &edx, &esi, &edi); 

    // idle
    eax = 0x5305;
    ebx = 0;
    ecx = 0;
    BootBiosInt15(&eax, &ebx, &ecx, &edx, &esi, &edi); // Virtual PC 07 stops here

    BootLog(L"\r\n*** HALT ***\r\n");

    _asm {
        cli
        hlt
    };

    BootLog(L"\r\n*** Busy Wait ***\r\n");

    for (;;)
        ;
}

//------------------------------------------------------------------------------
