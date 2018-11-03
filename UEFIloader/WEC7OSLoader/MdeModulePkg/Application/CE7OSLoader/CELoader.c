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
#include <Guid/Acpi.h>
#include <Library/UefiLib.h>
#include <Library/LocalApicLib.h>
#include <Protocol/LoadedImage.h>
#include <IndustryStandard/Acpi40.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "include/bldr.h"
#include "include/bootarg.h"
#include "include/bootdebug.h"


//------------------------------------------------------------------------------
//  Global variables

EFI_HANDLE          gCEImageHandle  = NULL;
EFI_SYSTEM_TABLE    *gCESystemTable = NULL;
extern VOID*        pAvailMem;

//------------------------------------------------------------------------------
typedef
void
(*PFN_LAUNCH)(
    );

EFI_GUID  gEfiAcpi20TableGuid = EFI_ACPI_20_TABLE_GUID;
    
//------------------------------------------------------------------------------
//  Local Functions
static
void
BootJumpTo(
    UINT32 address
    )
{
    ((PFN_LAUNCH)(address))();
}

//************************************************************************************
// BSquare solution for fixed GDT address is invalid in Startup.asm of WEC7 kernel
//************************************************************************************

// We want this structure packed exactly as declared!
#pragma pack(push, 1)
typedef struct {
    UINT16  GDTLength;
    VOID*   GDTPtr;
} FWORDPtr;
#pragma pack(pop)

static
VOID*
RelocateGDT(
    );
//************************************************************************************

/**
  UefiMain(), UEFI entry function, called by ProcessModuleEntryPointList() in AtuoGen.c,
 
  To find Firmware ACPI control strutcure in Acpi Tables since the S3 waking vector is stored 
  in the table
  
  @param Facs        a pointer to return out the address of RSDP 
  
  @retval  EFI_NOT_FOUND  The Facs is not existed or not found
  @retval  EFI_SUCCESS    Successfully found Facs. 
**/
EFI_STATUS
FindAcpiTable (
    UINT32      *pdwRSDPPtr
  )
{
    EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER    *pRsdp;
    UINTN                                           Index;
    EFI_STATUS                                      Status = EFI_SUCCESS;
    
    pRsdp  = NULL;
    
    // found ACPI table RSD_PTR from system table
    //
    for(Index = 0; Index < gST->NumberOfTableEntries; Index++) {
        if(CompareGuid (&(gST->ConfigurationTable[Index].VendorGuid), &gEfiAcpi20TableGuid) ||
            CompareGuid (&(gST->ConfigurationTable[Index].VendorGuid), &gEfiAcpi10TableGuid)) {
            //
            // A match was found.
            //
            pRsdp = gST->ConfigurationTable[Index].VendorTable;
            *pdwRSDPPtr = (UINT32) pRsdp;
            RTLMSG(PRINT_INFO, (L"FindAcpiTable: find Rsdp 0x%X\n", pRsdp));
            break;
        }
    }
    
    if(pRsdp == NULL) {
        RTLMSG(TRUE, (L"FindAcpiTable: failed to find Rsdp!!!!\n"));
        Status = EFI_NOT_FOUND;
    }
     
    return Status;
}

