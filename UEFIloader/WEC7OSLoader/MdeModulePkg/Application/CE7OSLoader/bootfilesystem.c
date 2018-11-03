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
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DevicePathToText.h>
#include <Library/MemoryAllocationLib.h>

#include "include/boot.h"

//------------------------------------------------------------------------------
extern EFI_HANDLE       gCEImageHandle;
extern EFI_SYSTEM_TABLE *gCESystemTable;

static CHAR16*          gpRootFilePath = NULL;
static EFI_FILE_HANDLE  ghSimpleFileSystemHandle= NULL;

PUCHAR                  gpFileBuffer = NULL;
DWORD                   gFileBufferLength;
DWORD                   gCurFileBufferIndex;
DWORD                   gCurFileBufferLength;
DWORD                   gCurFilePosition;

/**
  In DevicePathToStr(), it's used to convert logic device path to sttring

  @param  DevPath   A pointer that point to logic device path for convert.

  @return CHAR16 *  A pointer that point to converted string location.

**/
static
CHAR16 *
DevicePathToStr (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevPath
  )
{
  EFI_STATUS                       Status;
  EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *DevPathToText;
  CHAR16                           *ToText;

  if (DevPath == NULL) {
    ERRMSG(TRUE, (L"DevicePathToStr(NULL)\n"));
    return L"";
  }

  Status = gBS->LocateProtocol (
                               &gEfiDevicePathToTextProtocolGuid,
                               NULL,
                               (VOID **) &DevPathToText
                               );
  if (!EFI_ERROR (Status)) {
    INFOMSG(PRINT_INFO, (L"DevicePathToStr: gBS->LocateProtocol() code=0x%x\n", Status));
    INFOMSG(PRINT_INFO, (L"DevicePathToStr: calling ConvertDevicePathToText(0x%x)\n", DevPath));

    ToText = DevPathToText->ConvertDevicePathToText (
                                                    DevPath,
                                                    FALSE,
                                                    TRUE
                                                    );
    return ToText;
  }
  else {
      ERRMSG(TRUE, (L"DevicePathToStr: gBS->LocateProtocol() failed with error code=0x%x\n", Status));
  }

  return L"";
}


/**
  In GetFileSize(), it's used to get image realy size in byte.

  @param  hFileHandle   A file handle that acquired by call EFI_FILE_HANDLE of Open().

  @return UINT32        File of size will read.

**/
static
UINT32
GetFileSize(
    EFI_FILE_HANDLE hFileHandle
    )
{
    EFI_STATUS  Status;
    UINT64      Size = 0;

    Status = hFileHandle->SetPosition(
                                     hFileHandle,
                                     0xFFFFFFFFFFFFFFFF
                                     );
    if (EFI_ERROR(Status)) {
        ERRMSG(TRUE, (L"GetFileSize(): Unabled to set file position to EOF (handle=0x%x).\n", hFileHandle));
    }
    else {
        Status = hFileHandle->GetPosition(
                                         hFileHandle,
                                         &Size
                                         );
        if (EFI_ERROR(Status)) {
            ERRMSG(TRUE, (L"GetFileSize(): Unabled to get file's current position(handle=0x%x).\n", hFileHandle));
        }
        else {
            Status = hFileHandle->SetPosition(
                                             hFileHandle,
                                             0
                                             );
        }
    }

    return (UINT32)Size;
}


