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
#ifndef __BOOT_CORE_H
#define __BOOT_CORE_H

#include <bootTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define: BOOT_STATE_xxx
//
#define BOOT_STATE_POWERON          0
#define BOOT_STATE_RUN              1
#define BOOT_STATE_FAILURE          (-1)

#define BOOT_STATE_OEM              0x40000000

//------------------------------------------------------------------------------
//
//  Function:  OEMBootInit
//
//  This function is called by boot framework after it initializes global
//  variables, initialize system heap and initialize virtual memory (in
//  library with virtual memory support). This function should do required
//  hardware initialization and create boot context variable.
//
//  When initialization fails, function should return NULL instead context
//  variable pointer. If this happen boot framework will call OEMBootPowerOff.
//
//  When initialization succeed boot framework will continue and it will
//  call OEMBootLoad.
//
void*
OEMBootInit(
    );

//------------------------------------------------------------------------------
//
//  Function:  OEMBootLoad
//
//  Boot framework calls this function first after OEMBootInit with state
//  set to BOOT_STATE_POWERON. It will continue call it until function returns
//  state value different from BOOT_STATE_RUN or BOOT_STATE_FAILURE. Returned
//  value is used as new state.
//
//  When function returns BOOT_STATE_FAILURE framework will call 
//  OEMBootPowerOff.
//
//  When final image is placed in memory this function should return
//  BOOT_STATE_RUN. Framework than will call OEMBootRun.
//
enum_t
OEMBootLoad(
    void *pContext,
    enum_t state
    );

//------------------------------------------------------------------------------
//
//  Function:  OEMBootRun
//
//  Boot framework calls this function after OEMBootLoad returns
//  BOOT_STATE_RUN. Function should prepare hardware for loaded image run and
//  return physical start address. Boot framework switch CPU to physical
//  address mode if needed and jump to returned address.
//
uint32_t
OEMBootRun(
    void *pContext
    );

//------------------------------------------------------------------------------
//
//  Function:  OEMBootPowerOff
//
//  Boot framework calls this function when boot process fails. It isn't
//  expected code returns from this call. Depending on situation function
//  can switch off or reset device. It can be called with NULL context
//  if boot fails in OEMBootInit.
//
void
OEMBootPowerOff(
    void *pContext
    );

//------------------------------------------------------------------------------
//
//  Function:  OEMBootGetTickCount
//
//  This function returns system time with 1 ms resolution. Time has no
//  special base (in most cases it will be time since device reset/power on).
//  Code using this function should consider possible counter overflow.
//  Framework code doesn't call this function, but it will be used in most
//  boot loader implementations by some boot driver code.
//
uint32_t
OEMBootGetTickCount(
    );

//------------------------------------------------------------------------------
//
//  Function:  OEMBootStall
//
//  This function stalls CPU for given microseconds. It provide simply busy
//  wait loop. Framework code doesn't call this function.
//
void
OEMBootStall(
    uint32_t delay
    );

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_CORE_H
