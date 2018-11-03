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
#include <boot.h>
#include <bootCore.h>
#include <bootBios.h>
#include <bootBlockBios.h>

    
#include <fs.h>
#include <fat16_funcs.h>
#include <fat32_funcs.h>

// Number of times to retry an IO operation
#define FLOPPY_IO_RETRIES           4
#define FIXEDDISK_IO_RETRIES        0

extern "C" int InitializeFileSys (void);

typedef BOOL  (* _FSInit)(void);
typedef ULONG (* _FSOpenFile)(char*);
typedef void  (* _FSCloseFile)(void);
typedef BOOL  (* _FSReadFile)(PUCHAR, ULONG);
typedef BOOL  (* _FSRewind)(ULONG);


//extern PBIOSPB pBPB;
static DRIVE_INFO drv_info;
extern "C" { 
DWORD dwZoneMask = 0xffffffff;
}
BOOL BootDisk_Init (enum_t driveId);

_FSInit			pFSInit = NULL;
_FSOpenFile		pOpenFile = NULL;
_FSCloseFile	pCloseFile = NULL;
_FSReadFile		pReadFile = NULL;
_FSRewind		pRewind = NULL;
//======================================================================
//
//
int InitializeFileSys (void)
{
	BOOL FSInitGood = FALSE;

    BootBlockBiosParameterBlockType1_t *pBpb1;
    BootBlockBiosParameterBlockType2_t *pBpb2;
    BootBlockBiosParameterBlockTypeEx_t *pBpbEx;

	DEBUGMSG(ZONE_INFO | ZONE_INIT, (L"InitializeFileSys++\r\n"));

    // Get boot driver from Bios Parameter Block
    pBpbEx = (BootBlockBiosParameterBlockTypeEx_t *)0xFA03;
    pBpb2 = (BootBlockBiosParameterBlockType2_t *)0xFA03;
    pBpb1 = (BootBlockBiosParameterBlockType1_t *)0xFA03;
    if (memcmp(pBpbEx->versionId, "exFAT   ", sizeof(pBpbEx->versionId)) == 0)
        {
        drv_info.DriveId = pBpbEx->driveId;
		drv_info.usSectorsPerTrack = pBpb2->sectorsPerTrack;
		drv_info.usNumHeads = pBpb2->numberOfHeads;
		DEBUGMSG(ZONE_ERROR, (L"InitializeFileSys:: Found exFAT boot drive.  UNSUPPORTED FILE SYSTEM!!!\r\n"));
		return -1;
    }
    else if (memcmp(pBpb2->fatType, "FAT32   ", sizeof(pBpb2->fatType)) == 0)
    {
        drv_info.DriveId = pBpb2->driveId;
		drv_info.usSectorsPerTrack = pBpb2->sectorsPerTrack;
		drv_info.usNumHeads = pBpb2->numberOfHeads;

		pFSInit = &FAT32_FSInit;
		pOpenFile = &FAT32_FSOpenFile;
		pCloseFile = &FAT32_FSCloseFile;
		pReadFile = &FAT32_FSReadFile;
		pRewind = &FAT32_FSRewind;

		DEBUGMSG(ZONE_INFO, (L"InitializeFileSys:: Found FAT32 boot drive. FS init code %d\r\n", FSInitGood));
    }
    else if (memcmp(pBpb1->fatType, "FAT16   ", sizeof(pBpb1->fatType)) == 0)
    {
        drv_info.DriveId = pBpb1->driveId;
		drv_info.usSectorsPerTrack = pBpb1->sectorsPerTrack;
		drv_info.usNumHeads = pBpb1->numberOfHeads;

		pFSInit = &FAT16_FSInit;
		pOpenFile = &FAT16_FSOpenFile;
		pCloseFile = &FAT16_FSCloseFile;
		pReadFile = &FAT16_FSReadFile;
		pRewind = &FAT16_FSRewind;

		DEBUGMSG(ZONE_INFO, (L"InitializeFileSys:: Found FAT16 boot drive. FS init code %d\r\n", FSInitGood));
    }
    else if (memcmp(pBpb1->fatType, "FAT12   ", sizeof(pBpb1->fatType)) == 0)
    {
        drv_info.DriveId = pBpb1->driveId;
		drv_info.usSectorsPerTrack = pBpb1->sectorsPerTrack;
		drv_info.usNumHeads = pBpb1->numberOfHeads;

		DEBUGMSG(ZONE_ERROR, (L"InitializeFileSys:: Found FAT12 boot drive.  UNSUPPORTED FILE SYSTEM!!!\r\n"));
		return -1;
	}

   
	DEBUGMSG(ZONE_INFO, (L"Calling BootDisk_Init. id %x\r\n", drv_info.DriveId));
	FSInitGood = BootDisk_Init (drv_info.DriveId);
	DEBUGMSG(ZONE_INFO, (L"BootDisk_Init returned %d\r\n", FSInitGood));

	if (FSInitGood)
	{
		DEBUGMSG(ZONE_INFO, (L"Calling specific file system Init function. pFSInit\r\n"));
		FSInitGood =  pFSInit();
		DEBUGMSG(ZONE_INFO, (L"FSInit returned %d\r\n", FSInitGood));
	}

	return FSInitGood;
}