/**
  In BootFileSystemInit(), it's used BS to get directory of BOOTIA32.efi that load
  from boot device and for BootFileSystemOpen() use to make correctly  path to open
  image file, meanwhile to prepare file buffer to optimize read performance.

  @return EFI_SUCCESS  The boot device path and file buffer allocate are successfully.
  @return Other        Some error occurs when executing this function.

**/
EFI_STATUS
EFIAPI
BootFileSystemInit(
    )
{

    EFI_STATUS              Status;
    EFI_LOADED_IMAGE        *LoadedImage;
    EFI_DEVICE_PATH         *DevicePath;
    EFI_FILE_IO_INTERFACE   *Vol;
    EFI_FILE_HANDLE         RootFs;
    CHAR16                  *DevicePathAsString;
    static CHAR16           PathName[100];
    UINTN                   i;


    if ((gCEImageHandle == NULL) || (gCESystemTable == NULL)) {
        ERRMSG(TRUE, (L"BootFileSystemInit(): Invalid ImageHandle or SystemHandle.\n"));
        return FALSE;
    }

    //
    // Get the device handle and file path to the EFI OS Loader itself.
    //
    Status = gBS->HandleProtocol(
                                gCEImageHandle,
                                &gEfiLoadedImageProtocolGuid,
                                (VOID**)&LoadedImage
                                );

    if (EFI_ERROR(Status)) {
       ERRMSG(TRUE, (L"FATOpen(): Can not retrieve a LoadedImageProtocol handle "
       L"for ImageHandle(return code = 0x%x)\n", Status));
        goto CleanUp;
    }

    DevicePath = NULL;
    Status = gBS->HandleProtocol(
                                gCEImageHandle,
                                &gEfiLoadedImageDevicePathProtocolGuid,
                                (VOID**)&DevicePath
                                );

    if (EFI_ERROR(Status) || DevicePath==NULL) {
        ERRMSG(TRUE, (L"Can not find a DevicePath handle for LoadedImage, "
        L"(return code = 0x%x)\n", Status));
        goto CleanUp;
    }

    DevicePathAsString = DevicePathToStr(DevicePath);

    if (DevicePathAsString != NULL) {
        INFOMSG(PRINT_INFO, (L"Image device : %s\n", DevicePathAsString));
        FreePool(DevicePathAsString);
    }

    DevicePathAsString = DevicePathToStr(LoadedImage->FilePath);

    if (DevicePathAsString != NULL) {
        INFOMSG(PRINT_INFO, (L"Image file   : %s\n", DevicePathAsString));
        StrCpy(PathName,DevicePathAsString);
        FreePool(DevicePathAsString);
    }

    Status = gBS->HandleProtocol (
                                 LoadedImage->DeviceHandle,
                                 &gEfiSimpleFileSystemProtocolGuid,
                                 (VOID**)&Vol
                                 );

    if (EFI_ERROR(Status)) {
        ERRMSG(TRUE, (L"Can not get a FileSystem handle for LoadedImage->DeviceHandle\n"));
        goto CleanUp;
    }

    Status = Vol->OpenVolume (
                             Vol,
                             &RootFs
                             );

    if (EFI_ERROR(Status)) {
        ERRMSG(TRUE, (L"Can not open the volume for the file system\n"));
        goto CleanUp;
    }
    else {
         INFOMSG(PRINT_INFO, (L"Open the volume for the file system returns OK\n"));
    }

    ghSimpleFileSystemHandle = RootFs;

    //
    // Open the file OSKERNEL.BIN in the same path as the EFI OS Loader.
    //
    for(i=StrLen(PathName);i>0 && (PathName[i]!='/') && (PathName[i]!='\\');i--);
    if( (i > 0) && (PathName[i-1] == '\\') )
        i-- ;
    PathName[i++] = '\\';
    PathName[i] = 0;


    // if the path to NK.bin starts with "\/", change that to "\"
    if ((PathName[0] == '\\') && (PathName[1] == '/')) {
        PathName[1] = '\\';
        gpRootFilePath = &PathName[1];
    } else {
        gpRootFilePath = &PathName[0];
    }

    //Allocate 2MB buffer as cache
    gFileBufferLength = 0x200000;
    Status = gBS->AllocatePool(
                              EfiLoaderData,
                              gFileBufferLength,
                              &gpFileBuffer
                              );

    if (gpFileBuffer == NULL)
        ERRMSG(TRUE, (L"Can not allocate Cache buffer.\n"));

CleanUp:

    if (EFI_ERROR(Status))
         ERRMSG(TRUE, (L"-BootFileSystemInit(), Status 0x%X\n", Status));

    return Status;
}
/**
  In BootFileSystemOpen(), it's used to open NK.bin by EFI_FILE_PROTOCOL->Open () function
  and set global variables gCurFileBufferIndex, gCurFileBufferLength, gCurFilePosition to
  0 for initial file buffer index.

  @param  FileName      To point a string that indicate which file will be open, example, "nk.bin"
                        this string will combine to a directory that BOOTIA32.efi be load   in boot device.

  @param  hFileHandle   To store file handle that acquired by call EFI_FILE_HANDLE of Open()  function.

  @param  pFileLength   Point to a address to store file size that acquire by call GetFileSize().


  @return EFI_SUCCESS   The NK.bin is opened successfully.
  @return Other         Some error occurs when executing this function.

**/
EFI_STATUS
EFIAPI
BootFileSystemOpen(
    CHAR16          *FileName,
    EFI_FILE_HANDLE *pFileHandle,
    UINT32          *pFileLength
    )
{
    EFI_STATUS          Status = EFI_INVALID_PARAMETER;
    EFI_FILE_HANDLE     FileHandle;
    CHAR16              PathName[256];

    if ((gCEImageHandle == NULL) || (gCESystemTable == NULL)) {
        ERRMSG(TRUE, (L"BootFileSystemOpen(): Invalid ImageHandle or SystemHandle.\n"));
        goto CleanUp;
    }

    // Make sure FATInit() was called and correctly initialize
    if ((ghSimpleFileSystemHandle == NULL) || (gpRootFilePath == NULL)) {
        ERRMSG(TRUE, (L"BootFileSystemOpen():Invalid partition handle or path name.\n"));
        goto CleanUp;
    }

    *pFileHandle = NULL;

    StrCpy(PathName, gpRootFilePath);
    StrCat(PathName, FileName);

    Status = ghSimpleFileSystemHandle->Open (
                                            ghSimpleFileSystemHandle,
                                            &FileHandle,
                                            PathName,
                                            EFI_FILE_MODE_READ,
                                            0
                                            );

    if (EFI_ERROR(Status)) {
        ERRMSG(TRUE, (L"Can not open the file %s\n",PathName));
        goto CleanUp;
    }
    else {
        *pFileLength = GetFileSize(FileHandle);
        *pFileHandle = FileHandle;
        gCurFileBufferIndex  = 0;
        gCurFileBufferLength = 0;
        gCurFilePosition     = 0;
        Print(L"Opened %s\n", PathName);
    }
CleanUp:
    return Status;
}

