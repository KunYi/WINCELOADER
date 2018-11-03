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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:
    bootarg.h

Abstract:
    Definitions for arguments passed by loadcepc at boot time.

Notes:

--*/
#ifndef _BOOTARG_H
#define _BOOTARG_H

#include <IndustryStandard/Acpi40.h>

#include "include/bootTypes.h"

#define BOOT_ARG_VERSION_SIG    0x12345678
#define BOOT_ARG_MAJOR_VER      1
#define BOOT_ARG_MINOR_VER      3
#define MAX_DEV_NAMELEN         16  // Should match EDBG_MAX_DEV_NAMELEN.

typedef struct _BOOT_ARGS {
    UCHAR   ucVideoMode;
    UCHAR   ucComPort;
    UCHAR   ucBaudDivisor;
    UCHAR   ucPCIConfigType;

    // The following args are not set by older versions of loadcepc,
    // so include a sig to verify that the remaining params are valid.
    // Also, include a length to allow expansion in the future.
    DWORD   dwSig;
    #define BOOTARG_SIG         0x544F4F42 // "BOOT"
    DWORD   dwLen;              // Total length of boot args struct
    UCHAR   ucLoaderFlags;      // Flags set by loader
    UCHAR   ucEshellFlags;      // Flags from eshell
    UCHAR   ucEdbgAdapterType;  // Type of debug Ether adapter
    UCHAR   ucEdbgIRQ;          // IRQ line to use for debug Ether adapter
    #define BOOTARG_IRQ_INVALID 0xFF
    DWORD   dwEdbgBaseAddr;     // Base I/O address for debug Ether adapter
    DWORD   dwEdbgDebugZone;    // Allow EDBG debug zones to be turned on from loadcepc
    // The following is only valid if LDRFL_ADDR_VALID is set
    EDBG_ADDR EdbgAddr;         // IP/ether addr to use for debug Ethernet
    // The following addresses are only valid if LDRFL_JUMPIMG is set, and corresponding bit in
    // ucEshellFlags is set (configured by eshell, bit definitions in ethdbg.h).
    EDBG_ADDR EshellHostAddr;   // IP/ether addr and UDP port of host running eshell
    EDBG_ADDR DbgHostAddr;      // IP/ether addr and UDP port of host receiving dbg msgs
    EDBG_ADDR CeshHostAddr;     // IP/ether addr and UDP port of host running ether text shell
    EDBG_ADDR KdbgHostAddr;     // IP/ether addr and UDP port of host running kernel debugger
    DWORD DHCPLeaseTime;        // Length of DHCP IP lease in seconds
    WORD EdbgFlags;             // Information about the ethernet system
    WORD KitlTransport;         // Transport for Kitl communication

    DWORD dwEBootFlag;          // Eboot flags indicating whether EBoot supports warm reset (older version may not)
    DWORD dwEBootAddr;          // Eboot entry point set by eboot and used during warm reset
    DWORD dwLaunchAddr;         // Old image launch address saved by EBoot when it receives jmpimage

    // The following args added to support passing info to flat framebuffer display driver
    DWORD   pvFlatFrameBuffer;  // pointer to flat frame buffer
    WORD    vesaMode;           // VESA mode being used
    WORD    cxDisplayScreen;    // displayable X size
    WORD    cyDisplayScreen;    // displayable Y size
    WORD    cxPhysicalScreen;   // physical X size
    WORD    cyPhysicalScreen;   // physical Y size
    WORD    cbScanLineLength;   // scan line byte count
    WORD    bppScreen;          // color depth
    UCHAR   RedMaskSize;        // size of red color mask
    UCHAR   RedMaskPosition;    // position for red color mask
    UCHAR   GreenMaskSize;      // size of green color mask
    UCHAR   GreenMaskPosition;  // position for green color mask
    UCHAR   BlueMaskSize;       // size of blue color mask
    UCHAR   BlueMaskPosition;   // position for blue color mask

    // Version information for bootargs structure.
    ULONG   dwVersionSig;
    USHORT  MajorVersion;
    USHORT  MinorVersion;

    // Version 1.0 additions
    UCHAR   szDeviceNameRoot[MAX_DEV_NAMELEN];  // EDBG device name root.

    DWORD   dwImgStoreAddr;     // Flash storage address.
    DWORD   dwImgLoadAddr;      // RAM load address.
    DWORD   dwImgLength;        // Image length.
    BYTE    NANDBootFlags;      // Boot flags related to NAND support.
    BYTE    NANDBusNumber;      // NAND controller PCI bus number.
    DWORD   NANDSlotNumber;// NAND controller PCI slot number.

    // Version 1.1 additions
    ULARGE_INTEGER RamTop;

    // Version 1.2 uses PC-ffffffffff bootme names (previous cepc65535)

    // Version 1.3
    ULARGE_INTEGER VideoRam;


} BOOT_ARGS, *PBOOT_ARGS;

// Defs for loader flags
#define    LDRFL_USE_EDBG    0x0001  // Set to attempt to use debug Ethernet
// The following two flags are only looked at if LDRFL_USE_EDBG is set
#define    LDRFL_ADDR_VALID  0x0002  // Set if EdbgAddr field is valid
#define    LDRFL_JUMPIMG     0x0004  // If set, don't communicate with eshell to get

// The following flag is to indicate the user store needs to be formatted
#define    LDRFL_FORMAT_STORE     0x0020  // Format User Stores

// The following flag is used to disable VMINI
// This variable is not added to BootArg to avoid BC issues
#define    LDRFL_KITL_DISABLE_VMINI     0x0040  // Disable VMINI if set

// The following flag is only used for backup FLASH operation
#define     LDRFL_FLASH_BACKUP  0x80
// configuration, use ucEshellFlags field.

// Use this constant in EdbgIRQ to specify that EDBG should run without an interrupt.
#define EDBG_IRQ_NONE  0xFF

// Maximum file length
#define MAX_FILE_NAME_LEN           12              // TODO: reslove FAT32 question

// Maximum IP address length
#define MAX_IP_ADDRESS_LEN          15

// ARGS defaults
#define DEFAULT_DEBUG_ZONE          0x00000007      // DBGZONE_ERROR, DBGZONE_WARNING, DBGZONE_INFO
#define DEFAULT_DEVICE_NAME_ROOT    "Crown Bay"

#define DEFAULT_COMPORT             1
#define DEFAULT_BAUDRATE            1   //115200 baud

#define DEFAULT_DISPLAY_WIDTH       640
#define DEFAULT_DISPLAY_HEIGHT      480
#define DEFAULT_DISPLAY_DEPTH       8

#define EDBG_ADAPTER_SMC9000  0
#define EDBG_ADAPTER_NE2000   1
#define EDBG_ADAPTER_DEFAULT  2
#define EDBG_ADAPTER_DEC21140 3
#define EDBG_ADAPTER_RTL8139  4
#define EDBG_ADAPTER_DP83815  5
#define EDBG_ADAPTER_3C90X    6

#define EDBG_ADAPTER_OEM      16

#define EDBG_USB_RNDIS 'R'

#endif // _BOOTARG_H

