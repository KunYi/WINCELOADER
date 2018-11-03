/** @file
  This sample application bases on HelloWorld PCD setting
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2011, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiMemoryLib/MemLibInternals.h>

#include "include/bldr.h"
#include "include/globalvariable.h"

static EFI_GUID gEfiBSQUAREProtocolGuid =
    { 0x571B31A1, 0x9562, 0x11D2, { 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }};
//------------------------------------------------------------------------------

/**
  CheckIsGlobalVarVaild(), This function called by ReadConfigFromGlobalVar() to
  check GB is exist or not, if it get GB succes, the a BootConfig_t content will return
  to caller.

  @param  pBootConfig   A Pointer to a BootConfig_t for get GB vlaue.

  @return bool_t        A BOOL value to indicate GB is exist or not.

**/
bool_t
CheckIsGlobalVarVaild(
    BootConfig_t *pBootConfig
    )
{
    EFI_STATUS  status = EFI_INVALID_PARAMETER;
    UINTN       dataSize;
    bool_t      rc = true;

    DBGMSG(DBGZONE_INFO, (L"+CheckIsGlobalVarVaild()\n"));

    dataSize = sizeof (BootConfig_t);
    status = gRT->GetVariable (
                              L"BootConfigData",
                              &gEfiBSQUAREProtocolGuid,
                              NULL,
                              &dataSize,
                              (VOID *) pBootConfig
                              );

    if (EFI_ERROR (status)) {
        WARNMSG(TRUE, (L"-CheckIsGlobalVarVaild():No BootConfigData, "
        L"status = 0x%x\n", status));
        rc = false;
        goto cleanup;
    }

    if(!InternalMemCompareMem(pBootConfig->signature, L"BLDRCFG\x0A", 8)) {
        DBGMSG(DBGZONE_WARNING,
            (L"CheckIsGlobalVarVaild():Found BootConfigData, "
             L"but Signature not match!!!\n"));
        rc = false;
    }

cleanup:

    DBGMSG(DBGZONE_INFO, (L"-CheckIsGlobalVarVaild(), rc = 0x%x\n", rc));
    return rc;
}


/**
  InitializeGlobalVariables(), This function called by DefaultConfig() to
  check GB is exist or not, if it GB is not exist, this function will set
  default value to GB.

  @return bool_t        A BOOL value to indicate initialed BootConfig
                        is scuuess save into GB or not.

**/
bool_t
InitializeGlobalVariables(
    )
{
    EFI_STATUS      status = EFI_INVALID_PARAMETER;
    BootConfig_t    BootConfig;
    UINTN           dataSize;
    bool_t          rc = true;
    UINT32          attribute = 0x3;

    Print(L"+InitializeGlobalVariables()");

    InternalMemCopyMem(BootConfig.signature, BOOT_CONFIG_SIGNATURE, 8);
    BootConfig.version = BOOT_CONFIG_VERSION;
    BootConfig.ipAddress = 0x0100000A;
    BootConfig.kitlFlags = 0;
    BootConfig.bootDevice.type = DeviceTypeStore;        // Boot device location
    BootConfig.bootDevice.ifc = (enum_t)IfcTypeUndefined;
    BootConfig.bootDevice.busNumber = 0;
    BootConfig.bootDevice.location = 0;

    BootConfig.kitlDevice.type = DeviceTypeEdbg;        // Kitl device location
    BootConfig.kitlDevice.ifc = IfcTypePci;
    BootConfig.kitlDevice.busNumber = 0;
    BootConfig.kitlDevice.location = 0;

    BootConfig.displayWidth = 640;
    BootConfig.displayHeight = 480;
    BootConfig.displayBpp = 16;

    BootConfig.displayLogicalWidth = 640;
    BootConfig.displayLogicalHeight = 480;

    BootConfig.comPort = 1;                   // Debug port COM1
    BootConfig.baudDivisor = 1;               // baudrate 115000

    BootConfig.imageUpdateFlags = 0;         // Default no update mode

    dataSize = sizeof (BootConfig);
    status = gRT->SetVariable (
                              L"BootConfigData",
                              &gEfiBSQUAREProtocolGuid,
                              attribute,
                              dataSize,
                              (VOID *) &BootConfig
                              );

    if (EFI_ERROR (status)) {
        rc = false;
        Print(L"InitializeGlobalVariables():can't store BootConfig in Global "
        L"Variable, status = 0x%x\n", status);
    }

    Print(L"-InitializeGlobalVariables(), rc = 0x%x\n", rc);
    return rc;
}

/**
  UpdateGlobalVariables(), This function called by WriteConfigToDisk() to
  save BootConfig to GB.

  @param  pBootConfig   A Pointer to a BootConfig_t for save into GB.

  @return bool_t        A BOOL value to indicate BootConfig is scuuess save into GB or not.

**/
bool_t
UpdateGlobalVariables(
    BootConfig_t *pBootConfig
    )
{
    EFI_STATUS  status = EFI_INVALID_PARAMETER;
    UINTN       dataSize;
    bool_t      rc = true;
    UINT32      attribute = 0x3;

    DBGMSG(DBGZONE_INFO, (L"+UpdateGlobalVariables()\n"));

    dataSize = sizeof (BootConfig_t);
    status = gRT->SetVariable (
                              L"BootConfigData",
                              &gEfiBSQUAREProtocolGuid,
                              attribute,
                              dataSize,
                              (VOID *) pBootConfig
                              );

    if (EFI_ERROR (status)) {
        rc = false;
        Print(L"UpdateGlobalVariables():can't store BootConfig in Global Variable,"
        L" status = 0x%x\n", status);
    }

    DBGMSG(DBGZONE_INFO, (L"-UpdateGlobalVariables(), rc = 0x%x\n", rc));
    return status;
}