/**
  In BootFileSystemRead(), it will check if gpFileBuffer is allocated or not, if it's NULL,
  this function will use file size as Record Header request to issue EFI_FILE_PROTOCOL>Read() function,
  this way will cause read performance lower than BIOSLOADER, if gpFileBuffer is allocate,
  every read request will check file buffer to see if any data in there, if so,
  it will copy requested data from file buffer to RAM, but if remain data in file buffer is less than request,
  this function will read 2MB and do copy data for this read request, to use file buffer like a cache to reduce IO request and improve performance.

  @param  FileName      To point a string that indicate which file will be open,
                        this string will combine to a directory that BOOTIA32.efi be load
                        in boot device.

  @param  hFileHandle   A file handle that acquired by BootFileSystemOpen() function.

  @param  pFileLength   Point to a address to store file size that acquire by call GetFileSize.

  @return EFI_SUCCESS   The read file call is executed successfully.
  @return Other         Some error occurs when executing this function.

**/


UINT32
BootFileSystemRead(
    EFI_FILE_HANDLE hFileHandle,
    void            *pAddress,
    UINT32          Length
    )
{
    EFI_STATUS  Status;
    ULONG       Size;
    DWORD       curAvail;
    DWORD       curCopyLength;
    DWORD       ReadBytes;
    PUCHAR      pMemAddr = (PUCHAR)pAddress;

    if (gpFileBuffer == NULL) {
        Size = Length;

        // Call UEFI to read the file from FAT32.
        Status = hFileHandle->Read(
                                  hFileHandle,
                                  &Size,
                                  pMemAddr
                                  );

        if (EFI_ERROR(Status)) {
            ERRMSG(TRUE, (L"Failed to read file(handle = 0x%x).\n", hFileHandle));
            Size = 0;
        }
        else {
            gCurFilePosition     += Size;
        }
    }
    else {
        Size = 0;
        while (Length > 0) {
            if (gCurFileBufferIndex < gCurFileBufferLength) {
                curAvail = gCurFileBufferLength - gCurFileBufferIndex;
                curCopyLength = (curAvail >= Length) ? Length : curAvail;

                CopyMem(pMemAddr, &gpFileBuffer[gCurFileBufferIndex], curCopyLength);
                pMemAddr            += curCopyLength;
                gCurFileBufferIndex += curCopyLength;
                Size                += curCopyLength;
                Length              -= curCopyLength;
            }
            else {
                ReadBytes = gFileBufferLength;
                Status = hFileHandle->Read(
                                          hFileHandle,
                                          &ReadBytes,
                                          gpFileBuffer
                                          );

                gCurFileBufferIndex  = 0;

                if (EFI_ERROR(Status)) {
                    ERRMSG(TRUE, (L"Failed to read file(handle = 0x%x).\n", hFileHandle));
                    gCurFileBufferLength = 0;
                    break;
                }
                else {
                    gCurFileBufferLength = ReadBytes;
                    gCurFilePosition     += ReadBytes;

                }
            }
        }
    }

    return Size;
}