/**
  UefiMain(), UEFI entry function, called by ProcessModuleEntryPointList() in AtuoGen.c,
  it will do all things that includes initial hardware, set Boot config, load image,
  and then jump to image booting address.

  @param  ImageHandle   The firmware allocated handle for the EFI image.

  @param  SystemTable   A pointer to the EFI System Table.

  @return EFI_SUCCESS   The entry point is executed successfully.
  @return Other         Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
    enum_t                          BootState;
    uint32_t                        address;
    void *                          pContext = NULL;
    EFI_STATUS                      Status;
    UINTN                           MemoryMapSize;
    UINTN                           MapKey;
    UINTN                           DescriptorSize;
    UINT32                          DescriptorVersion;
    DWORD                           *pdwRSDPInRAM, dwRSDPPtr;
    static EFI_MEMORY_DESCRIPTOR    MemoryMap[256];

    RTLMSG(TRUE, (L"OS Loader application started!!!!\n"));

    // Save ImageHandle and SystemTable to be a global varuiable.
    gCEImageHandle = ImageHandle;
    gCESystemTable = SystemTable;
    
    // Call OEM init function
    pContext = OEMBootInit();
    if (pContext == NULL) {
        ERRMSG(TRUE, (L"UefiBootMain(): OEMBootInit failed \n"));
        goto powerOff;
    }
    
    // Find RSDP and store it to the last 4 bytes of BOOTARGS resvered page size 
    // of memory for OAL to parser other necessary Table in ACPI.
    // Actually, we did this is because OAL needs create a PCI routing table for 
    // PCI support, so BSquare will use this pointer to find PCI information to creat
    // a PCI routing table in OAL. 
    RTLMSG(PRINT_INFO, (L"UefiMain(): Called FindAcpiTable()\n"));
    Status = FindAcpiTable(&dwRSDPPtr);
    RTLMSG(PRINT_INFO, (L"UefiMain(): Called FindAcpiTable(), Status = 0x%X\n", Status));
    if(!EFI_ERROR(Status)) {
        pdwRSDPInRAM = (DWORD *) (IMAGE_SHARE_BOOT_ARGS_PA + (EFI_PAGE_SIZE - 4)); 
        CopyMem(pdwRSDPInRAM, &dwRSDPPtr, sizeof(DWORD));
        RTLMSG(PRINT_INFO, (L"UefiMain(): RSDP Table in RAM value is 0x%X!!!!\n",
        *pdwRSDPInRAM));
    }
    else
        ERRMSG(TRUE, (L"BootArgsInit(): FindAcpiTable failed to Get RSDP!!!!\n"));   

    // Call OEM load function until run state is returned
    BootState = BOOT_STATE_POWERON;

    for(;BootState != BOOT_STATE_RUN;) {
        BootState = OEMBootLoad(pContext, BootState);
        if (BootState == BOOT_STATE_FAILURE) {
            ERRMSG(TRUE, (L"UefiMain(): Loop Boot reserch failed, "
                L"(return code = 0x%x)\n", BootState));
            goto powerOff;
        }
        DBGMSG(DBGZONE_INFO, (L"UefiMain(): Loop Boot reserch, "
            L"Status = 0x%x\n", BootState));
        continue;
    }

    // Call OEM to prepare for image run
    address = OEMBootRun(pContext);

    //Pass GDT to Kernel
    RelocateGDT();
    
    // Disable all PIC interrupts.
    __asm
    {
        mov     al, 0FFh
        out     021h, al
    }

    // Disable imter APIC interrupts.
    DisableApicTimerInterrupt();    
    
    //
    //
    // Get the most current memory map.
    //
    MemoryMapSize = sizeof(MemoryMap);
    DBGMSG(DBGZONE_INFO,(L"CELoader Calliing gBS->GetMemoryMap()0x%x).\n",
        MemoryMapSize));
    Status = gBS->GetMemoryMap(
                              &MemoryMapSize,
                              MemoryMap,
                              &MapKey,
                              &DescriptorSize,
                              &DescriptorVersion
                              );

    if (!EFI_ERROR(Status)) {
        // Before jump to kernek, we need to call ExitBootServices()
        // to stop Boot services for release memory and so on
        Status = gBS->ExitBootServices(
                                       ImageHandle,
                                       MapKey
                                       );

        if (!EFI_ERROR(Status)) {
            // Jump to loaded image.
            if (address != 0)
                BootJumpTo(address);
        }
        else{
            ERRMSG(TRUE, (L"CELoader: Failed to terminate boot services, "
                L"(status = 0x%x).\n", Status));
        }
    }
    else{
        ERRMSG(TRUE, (L"CELoader: Failed to get memory map. "
            L"(status = 0x%x, size = 0x%x)\n", Status, MemoryMapSize));
    }

    return Status;
powerOff:
    Print(L"Spin forever...........");
    while(1)
    ;

    return Status;
}

/**
  GetGDT(), Loader will call this function to get GDT size form UEFI for make local GDT.

  @param  hFileHandle   A file handle that acquired by BootFileSystemOpen() function.

  @param  offset        Used to move file pointer to a request location.

  @return BOOLEAN.

**/
static
VOID*
GetGDT(
    UINT16* pLength
    )
{
    UINT32 TempGDT = 0;
    UINT16 TempGDTSize = 0;

    __asm {
        sub     esp, 8
        sgdt    [esp]
        mov     eax, dword ptr 2[esp]
        mov     TempGDT, eax
        mov     eax, dword ptr [esp]
        mov     TempGDTSize, ax
        add     esp, 8
    }

    *pLength = TempGDTSize;
    return (VOID*)TempGDT;
}


#pragma warning ( push )
#pragma warning ( disable : 4740 )
// C4740: flow in or out of inline asm code suppresses global optimizations

/**
  RelocateGDT(), UEFI has not allocate GDT memory base for OS to use, so Loader
  will call GetGDT() to get GDT size and pointer, then copy it to local GDT that
  include BOOTARGS content to pass into kernel.
  
  @return GDTBase.GDTPtr    A pointer to GDT address.
  
**/
static
VOID*
RelocateGDT(
    )
{
    BOOT_ARGS       **ppBootArgs = (BOOT_ARGS **)IMAGE_SHARE_BOOT_ARGS_PTR_PA;
    BOOT_ARGS       *pBootArgs   = (BOOT_ARGS *)IMAGE_SHARE_BOOT_ARGS_PA;
    static FWORDPtr GDTBase;
    VOID*           UEFIGDTPtr;
    UINT32          LocalGDTAddr = (UINT32)pAvailMem;

    *ppBootArgs = (BOOT_ARGS *)IMAGE_SHARE_BOOT_ARGS_PA;
    CopyMem(*ppBootArgs, pBootArgs, sizeof(BOOT_ARGS));

    // Make sure it is at the 64bit boundary.
    LocalGDTAddr += sizeof(UINT64) - 1;;
    LocalGDTAddr &= ~0x7;

    UEFIGDTPtr = GetGDT(&GDTBase.GDTLength);
    GDTBase.GDTPtr = (VOID*)LocalGDTAddr;

    pAvailMem = (VOID*)(LocalGDTAddr + GDTBase.GDTLength + 1);

    DBGMSG(DBGZONE_INFO, (L"Relocate GDT to 0x%x from 0x%x(size=0x%x)\n",
        GDTBase.GDTPtr, UEFIGDTPtr, GDTBase.GDTLength));
    CopyMem(GDTBase.GDTPtr, UEFIGDTPtr, GDTBase.GDTLength+1);
    DBGMSG(DBGZONE_INFO,
        (L"Relocate GDT getting ready to switch to local GDT\n"));

    __asm {
        lgdt    [GDTBase]
        push    cs
        push    offset LocalGDTIsLoadedNow
        retf
LocalGDTIsLoadedNow:
    }

    RTLMSG(PRINT_INFO, (L"Relocate GDT switching to local GDT done!!!\n"));

    return GDTBase.GDTPtr;
}
#pragma warning ( pop )

