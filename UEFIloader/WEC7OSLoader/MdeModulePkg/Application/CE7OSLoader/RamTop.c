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

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Guid/VariableFormat.h>

#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>

#include <Protocol\DevicePathUtilities.h>
#include <Protocol\DevicePathToText.h>

#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib/BaseLibInternals.h>
#include <Protocol/Runtime.h>

#include "include/boottypes.h"
#include "include/bootdebug.h"

#if 1
#define PRINTMEMORYDESCRIPTOR(x)
#else
#define PRINTMEMORYDESCRIPTOR(x)    PrintMemoryDescriptor(x)

/**
  PrintMemoryDescriptor(), This function to print all UEFI usaged RAM information.

  @param  pMemDescriptor   A pointer to a EFI_MEMORY_DESCRIPTOR content.

**/

VOID
PrintMemoryDescriptor(
    EFI_MEMORY_DESCRIPTOR *pMemDescriptor
    )

{
    switch (pMemDescriptor->Type) {
        case  EfiReservedMemoryType:
            Print(L"\tReserved Memory: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        case EfiLoaderCode:
            Print(L"Loader Code: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        case EfiLoaderData:
            Print(L"Loader Data: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        case EfiBootServicesCode:
            Print(L"Boot Service Code: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        case EfiBootServicesData:
            Print(L"Boot Service Data: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        case EfiRuntimeServicesCode:
            Print(L"Run Time Service Code: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        case EfiRuntimeServicesData:
            Print(L"Run Time Service Data: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        case EfiConventionalMemory:
            Print(L"Conventional Memory: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        case EfiUnusableMemory:
            Print(L"Unusable Memory: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        case EfiACPIReclaimMemory:
            Print(L"ACPI Reclaim Memory: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        case EfiACPIMemoryNVS:
            Print(L"ACPI Memory NVS: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        case EfiMemoryMappedIO:
            Print(L"Memory mapped IO: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        case EfiMemoryMappedIOPortSpace:
            Print(L"Memory Mapped IO Port Space: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        case EfiPalCode:
            Print(L"Pal Code: phy=0x%x, virt=0x%x, pages=0x%x\n",
                    (UINT32)pMemDescriptor->PhysicalStart, (UINT32)pMemDescriptor->VirtualStart,
                    (UINT32)pMemDescriptor->NumberOfPages);
            break;
        default:
            break;
    }
}
#endif


/**
  PrintMemoryDescriptor(), This function is to caculate UEFI usage RAM size and then
  get available RMA size that OS can use for runs, if we don't report the available sise
  to OS, it will cause booting failure in kernel code

  @param  pRamTop   A pointer to get ram top.

  @return EFI_SUCCESS   This is executed successfully to get available RAM size.
  @return Other         Some error occurs when executing this function.
    
**/
EFI_STATUS
GetRamTop (
    UINT64* pRamTop
    )
{
    EFI_STATUS              Status;
    EFI_MEMORY_DESCRIPTOR   *Buffer   = NULL;
    UINTN                   Size;
    UINTN                   MapKey;
    UINTN                   ItemSize;
    UINT32                  Version;
    UINT64                  RamTop      = 0;
    BOOLEAN                 RamTopFound = FALSE;

    Size = 0;
    Status = gBS->GetMemoryMap(&Size, Buffer, &MapKey, &ItemSize, &Version);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        Size += SIZE_1KB;
        Buffer = AllocateZeroPool(Size);
        Status = gBS->GetMemoryMap(&Size, Buffer, &MapKey, &ItemSize, &Version);
    }

    if (EFI_ERROR(Status)) {
        ERRMSG(TRUE, (L"GetRamTop failed to Get Memory Map (status = 0x%x).\n", Status));
    }
    else {
        UINT8               *Walker;

        for (Walker = (UINT8*)Buffer; Walker < (((UINT8*)Buffer)+Size) && Walker != NULL; Walker += ItemSize) {
            EFI_MEMORY_DESCRIPTOR *pMemDescriptor = (EFI_MEMORY_DESCRIPTOR *)Walker;
            UINT64                TempRamTop;

            PRINTMEMORYDESCRIPTOR(pMemDescriptor);

            // If we haven't hit Runtime, ACPI, MemoryMappedIO, or PALCode, we assume the RamTop is at the end of
            // current memory block. Once we hit a RunTime, ACPI, MemoryMappedIO, or PALCode, we set the RamTop to
            // the PhysicalStart in the current descriptor.
            switch (pMemDescriptor->Type) {
                case EfiReservedMemoryType:
                case EfiLoaderCode:
                case EfiLoaderData:
                case EfiBootServicesCode:
                case EfiBootServicesData:
                case EfiConventionalMemory:
                case EfiUnusableMemory:
                    TempRamTop = pMemDescriptor->PhysicalStart;
                    TempRamTop += MultU64x64(SIZE_4KB, pMemDescriptor->NumberOfPages);

                    if ((RamTopFound == FALSE) && (RamTop <= TempRamTop)) {
                        RamTop = TempRamTop;
                    }
                    break;
                case EfiRuntimeServicesCode:
                case EfiRuntimeServicesData:
                case EfiACPIReclaimMemory:
                case EfiACPIMemoryNVS:
                case EfiMemoryMappedIO:
                case EfiMemoryMappedIOPortSpace:
                case EfiPalCode:
                    if ((RamTop >= pMemDescriptor->PhysicalStart) || (RamTopFound == FALSE)){
                        RamTop = pMemDescriptor->PhysicalStart;
                    }
                    RamTopFound = TRUE;
                    break;
                default:
                    break;
            }
        }
    }

    if (Buffer != NULL) {
      FreePool(Buffer);
    }

    if (pRamTop != NULL) {
        *pRamTop = RamTop;
    }

    INFOMSG(TRUE, (L"GetRamTop: RampTop = 0x%x\n", RamTop));

    return (Status);
}