//======================================================================
// Redirection thunks to the loaded file system.
//
ULONG FSOpenFile( char* pFileName )
{
	return pOpenFile (pFileName);
}
void  FSCloseFile( void )
{
	pCloseFile ();
}
BOOL  FSReadFile( unsigned char* pAddress, ULONG Length )
{
	return pReadFile (pAddress, Length);
}
BOOL  FSRewind( ULONG Offset )
{
	return pRewind (Offset);
}
//=====================================================================

////////////////////
//------------------------------------------------------------------------------
typedef struct BiosDriveParameters_t {
    uint16_t size;
    uint16_t flags;
    uint32_t physicalCylinders;
    uint32_t physicalHeads;
    uint32_t physicalSectors;
    uint32_t sectorsLo;
    uint32_t sectorsHi;
    uint16_t sectorSize;
} BiosDriveParameters_t;

//------------------------------------------------------------------------------
//
//
BOOL BootDisk_Init (enum_t driveId)
{
    bool rc = false;
    BiosDriveParameters_t *pInfo;
    uint32_t eax, ebx, ecx, edx, esi, edi;

    // Find if there is BIOS Enhanced Disk Drive Services support
    eax = 0x4100;
    ebx = 0x55AA;
    edx = driveId;
    BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if ((ebx & 0xFFFF) == 0xAA55 && (ecx & 1))
	{
        drv_info.useEdd = true;
	}
    else
	{
        drv_info.useEdd = false;
	}
    
    // Get logical geometry
    eax = 0x0800;
    edx = drv_info.DriveId;
    BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if ((eax & 0xFF00) != 0) goto cleanUp;

	drv_info.usNumCyl = (((ecx & 0xFF00) >> 8)|((ecx & 0x00C0) << 2)) + 1;
	drv_info.usNumHeads = ((edx & 0xFF00) >> 8) + 1;
	drv_info.usSectorsPerTrack = (ecx & 0x003F);
    //m_drv_info.DriveId = (uint8_t)drv_info.DriveId;

    // Get additional info
    if (drv_info.useEdd)
	{
        pInfo = reinterpret_cast<BiosDriveParameters_t *>(BootBiosBuffer());
        eax = 0x4800;
        edx = drv_info.DriveId;
        esi = (uint16_t)pInfo;
        memset(pInfo, 0, sizeof(*pInfo));
        pInfo->size = sizeof(*pInfo);
        BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);

        drv_info.usSectorSize = pInfo->sectorSize;
        drv_info.dwTotalSectors = pInfo->sectorsLo; // | (pInfo->sectorsHi << 32);
	}
    else
	{
        drv_info.usSectorSize = 512;
        drv_info.dwTotalSectors = drv_info.usNumCyl * drv_info.usNumHeads * drv_info.usSectorsPerTrack;
	}
cleanUp:
	return TRUE;
}
//------------------------------------------------------------------------------