/**
  In BootFileSystemRewind(), it's will set file pointer location as offset by
  call EFI_FILE_PROTOCOL->SetPosition() function, if file buffer has been allocate,
  this function will set gCurFileBufferIndex  = 0, gCurFileBufferLength = 0 and
  gCurFilePosition = offset.

  @param  hFileHandle   A file handle that acquired by BootFileSystemOpen() function.

  @param  offset        Used to move file pointer to a request location.

  @return BOOLEAN.

**/
BOOLEAN
BootFileSystemRewind(
    EFI_FILE_HANDLE hFileHandle,
    UINT32          offset
    )
{
    EFI_STATUS  Status;

    if (gpFileBuffer == NULL) {
        Status = hFileHandle->SetPosition(
                                         hFileHandle,
                                         offset
                                         );
    }
    else {
        if ((offset > gCurFilePosition) || (offset < (gCurFilePosition - gCurFileBufferLength))) {
            // set the file position to offset.
            Status = hFileHandle->SetPosition(
                                             hFileHandle,
                                             offset
                                             );

            if (!EFI_ERROR(Status)) {
                gCurFileBufferIndex  = 0;
                gCurFileBufferLength = 0;
                gCurFilePosition     = offset;
            }
        }
        else {
            DWORD curStartingOffSet = gCurFilePosition - gCurFileBufferLength;

            gCurFileBufferIndex = offset - curStartingOffSet;
        }
    }

    return TRUE;
}

/**
  In BootFileSystemClose(), it will close file Handle by call EFI_FILE_PROTOCOL->Close() function.

  @param  hFileHandle   A file handle that acquired by BootFileSystemOpen() function.

**/


void
BootFileSystemClose(
    EFI_FILE_HANDLE hFileHandle
    )
{
    EFI_STATUS   Status;

    gCurFileBufferIndex  = 0;
    gCurFileBufferLength = 0;
    gCurFilePosition     = 0;

    Status = hFileHandle->Close(hFileHandle);
    if (EFI_ERROR(Status)) {
        ERRMSG(TRUE, (L"Can not close the file(handle=0x%x).\n", hFileHandle));
    }
}