BOOL Lba2Chs(
    size_t sector,
    uint8_t &c,
    uint8_t &h,
    uint8_t &s
    )
{
    bool_t rc = false;
    uint32_t sectorsPerCylinder, cylinder;

    c = h = s = 0xFF;
    uint32_t sectors = drv_info.usSectorsPerTrack * drv_info.usNumHeads * drv_info.usNumCyl;
    if (sector > sectors) goto cleanUp;

    sectorsPerCylinder = drv_info.usSectorsPerTrack * drv_info.usNumHeads;
    cylinder = sector / sectorsPerCylinder;
    sector -= cylinder * sectorsPerCylinder;
    h = (uint8_t)(sector / drv_info.usSectorsPerTrack);
    sector -= h * drv_info.usSectorsPerTrack;
    s = (uint8_t)((sector + 1) | ((cylinder >> 2) & 0xC0));
    c = (uint8_t)(cylinder & 0xFF);

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
typedef struct BiosDeviceAddressPacket_t {
    uint16_t size;
    uint16_t blocks;
    uint16_t bufferPtr[2];
    uint32_t blockLo;
    uint32_t blockHi;
} BiosDeviceAddressPacket_t;


BOOL BootDisk_Read(
    size_t sector,
    size_t sectors,
    uint8_t *pBuffer
    )
{
    BOOL rc = FALSE;
    BiosDeviceAddressPacket_t *pPacket;
    uint32_t eax, ebx, ecx, edx, esi, edi;

    pPacket = (BiosDeviceAddressPacket_t *)(BootBiosBuffer());
    
    while (sectors > 0)
        {
        if (drv_info.useEdd)
            {
            // Prepare address packet
            pPacket->size = sizeof(*pPacket);
            pPacket->blocks = (uint16_t)(sectors > 8 ? 8 : sectors);
            pPacket->bufferPtr[0] = (uint16_t)&pPacket[1];
            pPacket->bufferPtr[1] = 0;
            pPacket->blockLo = sector;
            pPacket->blockHi = 0;

            // Call Bios EDD Services
            eax = 0x4200;
            edx = drv_info.DriveId;
            esi = (uint16_t)pPacket;
            BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
            if ((eax & 0xFF00) != 0) goto cleanUp;

            // Copy data
            size_t size = pPacket->blocks * drv_info.usSectorSize;
            memcpy(pBuffer, &pPacket[1], size);
            pBuffer += size;
            sector += pPacket->blocks;
            sectors -= pPacket->blocks;
            
            }
        else
            {
            uint8_t c, h, s;

            // Translate LBA to CHS
            if (!Lba2Chs(sector, c, h, s)) goto cleanUp;

            // Call normal read (1 sector)
            eax = 0x0201;
            ebx = (uint16_t)&pPacket[1];
            ecx = s | (c << 8);
            edx = drv_info.DriveId | (h << 8);

            BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
            if ((eax & 0xFF00) != 0) goto cleanUp;

            // Copy data
            memcpy(pBuffer, &pPacket[1], drv_info.usSectorSize);
            pBuffer += drv_info.usSectorSize;
            sector++;
            sectors--;
            }
        }
    
    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

BOOL BootDisk_Write (
    size_t sector,
    size_t sectors,
    uint8_t *pBuffer
    )
{
    bool rc = false;
    BiosDeviceAddressPacket_t *pPacket;
    uint32_t eax, ebx, ecx, edx, esi, edi;

    pPacket = (BiosDeviceAddressPacket_t *)(BootBiosBuffer());

    while (sectors > 0)
        {
        if (drv_info.useEdd)
            {
            // Prepare address packet
            pPacket->size = sizeof(*pPacket);
            pPacket->blocks = (uint16_t)(sectors > 8 ? 8 : sectors);
            pPacket->bufferPtr[0] = (uint16_t)&pPacket[1];
            pPacket->bufferPtr[1] = 0;
            pPacket->blockLo = sector;
            pPacket->blockHi = 0;

            // Copy data
            size_t size = pPacket->blocks * drv_info.usSectorSize;
            memcpy(&pPacket[1], pBuffer, size);

            // Call Bios EDD Services
            eax = 0x4300;
            edx = drv_info.DriveId;
            esi = (uint32_t)pPacket;
            BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
            if ((eax & 0xFF00) != 0) goto cleanUp;

            pBuffer += size;
            sector += pPacket->blocks;
            sectors -= pPacket->blocks;
            
            }
        else
            {
            uint8_t c, h, s;

            // Translate LBA to CHS
            if (!Lba2Chs(sector, c, h, s)) goto cleanUp;

            // Copy data
            memcpy(&pPacket[1], pBuffer, drv_info.usSectorSize);

            // Call normal write
            eax = 0x0301;
            ebx = (uint16_t)&pPacket[1];
            ecx = s | (c << 8);
            edx = drv_info.DriveId | (h << 8);
            BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
            if ((eax & 0xFF00) != 0) goto cleanUp;

            pBuffer += drv_info.usSectorSize;
            sector++;
            sectors--;
            }
        }
    
    // Done
    rc = true;

cleanUp:
    return rc;
}

////////////////////

//
//
////
//// BIOS functions interface. See bios.asm for implementation.
////
////BYTE BIOS_ReadSectors(UCHAR Drive, USHORT c, UCHAR h, UCHAR s, USHORT nSectors, PUCHAR pBuffer);
//BYTE BIOS_ResetDisk(UCHAR Drive);
//
//
//
//
// Read the specified number of sectors from the Drive/LBA combination specified.
//
BOOLEAN ReadSectors(UCHAR Drive, ULONG LBA, USHORT nSectors, PUCHAR pBuffer)
{
	return BootDisk_Read (LBA, nSectors, pBuffer);

}
////
//// BIOS INT13H error number-to-text mapping.
////
//#ifdef FULLINT13MESSAGES
//struct
//{
//	BYTE   nErrVal;
//    LPCSTR pErrText;
//} DiskErrs[] = 
//{ 
//  { 0x01, "Invalid function in AH or invalid parameter" },
//  { 0x02, "Address mark not found" },
//  { 0x03, "Disk write-protected" },
//  { 0x04, "Sector not found/read error" },
//  { 0x05, "Reset failed (hard disk)" },
//  { 0x06, "Disk changed (floppy)" },
//  { 0x07, "Drive parameter activity failed (hard disk)" },
//  { 0x08, "DMA overrun" },
//  { 0x09, "Data boundary error (attempted DMA across a 64K boundary)" },
//  { 0x0A, "Bad sector detected (hard disk)" },
//  { 0x0B, "Bad track detected (hard disk)" },
//  { 0x0C, "Unsupported track or invalid media" },
//  { 0x0D, "Invalid number of sectors on format (PS/2 hard disk)" },
//  { 0x0E, "Control data address mark detected (hard disk)" },
//  { 0x0F, "DMA arbitration level out of range (hard disk)" },
//  { 0x10, "Uncorrectable CRC or ECC error on read" },
//  { 0x11, "Data ECC corrected (hard disk)" },
//  { 0x20, "Controller failure" },
//  { 0x31, "No media in drive (IBM/MS INT 13 extensions)" },
//  { 0x32, "Incorrect drive type stored in CMOS" },
//  { 0x40, "Seek failed" },
//  { 0x80, "Timeout (not ready)" },
//  { 0xAA, "Drive not ready (hard disk)" },
//  { 0xB0, "Volume not locked in drive (INT 13 extensions)" },
//  { 0xB1, "Volume locked in drive (INT 13 extensions)" },
//  { 0xB2, "Volume not removable (INT 13 extensions)" },
//  { 0xB3, "Volume in use (INT 13 extensions)" },
//  { 0xB4, "Lock count exceeded (INT 13 extensions)" },
//  { 0xB5, "Valid eject request failed (INT 13 extensions)" },
//  { 0xB6, "Volume present but read protected (INT 13 extensions)" },
//  { 0xBB, "Undefined error (hard disk)" },
//  { 0xCC, "Write fault (hard disk)" },
//  { 0xE0, "Status register error (hard disk)" },
//  { 0xFF, "Sense operation failed (hard disk)" },
//  {    0, "<unknown>" } 
//};
//#endif
//
////
//// Convert the error value returned by the BIOS INT13h call into a text message.
////
//#ifdef FULLINT13MESSAGES
//
//LPCSTR GetErrorText(BYTE nErrVal)
//{
//	BYTE nCount;
//
//	for (nCount = 0 ; DiskErrs[nCount].nErrVal ; nCount++)
//	{
//		if (DiskErrs[nCount].nErrVal == nErrVal)
//			break;
//	}
//
//	return(DiskErrs[nCount].pErrText);
//}
//#endif

