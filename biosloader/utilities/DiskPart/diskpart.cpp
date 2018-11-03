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

#include <stdio.h>
#include <SimpleString.h>
#include <StoreMgr.h>
#include <Diskio.h>
#include <intsafe.h>
#include "mbr.h"
#include "diskpart.h"
#include "fatutil.h"

//
// The globals are state data.  Which disk is selected, which partition, etc.
//
ULONG g_Disk;
ULONG g_Partition;
HANDLE g_hDisk;
HANDLE g_hPart;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
    CSimpleString strScriptFile;
    FILE* pInput = NULL;
    int nReturn = 0;
    bool fExit = false;

    CSimpleString strCommand;
    CSimpleString strOp;
    CSimpleString strParams;

    g_Disk = 0;
    g_Partition = 0;
    g_hDisk = INVALID_HANDLE_VALUE;
    g_hPart = INVALID_HANDLE_VALUE;

    if( !ParseCommandLine( lpCmdLine, &strScriptFile ) )
    {
        PrintCommandLineHelp();
        return 1;
    }

    if( strScriptFile.GetString() )
    {
        pInput = _tfopen( strScriptFile.GetString(), _T("r") );
    }
    else
    {
        pInput = stdin;
    }

    if( !pInput )
    {
        nReturn = 1;
        _tprintf( _T("Unable to open file: %s\n"),
                  pInput == stdin ? _T("stdin")
                                  : strScriptFile.GetString() );
        goto exit_winmain;
    }

    if( !strCommand.AllocateString( 128 ) ||
        !strOp.AllocateString( 50 ) ||
        !strParams.AllocateString( 100 ) )
    {
        nReturn = 1;
        _tprintf( _T("Unable to allocate command buffer.\n") );
        goto exit_winmain;
    }

    DisplayHeader();
    DisplayPrompt();
    while( GetCommand( pInput, &strCommand ) &&
           _tcsicmp( strCommand.GetString(), _T("Exit") ) )
    {
        if( pInput != stdin )
        {
            _tprintf( _T("%s"), strCommand.GetString() );
        }

        if( !ParseCommand( &strCommand, &strOp, &strParams ) )
        {
            goto exit_winmain;
        }

        _tprintf( _T("\n") );

        ExecCommand( &strOp, &strParams );

        ZeroMemory( strCommand.GetString(), strCommand.GetBufSizeInBytes() );
        ZeroMemory( strOp.GetString(), strOp.GetBufSizeInBytes() );
        ZeroMemory( strParams.GetString(), strParams.GetBufSizeInBytes() );

        DisplayPrompt();
    }

exit_winmain:
    if( pInput )
    {
        fclose( pInput );
    }

    return nReturn;
}

bool ParseCommandLine( TCHAR* pCmdLine, CSimpleString* strScriptFile )
{
    TCHAR* Delimiters = _T(" \t");
    TCHAR* strTok = NULL;
    bool fExpectScriptFile = false;

    strTok = _tcstok( pCmdLine, Delimiters );
    while( strTok != NULL )
    {
        if( fExpectScriptFile )
        {
            fExpectScriptFile = false;

            if( !strScriptFile->AllocateString( _tcsclen( strTok ) + 1 ) )
            {
                _tprintf( _T("Out of memory - aborting.\n") );
                return false;
            }

            if( !SUCCEEDED( StringCchCopy( strScriptFile->GetString(),
                                           strScriptFile->GetBufSizeInChars(),
                                           strTok ) ) )
            {
                _tprintf( _T("String copy failed: %d\n"), GetLastError() );
                return false;
            }

            strTok = _tcstok( NULL, Delimiters );
            continue;
        }

        if( strTok[0] != _T('-') &&
            strTok[0] != _T('/') )
        {
            _tprintf( _T("Invalid command line option: %s\n"), strTok );
            return false;
        }

        if( _tcsicmp( &strTok[1], _T("s") ) == 0 )
        {
            fExpectScriptFile = true;
            strTok = _tcstok( NULL, Delimiters );
            continue;
        }

        _tprintf( _T("Invalid command line option: %s\n"), strTok );
        return false;
    }

    if( fExpectScriptFile )
    {
        _tprintf( _T("Expecting a script file name.\n") );
        return false;
    }

    return true;
}

void PrintCommandLineHelp()
{
    _tprintf( _T("DiskPart.exe [-s file]\n\n") );
    _tprintf( _T("DiskPart can be used to partition, clean, or repartition a disk drive.\n") );
    _tprintf( _T("-s file\t DiskPart runs on the script file provided.\n") );
}

// /////////////////////////////////////////////////////////////////////////////
// SizeIsZero
//
// When strtoul returns zero, we need to check if it is really zero, or if an
// error occured.  Checking that each character in the string is '0' will tell
// us.
//
bool SizeIsZero( const TCHAR* strSize )
{
    size_t ulLength = 0;

    if( FAILED(StringCchLength( strSize,
                                MAX_PATH,
                                &ulLength )) )
    {
        return false;
    }

    //
    // Was it an error, or was 0 really selected?
    //
    for( size_t x = 0; x < ulLength; x++ )
    {
        if( strSize[x] != _T('0') )
        {
            return false;
        }
    }

    return true;
}

// /////////////////////////////////////////////////////////////////////////////
// ConvertLBAtoCHS
//
// Converts the LBA sector number to cylinder, head, sector geometry that's
// used for the partition table entries.
//
bool ConvertLBAtoCHS( const STOREINFO& StoreInfo,
                      DWORD dwSector,
                      BYTE* pCyl,
                      BYTE* pHead,
                      BYTE* pSector )
{
    DWORD dwResult = ERROR_SUCCESS;
    DWORD dwReturned = 0;
    DWORD sectorNum;
    DWORD head, cyl, sector, sectorsPerCyl;
    DISK_INFO DiskInfo = { 0 };

    if( !DeviceIoControl( g_hDisk,
                          IOCTL_DISK_GETINFO,
                          NULL,
                          0,
                          &DiskInfo,
                          sizeof(DISK_INFO),
                          &dwReturned,
                          NULL ) )
    {
        return false;
    }

    sectorNum = dwSector;

    // be sure the requested sector is within the capacity of the drive
    if( sectorNum > StoreInfo.snNumSectors )
    {
        return false;
    }

    //
    // TODO::What to do when this return 0 heads on the hard drive.  Making it
    // 1 for now.
    //

    if( DiskInfo.di_heads && DiskInfo.di_sectors )
    {
        sectorsPerCyl = DiskInfo.di_heads * DiskInfo.di_sectors;

        cyl = sectorNum / sectorsPerCyl;

        head = (sectorNum-(cyl * sectorsPerCyl)) / DiskInfo.di_sectors;

        sector = sectorNum - ((cyl * sectorsPerCyl) + (head * DiskInfo.di_sectors)) + 1;

        *pHead = (BYTE)head;
        *pCyl = (BYTE)cyl;

        // the sector # is actually 6 bits
        *pSector = (BYTE)sector & 0x3F;

        // the cylinder # is actually 10 bits and the upper 2 bits goes in the upper 2 bits of the sector #
        *pSector |= (cyl & 0x0300) >> 2;
    }
    else
    {
        *pHead = 0;
        *pCyl = 0;
        *pSector = 0;
    }

    return true;
}

// /////////////////////////////////////////////////////////////////////////////
// ComputeBootSectorChecksum (From exFAT)
//
DWORD ComputeBootSectorChecksum( BYTE* pBuffer, DWORD dwSize )
{
    DWORD Checksum =0;

    for( DWORD i = 0; i < dwSize; i++ )
    {
        // Skip the ExtendedFlags and PercentInUse field from the boot sector
        // in the checksum, since this will be modified
        //
        if( (i == 107) ||  // Extended Flags (Byte 0)
            (i == 108) ||  // Extended Flags (Byte 1)
            (i == 112) )   // Percent in use
        {
            continue;
        }

        Checksum = ((Checksum & 1) ? 0x80000000 : 0) +
                    (Checksum >> 1) +
                    pBuffer[i];
    }

    return Checksum;
}

bool GetCommand( FILE* pInput, CSimpleString* strCommand )
{
    size_t Length = 0;
    TCHAR* strTemp = strCommand->GetString();

    if( !_fgetts( strCommand->GetString(),
                  strCommand->GetBufSizeInChars(),
                  pInput ) )
    {
        return false;
    }

    if( FAILED(StringCchLength( strCommand->GetString(),
                                strCommand->GetBufSizeInChars(),
                                &Length )) )
    {
        return false;
    }

    //
    // Remove the new line character.
    //
    strTemp[Length - 1] = NULL;

    return true;
}

bool ParseCommand( CSimpleString* strCommand,
                   CSimpleString* strOp,
                   CSimpleString* strParams )
{
    TCHAR* strDelims = _T(" \t\n");
    TCHAR* strTok = _tcstok( strCommand->GetString(),
                             strDelims );

    if( !strTok )
    {
        return false;
    }

    StringCchCopy( strOp->GetString(),
                   strOp->GetBufSizeInChars(),
                   strTok );

    while( (strTok = _tcstok( NULL, strDelims )) != NULL  )
    {
        if( !strParams->Append( strTok ) ||
            !strParams->Append( _T(" ") ) )
        {
            _tprintf( _T("The operation parameters are an invalid length: %s"), strCommand->GetString() );
            return false;
        }
    }

    return true;
}

bool ExecCommand( CSimpleString* strOp, CSimpleString* strParams )
{
    if( _tcsicmp( strOp->GetString(), _T("ACTIVE") ) == 0 )
    {
        return ActivatePartition( strParams );
    }
    else if( _tcsicmp( strOp->GetString(), _T("INACTIVE") ) == 0 )
    {
        return DeActivatePartition();
    }
    else if( _tcsicmp( strOp->GetString(), _T("CLEAN") ) == 0 )
    {
        return CleanDisk();
    }
    else if( _tcsicmp( strOp->GetString(), _T("CREATE") ) == 0 )
    {
        return CreatePartition( strParams );
    }
    else if( _tcsicmp( strOp->GetString(), _T("SETPARTITIONTYPE") ) == 0 )
    {
        return SetPartitionType( strParams );
    }
    else if( _tcsicmp( strOp->GetString(), _T("FORMATPARTITION") ) == 0 )
    {
        return FormatPartition( strParams );
    }
    else if( _tcsicmp( strOp->GetString(), _T("DELETE") ) == 0 )
    {
        return DeletePartition();
    }
    else if( _tcsicmp( strOp->GetString(), _T("HELP") ) == 0 )
    {
        return PrintHelp( strParams );
    }
    else if( _tcsicmp( strOp->GetString(), _T("LIST") ) == 0 )
    {
        return ListObjects( strParams );
    }
    else if( _tcsicmp( strOp->GetString(), _T("SELECT") ) == 0 )
    {
        return SelectObject( strParams );
    }
    else if( _tcsicmp( strOp->GetString(), _T("DUMPMBR") ) == 0 )
    {
        return DumpMBR();
    }
    else if( _tcsicmp( strOp->GetString(), _T("DUMPSTORE") ) == 0 )
    {
        return DumpStoreInfo();
    }
    else if( _tcsicmp( strOp->GetString(), _T("DUMPPART") ) == 0 )
    {
        return DumpPartInfo();
    }
    else if( _tcsicmp( strOp->GetString(), _T("ZERO") ) == 0 )
    {
        return ZeroSector( strParams );
    }
    else if( _tcsicmp( strOp->GetString(), _T("DUMPSECTOR") ) == 0 )
    {
        return DumpSector( strParams );
    }
    else if( _tcsicmp( strOp->GetString(), _T("MBRCODE") ) == 0 )
    {
        return WriteMBRCode( strParams );
    }
    else if( _tcsicmp( strOp->GetString(), _T("BOOTSEC") ) == 0 )
    {
        return WriteBootSector( strParams );
    }
    else if( _tcsicmp( strOp->GetString(), _T("FIXPARTOFFSET") ) == 0 )
    {
        return FixPartOffset();
    }
    else if( _tcsicmp( strOp->GetString(), _T("FIXCRC") ) == 0 )
    {
        return FixCRC();
    }
    else
    {
        _tprintf( _T("Unknown command: %s\n"), strOp->GetString() );
        return false;
    }
}

DWORD GetNumberOfDisks( ULONG* ulNumDisks )
{
    STOREINFO StoreInfo = { 0 };
    HANDLE hFind = INVALID_HANDLE_VALUE;
    ULONG ulCount = 1;

    StoreInfo.cbSize = sizeof(STOREINFO);

    hFind = FindFirstStore( &StoreInfo );
    if( hFind == INVALID_HANDLE_VALUE )
    {
        return GetLastError();
    }

    while( FindNextStore( hFind, &StoreInfo ) )
    {
        ulCount++;
    }

    *ulNumDisks = ulCount;

    FindCloseStore( hFind );

    return ERROR_SUCCESS;
}

HANDLE GetDisk( STOREINFO* pStoreInfo )
{
    return OpenStore( pStoreInfo->szDeviceName );
}

bool GetDiskInfo( ULONG ulDisk, STOREINFO* pStoreInfo )
{
    bool fResult = false;
    ULONG ulNumDisks = 0;
    DWORD dwResult = ERROR_SUCCESS;
    STOREINFO StoreInfo = { 0 };
    HANDLE hFind = INVALID_HANDLE_VALUE;
    TCHAR DiskName[32] = { 0 };

    if( !pStoreInfo )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        fResult = false;
        goto exit_getdiskinfo;
    }

    StoreInfo.cbSize = sizeof(STOREINFO);

    hFind = FindFirstStore( &StoreInfo );
    if( hFind == INVALID_HANDLE_VALUE )
    {
        goto exit_getdiskinfo;
    }

    ULONG nDisk = 0;

    do
    {
        if( StoreInfo.sdi.dwDeviceClass == STORAGE_DEVICE_CLASS_BLOCK )
        {
            if( (StoreInfo.dwAttributes & STORE_ATTRIBUTE_REMOVABLE) == 0 )
            {
                nDisk += 1;
                if( nDisk == ulDisk )
                {
                    CopyMemory( pStoreInfo, &StoreInfo, sizeof(StoreInfo) );
                    fResult = true;
                    break;
                }
            }
        }

    } while( FindNextStore( hFind, &StoreInfo ) );

exit_getdiskinfo:
    if( hFind != INVALID_HANDLE_VALUE )
    {
        FindCloseStore( hFind );
    }

    return fResult;
}

bool SelectDisk( ULONG ulDisk )
{
    STOREINFO StoreInfo = { 0 };
    HANDLE hDisk = INVALID_HANDLE_VALUE;
    bool fResult = false;

    if( !GetDiskInfo( ulDisk, &StoreInfo ) )
    {
        goto exit_selectdisk;
    }

    //
    // Do accept ATAPI devices.
    // This might not be restrictive enough.
    //
    if( StoreInfo.sdi.dwDeviceClass != STORAGE_DEVICE_CLASS_BLOCK )
    {
        goto exit_selectdisk;
    }

    hDisk = GetDisk( &StoreInfo );
    if( hDisk != INVALID_HANDLE_VALUE )
    {
        if( g_hDisk != INVALID_HANDLE_VALUE )
        {
            CloseHandle( g_hDisk );
        }

        if( g_hPart != INVALID_HANDLE_VALUE )
        {
            CloseHandle( g_hPart );
            g_hPart = INVALID_HANDLE_VALUE;
        }

        g_Partition = 0;

        g_hDisk = hDisk;
        g_Disk = ulDisk;
        fResult = true;
    }

exit_selectdisk:
    return fResult;
}

DWORD GetNumberOfPartitions( ULONG* ulNumParts )
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    PARTINFO PartInfo = { 0 };
    ULONG ulCount = 1;

    PartInfo.cbSize = sizeof(PartInfo);

    hFind = FindFirstPartition( g_hDisk, &PartInfo );
    if( hFind == INVALID_HANDLE_VALUE )
    {
        return GetLastError();
    }

    while( FindNextPartition( hFind, &PartInfo ) )
    {
        ulCount++;
    }

    *ulNumParts = ulCount;

    FindClosePartition( hFind );

    return ERROR_SUCCESS;
}

HANDLE GetPartition( ULONG ulPart )
{
    DWORD dwResult = ERROR_SUCCESS;
    ULONG ulNumParts = 0;
    HANDLE hPart = INVALID_HANDLE_VALUE;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    PARTINFO PartInfo = { 0 };

    dwResult = GetNumberOfPartitions( &ulNumParts );
    if( dwResult != ERROR_SUCCESS )
    {
        SetLastError( dwResult );
        goto exit_getpart;
    }

    if( ulPart >= ulNumParts )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto exit_getpart;
    }

    PartInfo.cbSize = sizeof(PARTINFO);

    hFind = FindFirstPartition( g_hDisk, &PartInfo );
    if( hFind == INVALID_HANDLE_VALUE )
    {
        goto exit_getpart;
    }

    for( ULONG Part = 0; Part < ulPart; Part++ )
    {
        if( !FindNextPartition( hFind, &PartInfo ) )
        {
            goto exit_getpart;
        }
    }

    hPart = OpenPartition( g_hDisk, PartInfo.szPartitionName );
    if( hPart == INVALID_HANDLE_VALUE )
    {
        goto exit_getpart;
    }

exit_getpart:
    if( hFind != INVALID_HANDLE_VALUE )
    {
        FindClosePartition( hFind );
    }
    return hPart;
}

bool SelectPartition( ULONG ulPart )
{
    HANDLE hPart = INVALID_HANDLE_VALUE;
    bool fResult = false;

    hPart = GetPartition( ulPart );
    if( hPart != INVALID_HANDLE_VALUE )
    {
        if( g_hPart != INVALID_HANDLE_VALUE )
        {
            CloseHandle( g_hPart );
            g_hPart = INVALID_HANDLE_VALUE;
        }

        g_hPart = hPart;
        g_Partition = ulPart;
        fResult = true;
    }

    return fResult;
}

void PrintDiskHeader()
{
    _tprintf( _T("  Disk ###\tDevice Name\n") );
    _tprintf( _T("  --------\t-----------\n") );
}

void PrintDisk( ULONG ulDisk, const STOREINFO& StoreInfo )
{
    _tprintf( _T("%c Disk %3d\t%s\n"),
              g_hDisk != INVALID_HANDLE_VALUE && g_Disk == ulDisk ? _T('*') : _T(' '),
              ulDisk,
              StoreInfo.szDeviceName );
}

void PrintPartitions( BYTE* pSector, const STOREINFO& StoreInfo )
{
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    USHORT usEntries = 0;

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                       - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pSector + dwTableOffset);

    _tprintf( _T("  Partition ###\t    Type\tStarting Sector\t     Sectors\n") );
    _tprintf( _T("  -------------\t--------\t---------------\t------------\n") );

    for( USHORT x = 0; x < MAX_PARTTABLE_ENTRIES; x++ )
    {
        const TCHAR* strPartType = _T("Primary");

        if( pTable[x].Part_StartSector )
        {
            if( pTable[x].Part_FileSystem == PART_EXTENDED ||
                pTable[x].Part_FileSystem == PART_DOSX13X )
            {
                strPartType= _T("Extended");
            }

            _tprintf( _T("%c Partition %3d\t%8s\t%15d\t%12d\n"),
                      g_hPart != INVALID_HANDLE_VALUE &&
                        g_Partition == x ? _T('*') : _T(' '),
                      x,
                      strPartType,
                      pTable[x].Part_StartSector,
                      pTable[x].Part_TotalSectors );
        }

        if( pTable[x].Part_FileSystem == PART_EXTENDED ||
            pTable[x].Part_FileSystem == PART_DOSX13X )
        {
            _tprintf( _T("\tTODO::Fill in logical disks here.\n") );
        }
    }
}

USHORT EntriesInUse( BYTE* pSector, const STOREINFO& StoreInfo )
{
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    USHORT x = 0;

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                        - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pSector + dwTableOffset);

    while( pTable[x].Part_StartSector && x < MAX_PARTTABLE_ENTRIES )
    {
        x++;
    }

    return x;
}

bool SectorIsFree( DWORD dwSector, BYTE* pMBR, const STOREINFO& StoreInfo )
{
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    USHORT usEntries = 0;
    USHORT x = 0;

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                        - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pMBR + dwTableOffset);

    while( pTable[x].Part_StartSector )
    {
        if( dwSector >= pTable[x].Part_StartSector &&
            dwSector < pTable[x].Part_StartSector + pTable[x].Part_TotalSectors )
        {
            return false;
        }

        x += 1;
    }

    return true;
}

DWORD GetLargestPossiblePartition( BYTE* pMBR, const STOREINFO& StoreInfo )
{
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    DWORD LargeSize = 0;
    DWORD dwCurSector = 1;
    USHORT usPart = 0;

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                        - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pMBR + dwTableOffset);

    while( pTable[usPart].Part_StartSector && usPart < MAX_PARTTABLE_ENTRIES )
    {
        if( pTable[usPart].Part_StartSector > dwCurSector &&
            pTable[usPart].Part_StartSector - dwCurSector > LargeSize )
        {
            LargeSize = pTable[usPart].Part_StartSector - dwCurSector;
            dwCurSector = pTable[usPart].Part_StartSector + pTable[usPart].Part_TotalSectors;
        }

        usPart += 1;
    }

    if( dwCurSector < StoreInfo.snNumSectors &&
        StoreInfo.snNumSectors - dwCurSector > LargeSize )
    {
        LargeSize = (DWORD)StoreInfo.snNumSectors - dwCurSector;
    }

    return LargeSize;
}

bool GenerateMBREntry( PPARTENTRY pEntry,
                       const STOREINFO& StoreInfo,
                       DWORD dwSector,
                       DWORD dwNumSectors )
{
    pEntry->Part_BootInd = 0;
    pEntry->Part_FileSystem = PART_DOS32;
    pEntry->Part_StartSector = dwSector;
    pEntry->Part_TotalSectors = dwNumSectors;

    if( !ConvertLBAtoCHS( StoreInfo,
                          dwSector,
                          &pEntry->Part_FirstTrack,
                          &pEntry->Part_FirstHead,
                          &pEntry->Part_FirstSector ) )
    {
        _tprintf( _T("Unable to create partition.\n") );
        return false;
    }

    if( !ConvertLBAtoCHS( StoreInfo,
                          pEntry->Part_StartSector + pEntry->Part_TotalSectors - 1,
                          &pEntry->Part_LastTrack,
                          &pEntry->Part_LastHead,
                          &pEntry->Part_LastSector ) )
    {
        _tprintf( _T("Unable to create partition.\n") );
        return false;
    }

    return true;
}

bool InsertPartition( BYTE* pMBR,
                      const STOREINFO& StoreInfo,
                      DWORD dwStart,
                      DWORD dwNumSectors )
{
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    DWORD dwCurSector = 1;
    USHORT usPart = 0;
    DWORD dwMinSize = dwNumSectors;

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                        - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pMBR + dwTableOffset);

    //
    // Impose a minimum size of 5MB.
    //
    if( dwMinSize < ( 5 * 1024 * 1024 ) / StoreInfo.dwBytesPerSector )
    {
        dwMinSize = ( 5 * 1024 * 1024 ) / StoreInfo.dwBytesPerSector;

        if( dwNumSectors )
        {
            dwNumSectors = dwMinSize;
        }
    }

    //
    // If there is currently no entry, then just put this one at the beginning.
    //
    if( pTable[0].Part_StartSector == 0 )
    {
        return GenerateMBREntry( &pTable[0],
                                 StoreInfo,
                                 dwStart ? dwStart : 1,
                                 dwNumSectors
                                     ? dwMinSize
                                     : (DWORD)StoreInfo.snNumSectors - 1 );
    }

    //
    // Each time through this loop we will check to see if the space
    // between the end of the last entry (value kept in dwCurSector)
    // and the start of this one is large enough for the desired partition
    // to be created.  dwCurSector is initialized to 1, as this is the first
    // available sector on the disk due to the MBR.
    //
    // There has to be a free entry, so we shouldn't have to go past
    // the first 3.  We've already validated that there is an empty
    // entry to use.
    //
    while( pTable[usPart].Part_StartSector && usPart < 3 )
    {
        //
        // This should never happened.  We've already verified that the
        // partition should fit on the disk and that it doesn't fall in
        // another partition.
        //
        if( dwStart != 0 &&
            pTable[usPart].Part_StartSector > dwStart &&
            dwCurSector > dwStart )
        {
            _tprintf( _T("Diskpart cannot create the partition at the given location.\n") );
            return false;
        }

        //
        // If no start was specified and we meet the size requirements
        // then use this location.
        //
        if( dwStart == 0 &&
            pTable[usPart].Part_StartSector > dwCurSector &&
            pTable[usPart].Part_StartSector - dwCurSector >= dwMinSize )
        {
            MoveMemory( &pTable[usPart + 1],
                        &pTable[usPart],
                        (3 - usPart) * sizeof(PARTENTRY) );

            return GenerateMBREntry( &pTable[usPart],
                                     StoreInfo,
                                     dwCurSector,
                                     dwNumSectors
                                       ? dwMinSize
                                       : pTable[usPart+1].Part_StartSector - dwCurSector );
        }

        if( dwStart &&
            pTable[usPart].Part_StartSector > dwStart )
        {
            MoveMemory( &pTable[usPart + 1],
                        &pTable[usPart],
                        (3 - usPart) * sizeof(PARTENTRY) );

            return GenerateMBREntry( &pTable[usPart],
                                     StoreInfo,
                                     dwStart,
                                     dwNumSectors
                                       ? dwMinSize
                                       : pTable[usPart+1].Part_StartSector - dwCurSector );

        }

        dwCurSector = pTable[usPart].Part_StartSector +
                      pTable[usPart].Part_TotalSectors;

        usPart += 1;
    }

    //
    // We simply need to add an entry after the last one.
    //
    return GenerateMBREntry( &pTable[usPart],
                             StoreInfo,
                             dwStart ? dwStart : dwCurSector,
                             dwNumSectors
                                     ? dwMinSize
                                     : (DWORD)StoreInfo.snNumSectors - dwCurSector - 1 );

}

// -----------------------------------------------------------------------------
// Helper functions to act on the MBR.
// -----------------------------------------------------------------------------

// /////////////////////////////////////////////////////////////////////////////
// CompactMBR
//
// Make sure there the MBR is not fragmented - removing any empty entries
// between two valid entries in the MBR's partition table.
//
void CompactMBR( BYTE* pMBR, const STOREINFO& StoreInfo )
{
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    DWORD LargeSize = 0;
    USHORT Partition = 0;

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                        - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pMBR + dwTableOffset);

    for( USHORT x = 0; x < 3; x++ )
    {
        if( pTable[Partition].Part_StartSector != 0 )
        {
            Partition++;
        }
        else
        {
            MoveMemory( &pTable[Partition],
                        &pTable[Partition + 1],
                        (3 - x) * sizeof(PARTENTRY) );
            ZeroMemory( &pTable[Partition + 3 - x], sizeof(PARTENTRY) );
        }
    }
}

// /////////////////////////////////////////////////////////////////////////////
// MBRIsValid
//
// This will check that the partition entries are written in sequential order
// according to their starting sector and that there is only one extended
// partition on the disk.
//
bool MBRIsValid( BYTE* pSector, const STOREINFO& StoreInfo )
{
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    USHORT usEntries = 0;
    USHORT usNumExParts = 0;

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                        - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pSector + dwTableOffset);

    for( int x = 0; x < 3; x++ )
    {
        if( !pTable[x].Part_StartSector )
        {
            continue;
        }

        for( int y = x + 1; y < MAX_PARTTABLE_ENTRIES; y++ )
        {
            if( !pTable[y].Part_StartSector )
            {
                continue;
            }

            if( pTable[x].Part_StartSector > pTable[y].Part_StartSector )
            {
                return false;
            }
        }
    }

    for( USHORT x = 0; x < MAX_PARTTABLE_ENTRIES; x++ )
    {
        if( pTable[x].Part_FileSystem == PART_EXTENDED ||
            pTable[x].Part_FileSystem == PART_DOSX13X )
        {
            usNumExParts++;
        }
    }

    if( usNumExParts > 1 )
    {
        return false;
    }

    return true;
}

// /////////////////////////////////////////////////////////////////////////////
// ReadMBR
//
// This function actually has three tasks - bad code I realize but it works all
// the same.
//
// 1. Retrieve the STOREINFO about the disk from the storage manager.
// 2. Allocate memory for the sector based on the sector size in the STOREINFO.
// 3. Read in the first sector from the current disk.
//
// The store info and the sector are both returned by this function.
//
bool ReadMBR( BYTE** ppSector, STOREINFO* pStoreInfo )
{
    bool fResult = true;
    BYTE* pSector = NULL;
    STOREINFO StoreInfo = { 0 };
    SG_REQ IoInfo = { 0 };
    DWORD dwBytesRead = 0;

    StoreInfo.cbSize = sizeof(STOREINFO);
    if( !GetStoreInfo( g_hDisk, &StoreInfo ) )
    {
        _tprintf( _T("Unable to collect disk information: %d\n"), GetLastError() );
        fResult = false;
        goto exit_readmbr;
    }

    pSector = new BYTE[StoreInfo.dwBytesPerSector];
    if( !pSector )
    {
        _tprintf( _T("Unable to allocate memory: %d\n"), GetLastError() );
        fResult = false;
        goto exit_readmbr;
    }

    IoInfo.sr_start = 0;
    IoInfo.sr_num_sec = 1;
    IoInfo.sr_num_sg = 1;
    IoInfo.sr_sglist[0].sb_buf = pSector;
    IoInfo.sr_sglist[0].sb_len = StoreInfo.dwBytesPerSector;

    if( !DeviceIoControl( g_hDisk,
                          IOCTL_DISK_READ,
                          &IoInfo,
                          sizeof(IoInfo),
                          NULL,
                          0,
                          &dwBytesRead,
                          NULL ) )
    {
        _tprintf( _T("Unable to read from the disk: %d\n"), GetLastError() );
        fResult = false;
        goto exit_readmbr;
    }

exit_readmbr:
    if( !fResult && pSector )
    {
        delete [] pSector;
    }
    else if( pSector )
    {
        *ppSector = pSector;
        CopyMemory( pStoreInfo, &StoreInfo, sizeof(StoreInfo) );
    }

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// WriteMBR
//
// Write the given sector data to disk.  Make sure that the signature is tacked
// on at the end of the sector.
//
bool WriteMBR( BYTE* pSector, const STOREINFO& StoreInfo )
{
    bool fResult = true;
    SG_REQ IoInfo = { 0 };
    DWORD dwBytesWritten = 0;

    *(WORD *)(pSector + StoreInfo.dwBytesPerSector - 2) = BOOTSECTRAILSIGH;

    IoInfo.sr_start = 0;
    IoInfo.sr_num_sec = 1;
    IoInfo.sr_num_sg = 1;
    IoInfo.sr_sglist[0].sb_buf = pSector;
    IoInfo.sr_sglist[0].sb_len = StoreInfo.dwBytesPerSector;

    if( !DeviceIoControl( g_hDisk,
                          IOCTL_DISK_WRITE,
                          &IoInfo,
                          sizeof(IoInfo),
                          NULL,
                          0,
                          &dwBytesWritten,
                          NULL ) )
    {
        _tprintf( _T("Unable to write new MBR: %d\n"), GetLastError() );
        fResult = false;
        goto exit_writembr;
    }

exit_writembr:

    return fResult;
}

// -----------------------------------------------------------------------------
// Help function for the diskpart commands.
// -----------------------------------------------------------------------------

void PrintActiveHelp()
{
    _tprintf( _T("\nMicrosoft Windows CE DiskPart\n\n") );
    _tprintf( _T("ACTIVE [BootSector=filename] [BootLoader=filename]\n") );
    _tprintf( _T("BootSector\t-Write the file to the bootsector.\n") );
    _tprintf( _T("BootLoader\t-Copy the file to the root directory.\n") );
    _tprintf( _T("\n") );
}

void PrintSelectHelp()
{
    _tprintf( _T("\nMicrosoft Windows CE DiskPart\n\n") );
    _tprintf( _T("SELECT Disk|Partition\n") );
    _tprintf( _T("Disk\t\t-Move the focus to a disk, or see disk focus.\n") );
    _tprintf( _T("Partition\t-Move the focus to a partition, or see partition focus.\n") );
    _tprintf( _T("\n") );
}

void PrintListHelp()
{
    _tprintf( _T("\nMicrosoft Windows CE DiskPart\n\n") );
    _tprintf( _T("LIST Disk|Partition\n") );
    _tprintf( _T("Disk\t\t-Prints out a list of disks.\n") );
    _tprintf( _T("Partition\t-Prints out a list of partitions.\n") );
    _tprintf( _T("\n") );
}

void PrintCreateHelp()
{
    _tprintf( _T("\nMicrosoft Windows CE DiskPart\n\n") );
    _tprintf( _T("CREATE Primary|Extended|Logical [size=x] [offset=x]\n") );
    _tprintf( _T("Primary\t\t- Creates a primary partition.\n") );
    _tprintf( _T("Extended\t- Not currently supported.\n") );
    _tprintf( _T("Logical\t\t- Not currently supported.\n") );
    _tprintf( _T("Size\t\t- Specifies the size of the partition in MB\n") );
    _tprintf( _T("Offset\t\t- Specifies the starting sector\n") );
    _tprintf( _T("\n") );
}

void PrintMBRCodeHelp()
{
    _tprintf( _T("\nMicrosoft Windows CE DiskPart\n\n") );
    _tprintf( _T("MBRCODE File=file.dat\n") );
    _tprintf( _T("File\t-File containing the code to store in the MBR.\n") );
    _tprintf( _T("\n") );
}

void PrintZeroSectorHelp()
{
    _tprintf( _T("\nMicrosoft Windows CE DiskPart\n\n") );
    _tprintf( _T("ZEROSECTOR Sector=x\n") );
    _tprintf( _T("Sector\t-File containing the code to store in the MBR.\n") );
    _tprintf( _T("\n") );
}

void PrintDumpSectorHelp()
{
    _tprintf( _T("\nMicrosoft Windows CE DiskPart\n\n") );
    _tprintf( _T("DUMPSECTOR Sector=x\n") );
    _tprintf( _T("Sector\t-Sector number to write all zeroes to.\n") );
    _tprintf( _T("\n") );
}

void PrintWriteBootSectorHelp()
{
    _tprintf( _T("\nMicrosoft Windows CE DiskPart\n\n") );
    _tprintf( _T("BOOTSEC Sector=x File=filename\n") );
    _tprintf( _T("Sector\t-Sector number to write the code.\n") );
    _tprintf( _T("File\t-File containing the code to write.\n") );
    _tprintf( _T("\n") );
}

void PrintSetPartitionTypeHelp()
{
    _tprintf( _T("\nMicrosoft Windows CE DiskPart\n\n") );
    _tprintf( _T("FORMATPARTITION x\n") );
    _tprintf( _T("x\t-Partition type. (0-255, see bootpart.h for known types)\n") );
    _tprintf( _T("\n") );
}

// -----------------------------------------------------------------------------
// Command Functions
// -----------------------------------------------------------------------------

// /////////////////////////////////////////////////////////////////////////////
// ActivatePartition
//
// Will set the active partition flag on this partition and will clear this flag
// on all other partitions for this disk.
//
// Optionally a file name containing the boot sector and another file name
// for the boot loader can be passed to this function and both will be written
// to the selected partition.
//
bool ActivatePartition( CSimpleString* strParams )
{
    bool fResult = true;
    BYTE* pMBR = NULL;
    BYTE* pSector = NULL;
    STOREINFO StoreInfo = { 0 };
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    TCHAR* Delimiters = _T(" ");
    TCHAR* strTok = NULL;
    CSimpleString strBootSector;
    CSimpleString strBootLoader;
    SG_REQ IoInfo = { 0 };
    DWORD dwBytesWritten = 0;
    DWORD dwBytesRead = 0;
    size_t stLength = 0;

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a disk and partition before using ACTIVE.\n") );
        fResult = false;
        goto exit_activatepartition;
    }

    if( g_hPart == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a partition before using ACTIVE.\n") );
        fResult = false;
        goto exit_activatepartition;
    }

    strTok = _tcstok( strParams->GetString(), Delimiters );
    while( strTok != NULL )
    {
        if( _tcsnicmp( strTok, _T("bootsector"), 10 ) == 0 && strTok[10] == _T('=') )
        {
            if( FAILED(StringCchLength( strTok + 11, MAX_PATH, &stLength ) ) )
            {
                _tprintf( _T("Unable to activate partition %d.\n"), __LINE__ );
                fResult = false;
                goto exit_activatepartition;
            }

            if( !strBootSector.AllocateString( stLength + 1 ) )
            {
                _tprintf( _T("Out of memory.\n" ) );
                fResult = false;
                goto exit_activatepartition;
            }

            if( FAILED(StringCchCopy( strBootSector.GetString(),
                                      stLength + 1,
                                      strTok + 11 ) ) )
            {
                _tprintf( _T("Unable to activate partition %d.\n"), __LINE__ );
                fResult = false;
                goto exit_activatepartition;
            }
        }
        else if( _tcsnicmp( strTok, _T("bootloader"), 10 ) == 0 && strTok[10] == _T('=') )
        {
            if( FAILED(StringCchLength( strTok + 11, MAX_PATH, &stLength ) ) )
            {
                _tprintf( _T("Unable to activate partition %d.\n"), __LINE__ );
                fResult = false;
                goto exit_activatepartition;
            }

            if( !strBootLoader.AllocateString( stLength + 1 ) )
            {
                _tprintf( _T("Out of memory.\n" ) );
                fResult = false;
                goto exit_activatepartition;
            }

            if( FAILED(StringCchCopy( strBootLoader.GetString(),
                                      stLength + 1,
                                      strTok + 11 ) ) )
            {
                _tprintf( _T("Unable to activate partition: %d.\n"), __LINE__ );
                fResult = false;
                goto exit_activatepartition;
            }
        }
        else if( _tcsnicmp( strTok, _T("help"), 4 ) == 0 )
        {
            PrintActiveHelp();
            fResult = false;
            goto exit_activatepartition;
        }
        else
        {
            _tprintf( _T("The arguments you specified for this command are not valid.\n") );
            fResult = false;
            goto exit_activatepartition;
        }

        strTok = _tcstok( NULL, Delimiters );
    }

    if( !(fResult = ReadMBR( &pMBR, &StoreInfo )) )
    {
        goto exit_activatepartition;
    }

    if( !(fResult = MBRIsValid( pMBR, StoreInfo )) )
    {
        _tprintf( _T("The selected disk is not compatible with Windows CE.\n") );
        goto exit_activatepartition;
    }

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                       - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pMBR + dwTableOffset);

    //
    // Make sure that only the current partition has the active bit set.
    //
    for( USHORT x = 0; x < MAX_PARTTABLE_ENTRIES; x++ )
    {
        BYTE Flags = pTable[x].Part_BootInd & ~PART_ACTIVE;

        if( x == g_Partition )
        {
             Flags |= PART_ACTIVE;
        }

        pTable[x].Part_BootInd = Flags;
    }

    if( !(fResult = WriteMBR( pMBR, StoreInfo )) )
    {
        goto exit_activatepartition;
    }

    //
    // Now write the boot sector if it was given.
    //
    if( strBootSector.GetString() )
    {
        DWORD dwImgSize = 0;

        pSector = new BYTE[StoreInfo.dwBytesPerSector];
        if( !pSector )
        {
            _tprintf( _T("Out of memory.\n") );
            fResult = false;
            goto exit_activatepartition;
        }

        HANDLE hBootSector = CreateFile( strBootSector.GetString(),
                                         GENERIC_READ,
                                         FILE_SHARE_READ,
                                         NULL,
                                         OPEN_EXISTING,
                                         FILE_ATTRIBUTE_NORMAL,
                                         NULL );
        if( hBootSector == INVALID_HANDLE_VALUE )
        {
            _tprintf( _T("Unable to write the boot sector %d.\n"), __LINE__ );
            fResult = false;
            goto exit_activatepartition;
        }

        dwImgSize = GetFileSize( hBootSector, NULL );

        if( dwImgSize > StoreInfo.dwBytesPerSector )
        {
            _tprintf( _T("The boot sector file is too large to fit in a sector.\n") );
            fResult = false;
            goto exit_activatepartition;
        }

        IoInfo.sr_start = pTable[g_Partition].Part_StartSector;
        IoInfo.sr_num_sec = 1;
        IoInfo.sr_num_sg = 1;
        IoInfo.sr_sglist[0].sb_buf = pSector;
        IoInfo.sr_sglist[0].sb_len = StoreInfo.dwBytesPerSector;

        if( !DeviceIoControl( g_hDisk,
                              IOCTL_DISK_READ,
                              &IoInfo,
                              sizeof(IoInfo),
                              NULL,
                              0,
                              &dwBytesRead,
                              NULL ) )
        {
            _tprintf( _T("Unable to read partition %d, sector 0: %d\n"),
                      g_Partition,
                      GetLastError() );
            fResult = false;
            goto exit_activatepartition;
        }

        if( !ReadFile( hBootSector,
                       pSector + StoreInfo.dwBytesPerSector - dwImgSize,
                       dwImgSize,
                       &dwBytesRead,
                       NULL ) )
        {
            _tprintf( _T("Unable to read the boot sector file:%d\n"), GetLastError() );
            fResult = false;
            goto exit_activatepartition;
        }

        IoInfo.sr_start = pTable[g_Partition].Part_StartSector;
        IoInfo.sr_num_sec = 1;
        IoInfo.sr_num_sg = 1;
        IoInfo.sr_sglist[0].sb_buf = pSector;
        IoInfo.sr_sglist[0].sb_len = StoreInfo.dwBytesPerSector;

        if( !DeviceIoControl( g_hDisk,
                              IOCTL_DISK_WRITE,
                              &IoInfo,
                              sizeof(IoInfo),
                              NULL,
                              0,
                              &dwBytesWritten,
                              NULL ) )
        {
            _tprintf( _T("Unable to write boot sector: %d\n"), GetLastError() );
            fResult = false;
            goto exit_activatepartition;
        }
    }

    if( strBootLoader.GetString() )
    {
        CSimpleString strNewPath;
        PARTINFO PartInfo = { 0 };
        size_t stTotalLength = 3;   // \\VolumeName\\file.xxx\0
        size_t stOffset = 0;

        PartInfo.cbSize = sizeof(PartInfo);

        if( !GetPartitionInfo( g_hPart, &PartInfo ) )
        {
            fResult = false;
            _tprintf( _T("Unable to get partition information: %d\n"), GetLastError() );
            goto exit_activatepartition;
        }

        if( FAILED(StringCchLength( PartInfo.szVolumeName, MAX_PATH, &stLength ) ) )
        {
            _tprintf( _T("Unable to activate partition %d.\n"), __LINE__ );
            fResult = false;
            goto exit_activatepartition;
        }

        stTotalLength += stLength;

        if( FAILED(StringCchLength( strBootLoader.GetString(), MAX_PATH, &stLength ) ) )
        {
            _tprintf( _T("Unable to activate partition %d.\n"), __LINE__ );
            fResult = false;
            goto exit_activatepartition;
        }

        stTotalLength += stLength;

        //
        // Find the offset of the file name within the path provided.
        //
        for( size_t x = 0; x < stLength; x++ )
        {
            if( strBootLoader.GetString()[x] == _T('\\') )
            {
                stOffset = x;
            }
        }

        if( !strNewPath.AllocateString( stTotalLength ) )
        {
            _tprintf( _T("Out of memory.\n" ) );
            fResult = false;
            goto exit_activatepartition;
        }

        _sntprintf( strNewPath.GetString(),
                    strNewPath.GetBufSizeInChars() - 1,
                    _T("\\%s\\%s"),
                    PartInfo.szVolumeName,
                    &strBootLoader.GetString()[stOffset] );

        if( !CopyFile( strBootLoader.GetString(),
                       strNewPath.GetString(),
                       FALSE ) )
        {
            _tprintf( _T("Unable to copy boot loader from %s to %s\n"),
                      strBootLoader.GetString(),
                      strNewPath.GetString() );
            fResult = false;
            goto exit_activatepartition;
        }
    }

exit_activatepartition:
    if( pMBR )
    {
        delete [] pMBR;
    }

    if( pSector )
    {
        delete [] pSector;
    }

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// CleanDisk
//
// This function will erase the partition table in the MBR.  It will not erase
// anything else there, leaving the boot code and the signature at the end of
// the MBR.
//
bool CleanDisk()
{
    STOREINFO StoreInfo = { 0 };
    BYTE* pBuffer = NULL;
    DWORD dwBytesRead = 0;
    bool fResult = true;
    DWORD dwTableOffset = 0;
    CSimpleString strStoreName;
    size_t stLength;

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a disk before using CLEAN.\n") );
        fResult = false;
        goto exit_cleandisk;
    }

    if( !(fResult = ReadMBR( &pBuffer, &StoreInfo )) )
    {
        goto exit_cleandisk;
    }

    if( g_hPart != INVALID_HANDLE_VALUE )
    {
        g_Partition = 0;
        CloseHandle( g_hPart );
        g_hPart = INVALID_HANDLE_VALUE;
    }

    if( !DismountStore( g_hDisk ) )
    {
        _tprintf( _T("Unable to dismount the store: %d\n"), GetLastError() );
        fResult = false;
        goto exit_cleandisk;
    }

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                        - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    ZeroMemory( pBuffer + dwTableOffset, MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY) );

    if( !(fResult = WriteMBR( pBuffer, StoreInfo )) )
    {
        goto exit_cleandisk;
    }

    CloseHandle( g_hDisk );
    g_hDisk = INVALID_HANDLE_VALUE;
    g_Disk = 0;

    StringCchLength( StoreInfo.szDeviceName, MAX_PATH, &stLength );

    if( !strStoreName.AllocateString( sizeof("\\storemgr\\") +
                                      stLength ) )
    {
        _tprintf( _T("Unable to allocate store name: %d\n"), GetLastError() );
        fResult = false;
        goto exit_cleandisk;
    }

    if( !strStoreName.Append( _T("\\storemgr\\") ) ||
        !strStoreName.Append( StoreInfo.szDeviceName ) )
    {
        _tprintf( _T("Unable to clean disk.\n") );
        fResult = false;
        goto exit_cleandisk;
    }

    //
    // This should force the storage manager to refresh this store.  Ackward.
    //
    MoveFile( strStoreName.GetString(), strStoreName.GetString() );

exit_cleandisk:
    if( pBuffer )
    {
        delete [] pBuffer;
    }

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// CreatePartition
//
// This function will create a partition in the given disk's MBR.
//
// TODO::Currently there is only support for a primary partition.  Add support
// for extended partitions and logical drives.
//
bool CreatePartition( CSimpleString* strParams )
{
    bool fResult = true;
    TCHAR* Delimiters = _T(" ");
    TCHAR* strTok = NULL;
    PartitionTypes Type = ptInvalid;
    ULONG ulSelect = 0;
    DWORD dwSize = 0;
    DWORD dwStart = 0;
    BYTE* pSector = NULL;
    STOREINFO StoreInfo = { 0 };
    DWORD dwNumSectors = 0;
    CSimpleString strStoreName;
    size_t stLength= 0;

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a disk before using CREATE.\n") );
        return false;
    }

    strTok = _tcstok( strParams->GetString(), Delimiters );
    if( !strTok )
    {
        PrintCreateHelp();
        return false;
    }

    if( !_tcsicmp( strTok, _T("primary") ) )
    {
        Type = ptPrimary;
    }
    else if( !_tcsicmp( strTok, _T("extended") ) )
    {
        Type = ptExtended;
    }
    else if( !_tcsicmp( strTok, _T("logical") ) )
    {
        Type = ptLogical;
    }
    else
    {
        PrintCreateHelp();
        return false;
    }

    //
    // Determine if a size or offset was specified.
    //
    while( strTok = _tcstok( NULL, Delimiters ) )
    {
        if( _tcsnicmp( strTok, _T("size"), 4 ) == 0 && strTok[4] == _T('=') )
        {
            dwSize = _tcstoul( strTok + 5, NULL, 0 );

            if( !dwSize )
            {
                _tprintf( _T("The arguments you specified for this command are not valid.\n") );
                return false;
            }
        }
        else if( _tcsnicmp( strTok, _T("offset"), 6 ) == 0 && strTok[6] == _T('=') )
        {
            dwStart = _tcstoul( strTok + 7, NULL, 0 );

            if( !dwStart )
            {
                _tprintf( _T("The arguments you specified for this command are not valid.\n") );
                return false;
            }
        }
        else
        {
            _tprintf( _T("The arguments you specified for this command are not valid.\n") );
            return false;
        }
    }

    //
    // We need to confirm that we can create a partition of the given type.
    //
    if( !(fResult = ReadMBR( &pSector, &StoreInfo )) )
    {
        goto exit_createpartition;
    }

    //
    // Make sure that there are no missing entries in the MBR.
    //
    CompactMBR( pSector, StoreInfo );

    //
    // Make sure the entries are ordered correctly and that there is only one
    // extended partition.
    //
    if( !(fResult = MBRIsValid( pSector, StoreInfo )) )
    {
        _tprintf( _T("The selected disk is not compatible with Windows CE.\n") );
        goto exit_createpartition;
    }

    //
    // Check the number of partition entries currently in use on the disk.
    //
    if( EntriesInUse( pSector, StoreInfo ) == MAX_PARTTABLE_ENTRIES )
    {
        _tprintf( _T("DiskPart was unable to create the specified partition.\n") );
        fResult = false;
        goto exit_createpartition;
    }

    DWORD SectorsPerMB = (1024 * 1024) / StoreInfo.dwBytesPerSector;

    //
    // Make sure we don't overflow a 32-bit value.
    //
    if( dwSize && (ULONG_MAX / dwSize < SectorsPerMB) )
    {
        _tprintf( _T("The arguments specified for this command are not valid.\n") );
        fResult = false;
        goto exit_createpartition;
    }

    dwNumSectors = dwSize * SectorsPerMB;

    //
    // If we are given a start then we need to confirm that:
    //   a) It is a valid location on the disk
    //   b) It is not already in a partition
    //
    // If we are given a size then we need to confirm that:
    //   a) There is an extent on the disk large enough for this size
    //
    // If we are given a start and an extent then we need to confirm that:
    //   a) The given location has a large enough free extent for the size
    //
    // If we are given neither then we will find the first free area on disk
    // and create the largest possible extent at that location.
    //
    // CE does not allow a partition N+1 to start at a sector before the start
    // of partition N.
    //
    if( dwStart >= StoreInfo.snNumSectors )
    {
        _tprintf( _T("The arguments specified for this command are not valid.\n") );
        fResult = false;
        goto exit_createpartition;
    }

    if( dwNumSectors >= StoreInfo.snNumSectors )
    {
        _tprintf( _T("The arguments specified for this command are not valid.\n") );
        fResult = false;
        goto exit_createpartition;
    }

    if( ULONG_MAX - dwStart < dwNumSectors )
    {
        _tprintf( _T("The arguments specified for this command are not valid.\n") );
        fResult = false;
        goto exit_createpartition;
    }

    if( dwNumSectors + dwStart >= StoreInfo.snNumSectors )
    {
        _tprintf( _T("The arguments specified for this command are not valid.\n") );
        fResult = false;
        goto exit_createpartition;
    }

    if( dwStart && !SectorIsFree( dwStart, pSector, StoreInfo ) )
    {
        _tprintf( _T("The arguments specified for this command are not valid.\n") );
        fResult = false;
        goto exit_createpartition;
    }

    if( dwNumSectors > GetLargestPossiblePartition( pSector, StoreInfo ) )
    {
        _tprintf( _T("DiskPart was unable to create the specified partition.\n") );
        fResult = false;
        goto exit_createpartition;
    }

    if( !DismountStore( g_hDisk ) )
    {
        _tprintf( _T("Unable to dismount the store: %d\n"), GetLastError() );
        fResult = false;
        goto exit_createpartition;
    }

    if( !(fResult = InsertPartition( pSector, StoreInfo, dwStart, dwNumSectors )) )
    {
        goto exit_createpartition;
    }

    if( !(fResult = WriteMBR( pSector, StoreInfo )) )
    {
        goto exit_createpartition;
    }

    StringCchLength( StoreInfo.szDeviceName, MAX_PATH, &stLength );

    if( !strStoreName.AllocateString( sizeof("\\storemgr\\") +
                                      stLength ) )
    {
        _tprintf( _T("Unable to allocate store name: %d\n"), GetLastError() );
        fResult = false;
        goto exit_createpartition;
    }

    if( !strStoreName.Append( _T("\\storemgr\\") ) ||
        !strStoreName.Append( StoreInfo.szDeviceName ) )
    {
        _tprintf( _T("Unable to create the partition.\n") );
        fResult = false;
        goto exit_createpartition;
    }

    //
    // This should force the storage manager to refresh this store.  Ackward.
    //
    MoveFile( strStoreName.GetString(), strStoreName.GetString() );

    //
    // The storage manager will asynchronously update the storage info.  Let's
    // give it time to do this.
    //
    // TODO::Is there an event that we could wait for instead?
    //
    Sleep( 2000 );

exit_createpartition:
    if( pSector )
    {
        delete [] pSector;
    }

    return fResult;

}

// /////////////////////////////////////////////////////////////////////////////
// DeletePartition
//
// This function will remove a partition from the given disk's MBR.  The MBR
// will be compacted so that there are no blank partitions between valid
// partitions.
//
bool DeletePartition()
{
    bool fResult = true;
    BYTE* pSector = NULL;
    STOREINFO StoreInfo = { 0 };
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    CSimpleString strStoreName;
    size_t stLength = 0;

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a disk and partition before using DELETE PARTITION.\n") );
        return false;
    }

    if( g_hPart == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a partition before using DELETE PARTITION.\n") );
        return false;
    }

    if( !DismountStore( g_hDisk ) )
    {
        _tprintf( _T("Unable to dismount the store: %d\n"), GetLastError() );
        fResult = false;
        goto exit_deletepartition;
    }

    if( !(fResult = ReadMBR( &pSector, &StoreInfo )) )
    {
        goto exit_deletepartition;
    }

    if( !(fResult = MBRIsValid( pSector, StoreInfo )) )
    {
        _tprintf( _T("The selected disk is not compatible with Windows CE.\n") );
        goto exit_deletepartition;
    }

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                        - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pSector + dwTableOffset);

    ZeroMemory( &pTable[g_Partition], sizeof(PARTENTRY) );

    CompactMBR( pSector, StoreInfo );

    if( !(fResult = WriteMBR( pSector, StoreInfo )) )
    {
        goto exit_deletepartition;
    }

    CloseHandle( g_hPart );
    g_hPart = INVALID_HANDLE_VALUE;
    g_Partition = 0;

    StringCchLength( StoreInfo.szDeviceName, MAX_PATH, &stLength );

    if( !strStoreName.AllocateString( sizeof("\\storemgr\\") +
                                      stLength ) )
    {
        _tprintf( _T("Unable to allocate store name: %d\n"), GetLastError() );
        fResult = false;
        goto exit_deletepartition;
    }

    if( !strStoreName.Append( _T("\\storemgr\\") ) ||
        !strStoreName.Append( StoreInfo.szDeviceName ) )
    {
        _tprintf( _T("Unable to delete the partition.\n") );
        fResult = false;
        goto exit_deletepartition;
    }

    //
    // This should force the storage manager to refresh this store.  Ackward.
    //
    MoveFile( strStoreName.GetString(), strStoreName.GetString() );

    //
    // The storage manager will asynchronously update the storage info.  Let's
    // give it time to do this.
    //
    // TODO::Is there an event that we could wait for instead?
    //
    Sleep( 2000 );

exit_deletepartition:
    if( pSector )
    {
        delete [] pSector;
    }

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// SetPartitionType
//
bool SetPartitionType( CSimpleString* strParams )
{
    bool fResult = true;
    TCHAR* Delimiters = _T(" ");
    TCHAR* strTok = NULL;
    PartitionTypes Type = ptInvalid;
    ULONG ulSelect = 0;
    DWORD dwType = 0;
    STOREINFO StoreInfo = { 0 };
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    BYTE* pMBR = NULL;

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a disk and partition before using SETPARTITIONTYPE.\n") );
        goto exit_setpartitiontype;
    }

    if( g_hPart == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a partition before using SETPARTITIONTYPE.\n") );
        goto exit_setpartitiontype;
    }

    strTok = _tcstok( strParams->GetString(), Delimiters );
    if( !strTok )
    {
        PrintSetPartitionTypeHelp();
        fResult = false;
        goto exit_setpartitiontype;
    }

    dwType = _tcstoul( strTok, NULL, 0 );
    if( !dwType )
    {
        _tprintf( _T("The arguments you specified for this command are not valid.\n") );
        fResult = false;
        goto exit_setpartitiontype;
    }

    if( dwType > MAXBYTE )
    {
        _tprintf( _T("The partition type is invalid.  See bootpart.h.\n") );
        fResult = false;
        goto exit_setpartitiontype;
    }

    //
    // We need to confirm that we can create a partition of the given type.
    //
    if( !(fResult = ReadMBR( &pMBR, &StoreInfo )) )
    {
        goto exit_setpartitiontype;
    }

    if( !(fResult = MBRIsValid( pMBR, StoreInfo )) )
    {
        _tprintf( _T("The selected disk is not compatible with Windows CE.\n") );
        goto exit_setpartitiontype;
    }

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                        - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pMBR + dwTableOffset);
    pTable->Part_FileSystem = (BYTE)dwType;

    if( !(fResult = WriteMBR( pMBR, StoreInfo )) )
    {
        goto exit_setpartitiontype;
    }

exit_setpartitiontype:
    if( pMBR )
    {
        delete [] pMBR;
    }

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// FormatPartition
//
bool FormatPartition( CSimpleString* strParams )
{
    bool fResult = true;
    HINSTANCE hInstance = LoadLibrary( L"FatUtil.DLL" );
    FORMAT_PARAMS FormatOptions = {0};
    PFN_FORMATVOLUMEEX FormatFunction = NULL;

    if( hInstance == NULL )
    {
        fResult = false;
        _tprintf( _T("Unable to format partition: cannot load FatUtil.DLL.\n") );
        goto exit_formatpartition;
    }

    if( g_hPart == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a partition before using FORMATPARTITION.\n") );
        goto exit_formatpartition;
    }

    FormatFunction = (PFN_FORMATVOLUMEEX)GetProcAddress( hInstance, _T("FormatVolumeEx") );
    if( FormatFunction == NULL )
    {
        fResult = false;
        _tprintf( _T("Unable to load FormatVolumeEx from FatUtil.DLL.\n") );
        goto exit_formatpartition;
    }

    DismountPartition( g_hPart );
    FormatOptions.cbSize = sizeof(FormatOptions);
    FormatOptions.fo.dwFatVersion = 16;
    FormatOptions.fo.dwNumFats = 1;
    FormatOptions.fo.dwRootEntries = 32;
    FormatOptions.pfnMessage = NULL;
    FormatOptions.pfnProgress = NULL;
    if( FormatFunction( g_hPart, &FormatOptions ) != ERROR_SUCCESS )
    {
        fResult = false;
        _tprintf( _T("Failed to format the partition.\n") );
        goto exit_formatpartition;
    }

    if( !MountPartition( g_hPart ) )
    {
        fResult = false;
        _tprintf( _T("Unable to mount partition: %d\n"), GetLastError() );
        goto exit_formatpartition;
    }

    _tprintf( _T("Partition mounted.\n") );

exit_formatpartition:
    if( hInstance )
    {
        FreeLibrary( hInstance );
    }

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// PrintHelp
//
// This function will display a list of commands supported by the diskpart
// utility.
//
bool PrintHelp( CSimpleString* strParams )
{
    _tprintf( _T("The following commands are supported by DiskPart:\n") );
    _tprintf( _T("ACTIVE        - Activate the selected partition.\n") );
    _tprintf( _T("INACTIVE      - Clear the active flag on the selected partition.\n") );
    _tprintf( _T("CLEAN         - Erase the partition table in the MBR.\n") );
    _tprintf( _T("CREATE        - Create a table entry for a partition in the MBR.\n") );
    _tprintf( _T("DELETE        - Delete a table entry for a partition in the MBR.\n") );
    _tprintf( _T("LIST          - Print a list of partitions or disks.\n") );
    _tprintf( _T("SELECT        - Select a disk or partition.\n") );
    _tprintf( _T("DUMPMBR       - Print the partition table of the MBR.\n") );
    _tprintf( _T("DUMPSTORE     - Print the STOREINFO on the current disk.\n") );
    _tprintf( _T("DUMPPART      - Print the PARTINFO on the current partition.\n") );
    _tprintf( _T("ZERO          - Write all zeroes to a given sector on the disk.\n") );
    _tprintf( _T("DUMPSECTOR    - Print the contents of a sector on the disk.\n") );
    _tprintf( _T("MBRCODE       - Write the contents of a file to the code portion of the MBR.\n") );
    _tprintf( _T("BOOTSEC       - Write a file to the boot sector of the disk.\n") );
    _tprintf( _T("FIXPARTOFFSET - Update the BPB with the correct offset of the partition.\n") );
    _tprintf( _T("SETPARTITIONTYPE - Update the partition type in the MBR.\n") );
    _tprintf( _T("FORMATPARTITION  - Format the current partition as FAT16.\n") );
    return true;
}

// /////////////////////////////////////////////////////////////////////////////
// ListObjects
//
// This function will display a list partitions or disks, depending on which
// was requested.
//
bool ListObjects( CSimpleString* strParams )
{
    bool fResult = true;
    TCHAR* Delimiters = _T(" ");
    TCHAR* strTok = NULL;
    StoreObjects Type = soInvalid;
    ULONG ulSelect = 0;

    STOREINFO StoreInfo = { 0 };
    PARTINFO PartInfo = { 0 };
    HANDLE hFind = INVALID_HANDLE_VALUE;
    ULONG ulCount = 0;
    BYTE* pSector = NULL;

    strTok = _tcstok( strParams->GetString(), Delimiters );
    if( !strTok )
    {
        PrintListHelp();
        fResult = false;
        goto exit_listobjects;
    }

    if( !_tcsicmp( strTok, _T("disk") ) )
    {
        Type = soDisk;
    }
    else if( !_tcsicmp( strTok, _T("partition") ) )
    {
        Type = soPart;
    }
    else
    {
        PrintListHelp();
        fResult = false;
        goto exit_listobjects;
    }

    strTok = _tcstok( NULL, Delimiters );
    if( strTok )
    {
        _tprintf( _T("The arguments you specified for this command are not valid.") );
        fResult = false;
        goto exit_listobjects;
    }

    if( Type == soDisk )
    {
        DWORD dwResult = ERROR_SUCCESS;
        ULONG NumDisks = 0;

        PrintDiskHeader();

        //
        // Disks will start at index 1.
        //
        ulCount = 1;

        StoreInfo.cbSize = sizeof(StoreInfo);
        hFind = FindFirstStore( &StoreInfo );
        if( hFind == INVALID_HANDLE_VALUE )
        {
            _tprintf( _T("No stores found: %d\n"), GetLastError() );
            return false;
        }

        do
        {
            //
            // Do not display CD/DVD devices.
            // This might not be restrictive enough.
            //
            if( StoreInfo.sdi.dwDeviceClass == STORAGE_DEVICE_CLASS_BLOCK )
            {
                if( !(StoreInfo.dwAttributes & STORE_ATTRIBUTE_REMOVABLE) )
                {
                    PrintDisk( ulCount++, StoreInfo );
                }
            }
        } while( FindNextStore( hFind, &StoreInfo ) );

    }
    else
    {
        if( g_hDisk == INVALID_HANDLE_VALUE )
        {
            _tprintf( _T("There is not disk selected to list partitions.\n") );
            _tprintf( _T("Select a disk and try again.\n") );
            return true;
        }

        if( !(fResult = ReadMBR( &pSector, &StoreInfo )) )
        {
            goto exit_listobjects;
        }

        if( !(fResult = MBRIsValid( pSector, StoreInfo )) )
        {
            _tprintf( _T("The selected disk is not compatible with Windows CE.\n") );
            goto exit_listobjects;
        }

        PrintPartitions( pSector, StoreInfo );
    }

exit_listobjects:
    if( hFind != INVALID_HANDLE_VALUE )
    {
        if( Type == soDisk )
        {
            FindCloseStore( hFind );
        }
        else
        {
            FindClosePartition( hFind );
        }
    }

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// DeActivatePartition
//
// This function will clear the active partition flag on the selected partition.
//
bool DeActivatePartition()
{
    if( !g_Disk || !g_Partition )
    {
        _tprintf( _T("SELECT a partition before using INACTIVE.\n") );
        return false;
    }

    //
    // TODO::Deactivate the selected partition on the selected disk.
    //

    return true;
}

// /////////////////////////////////////////////////////////////////////////////
// SelectObject
//
// This function will select a disk or partition to be acted on for other
// commands.
//
bool SelectObject( CSimpleString* strParams )
{
    bool fResult = true;
    TCHAR* Delimiters = _T(" ");
    TCHAR* strTok = NULL;
    StoreObjects Type = soInvalid;
    ULONG ulSelect = 0;

    strTok = _tcstok( strParams->GetString(), Delimiters );
    if( !strTok )
    {
        PrintSelectHelp();
        fResult = false;
        goto exit_selectobject;
    }

    if( !_tcsicmp( strTok, _T("disk") ) )
    {
        Type = soDisk;
    }
    else if( !_tcsicmp( strTok, _T("partition") ) )
    {
        Type = soPart;
    }
    else
    {
        PrintSelectHelp();
        fResult = false;
        goto exit_selectobject;
    }

    strTok = _tcstok( NULL, Delimiters );
    if( !strTok )
    {
        if( Type == soDisk )
        {
            if( g_hDisk == INVALID_HANDLE_VALUE )
            {
                _tprintf( _T("There is no disk selected.\n") );
                goto exit_selectobject;
            }

            _tprintf( _T("Disk %d is now the selected disk.\n"), g_Disk );
            goto exit_selectobject;
        }
        else
        {
            if( g_hDisk == INVALID_HANDLE_VALUE )
            {
                _tprintf( _T("There is not disk selected to set the partition.\n") );
                _tprintf( _T("Select a disk and try again.\n") );
                goto exit_selectobject;
            }

            if( g_hPart == INVALID_HANDLE_VALUE )
            {
                _tprintf( _T("There is no partition selected.\n") );
                goto exit_selectobject;
            }

            _tprintf( _T("Partition %d is now the selected partition.\n"), g_Partition );
            goto exit_selectobject;
        }
    }

    ulSelect = _tcstoul( strTok, NULL, 0 );

    if( !ulSelect && !SizeIsZero( strTok ) )
    {
        _tprintf( _T("The arguments you specified for this command are not valid.\n") );
    }

    if( Type == soDisk )
    {
        if( !SelectDisk( ulSelect ) )
        {
            _tprintf( _T("The specified disk is not valid.\n") );
            fResult = false;
            goto exit_selectobject;
        }
    }
    else
    {
        if( !SelectPartition( ulSelect ) )
        {
            _tprintf( _T("The specified partition is not valid.\n") );
            fResult = false;
            goto exit_selectobject;
        }
    }

exit_selectobject:

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// DumpMBR
//
// This function will display the partition table for the MBR on the current
// disk.
//
bool DumpMBR()
{
    bool fResult = true;
    BYTE* pSector = NULL;
    STOREINFO StoreInfo = { 0 };
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    USHORT usEntries = 0;

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("There is not disk selected to list partitions.\n") );
        _tprintf( _T("Select a disk and try again.\n") );
        return true;
    }

    //
    // We need to confirm that we can create a partition of the given type.
    //
    if( !(fResult = ReadMBR( &pSector, &StoreInfo )) )
    {
        goto exit_dumpmbr;
    }

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                        - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pSector + dwTableOffset);

    for( USHORT x = 0; x < MAX_PARTTABLE_ENTRIES; x++ )
    {
        _tprintf( _T("  [%d]\tFlags:\t%d\n"), x, pTable[x].Part_BootInd );
        _tprintf( _T("  \tStart Sector:\t%d\n"), pTable[x].Part_StartSector );
        _tprintf( _T("  \tTotal Sectors:\t%d\n"), pTable[x].Part_TotalSectors );
        _tprintf( _T("  \tFile System:\t%d\n"), pTable[x].Part_FileSystem );
    }

exit_dumpmbr:
    if( pSector )
    {
        delete [] pSector;
    }

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// DumpStoreInfo
//
// This function will display the STOREINFO for the current disk.
//
bool DumpStoreInfo()
{
    bool fResult = true;
    STOREINFO StoreInfo = { 0 };

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("There is not disk selected to list partitions.\n") );
        _tprintf( _T("Select a disk and try again.\n") );
        return true;
    }

    StoreInfo.cbSize = sizeof(STOREINFO);
    if( !(fResult = GetStoreInfo( g_hDisk, &StoreInfo ) != FALSE) )
    {
        _tprintf( _T("Unable to collect disk information: %d\n"), GetLastError() );
        goto exit_dumpmbr;
    }

    _tprintf( _T("Store Info\n") );
    _tprintf( _T("----------\n") );
    _tprintf( _T("Store Name:\t%s\n"), StoreInfo.szStoreName );
    _tprintf( _T("Device Name:\t%s\n"), StoreInfo.szDeviceName );
    _tprintf( _T("Attributes:\t%d\n"), StoreInfo.dwAttributes );
    _tprintf( _T("Bytes/Sector:\t%d\n"), StoreInfo.dwBytesPerSector );
    _tprintf( _T("Free Sectors:\t%I64d\n"), StoreInfo.snFreeSectors );
    _tprintf( _T("Total Sectors:\t%I64d\n"), StoreInfo.snNumSectors );
    _tprintf( _T("Profile Name:\t%s\n"), StoreInfo.sdi.szProfile );
    _tprintf( _T("\n") );

exit_dumpmbr:

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// DumpPartInfo
//
// This function will display the PARTINFO for the current disk.
//
bool DumpPartInfo()
{
    bool fResult = true;
    PARTINFO PartInfo = { 0 };

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a disk and partition before DUMPPART.\n") );
        _tprintf( _T("Select a disk and try again.\n") );
        return true;
    }

    if( g_hPart == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a partition before DUMPPART.\n") );
        _tprintf( _T("Select a disk and try again.\n") );
        return true;
    }

    PartInfo.cbSize = sizeof(PARTINFO);
    if( !(fResult = GetPartitionInfo( g_hPart, &PartInfo ) != FALSE) )
    {
        _tprintf( _T("Unable to collect disk information: %d\n"), GetLastError() );
        goto exit_dumpmbr;
    }

    _tprintf( _T("Partition Info\n") );
    _tprintf( _T("--------------\n") );
    _tprintf( _T("Partition Name:\t%s\n"), PartInfo.szPartitionName );
    _tprintf( _T("File System:\t%s\n"), PartInfo.szFileSys );
    _tprintf( _T("Volume Name:\t%s\n"), PartInfo.szVolumeName );
    _tprintf( _T("Total Sectors:\t%I64d\n"), PartInfo.snNumSectors );
    _tprintf( _T("Attributes:\t0x%x\n"), PartInfo.dwAttributes );
    _tprintf( _T("Partition Type:\t%d\n"), PartInfo.bPartType );
    _tprintf( _T("\n") );

exit_dumpmbr:

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// ZeroSector
//
// This function will completely erase a given sector on the current disk.
//
bool ZeroSector( CSimpleString* strParams )
{
    bool fResult = true;
    TCHAR* Delimiters = _T(" ");
    TCHAR* strTok = NULL;
    ULONG ulSelect = 0;
    BYTE* pSector = NULL;
    STOREINFO StoreInfo = { 0 };
    DWORD dwSector = 0;
    SG_REQ IoInfo = { 0 };
    DWORD dwBytesWritten = 0;

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a disk before using ZEROSECTOR.\n") );
        fResult = false;
        goto exit_zerosector;
    }

    strTok = _tcstok( strParams->GetString(), Delimiters );
    if( !strTok )
    {
        PrintZeroSectorHelp();
        fResult = false;
        goto exit_zerosector;
    }

    dwSector = _tcstoul( strTok, NULL, 0 );
    if( !dwSector && !SizeIsZero( strTok) )
    {
        _tprintf( _T("The arguments you specified for this command are not valid.\n") );
        return false;
    }

    StoreInfo.cbSize = sizeof(STOREINFO);
    if( !GetStoreInfo( g_hDisk, &StoreInfo ) )
    {
        _tprintf( _T("Unable to collect disk information: %d\n"), GetLastError() );
        fResult = false;
        goto exit_zerosector;
    }

    if( dwSector >= StoreInfo.snNumSectors )
    {
        _tprintf( _T("The specified sector is beyond the end of the disk.\n") );
        fResult = false;
        goto exit_zerosector;
    }

    pSector = new BYTE[StoreInfo.dwBytesPerSector];
    if( !pSector )
    {
        _tprintf( _T("Out of memory.\n") );
        fResult = false;
        goto exit_zerosector;
    }

    ZeroMemory( pSector, StoreInfo.dwBytesPerSector );

    IoInfo.sr_start = dwSector;
    IoInfo.sr_num_sec = 1;
    IoInfo.sr_num_sg = 1;
    IoInfo.sr_sglist[0].sb_buf = pSector;
    IoInfo.sr_sglist[0].sb_len = StoreInfo.dwBytesPerSector;

    if( !DeviceIoControl( g_hDisk,
                          IOCTL_DISK_WRITE,
                          &IoInfo,
                          sizeof(IoInfo),
                          NULL,
                          0,
                          &dwBytesWritten,
                          NULL ) )
    {
        _tprintf( _T("Unable to write sector: %d\n"), GetLastError() );
        fResult = false;
        goto exit_zerosector;
    }

exit_zerosector:
    if( pSector )
    {
        delete [] pSector;
    }

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// DumpSector
//
// This will output a sector to the screen in hex format.
//
bool DumpSector( CSimpleString* strParams )
{
    bool fResult = true;
    TCHAR* Delimiters = _T(" ");
    TCHAR* strTok = NULL;
    ULONG ulSelect = 0;
    BYTE* pSector = NULL;
    STOREINFO StoreInfo = { 0 };
    DWORD dwSector = 0;
    SG_REQ IoInfo = { 0 };
    DWORD dwBytesRead = 0;
    DWORD dwOffset = 0;

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a disk before using DUMPSECTOR.\n") );
        fResult = false;
        goto exit_dumpsector;
    }

    strTok = _tcstok( strParams->GetString(), Delimiters );
    if( !strTok )
    {
        PrintDumpSectorHelp();
        fResult = false;
        goto exit_dumpsector;
    }

    dwSector = _tcstoul( strTok, NULL, 0 );
    if( !dwSector && !SizeIsZero( strTok ) )
    {
        _tprintf( _T("The arguments you specified for this command are not valid.\n") );
        fResult = false;
        goto exit_dumpsector;
    }

    StoreInfo.cbSize = sizeof(STOREINFO);
    if( !GetStoreInfo( g_hDisk, &StoreInfo ) )
    {
        _tprintf( _T("Unable to collect disk information: %d\n"), GetLastError() );
        fResult = false;
        goto exit_dumpsector;
    }

    if( dwSector >= StoreInfo.snNumSectors )
    {
        _tprintf( _T("The specified sector is beyond the end of the disk.\n") );
        fResult = false;
        goto exit_dumpsector;
    }

    pSector = new BYTE[StoreInfo.dwBytesPerSector];
    if( !pSector )
    {
        _tprintf( _T("Out of memory.\n") );
        fResult = false;
        goto exit_dumpsector;
    }

    IoInfo.sr_start = dwSector;
    IoInfo.sr_num_sec = 1;
    IoInfo.sr_num_sg = 1;
    IoInfo.sr_sglist[0].sb_buf = pSector;
    IoInfo.sr_sglist[0].sb_len = StoreInfo.dwBytesPerSector;

    if( !DeviceIoControl( g_hDisk,
                          IOCTL_DISK_READ,
                          &IoInfo,
                          sizeof(IoInfo),
                          NULL,
                          0,
                          &dwBytesRead,
                          NULL ) )
    {
        _tprintf( _T("Unable to write sector: %d\n"), GetLastError() );
        fResult = false;
        goto exit_dumpsector;
    }

    dwBytesRead = StoreInfo.dwBytesPerSector;

    while( dwBytesRead )
    {
        DWORD dwToShow = dwBytesRead < 16 ? dwBytesRead : 16;
        for( DWORD x = 0; x < dwToShow; x++ )
        {
            _tprintf( _T("%02X "), pSector[dwOffset + x] );
        }

        for( DWORD y = 0; y < dwToShow; y++ )
        {
            int c = pSector[dwOffset + y];
            if( isalpha( c ) && (c <= 0x80 || c >= 0x8f) )
            {
                _tprintf( _T("%c"), pSector[dwOffset + y] );
            }
            else
            {
                _tprintf( _T(".") );
            }
        }

        _tprintf( _T("\n") );
        dwOffset += dwToShow;
        dwBytesRead -= dwToShow;
    }

exit_dumpsector:
    if( pSector )
    {
        delete [] pSector;
    }

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// WriteMBRCode
//
// This function will write data to the code area in the MBR.
//
// It is passed a file name containing the code to write to the MBR.
//
// file=xxxx.xxx
//
bool WriteMBRCode( CSimpleString* strParams )
{
    bool fResult = true;
    TCHAR* Delimiters = _T(" ");
    TCHAR* strTok = NULL;
    BYTE* pSector = NULL;
    STOREINFO StoreInfo = { 0 };
    DWORD dwSector = 0;
    SG_REQ IoInfo = { 0 };
    DWORD dwBytesWritten = 0;
    DWORD dwSizeToWrite = 0;
    CSimpleString strFileName;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a disk before using MBRCODE.\n") );
        fResult = false;
        goto exit_writembrcode;
    }

    strTok = _tcstok( strParams->GetString(), Delimiters );
    if( !strTok )
    {
        PrintMBRCodeHelp();
        fResult = false;
        goto exit_writembrcode;
    }

    if( _tcsnicmp( strTok, _T("file"), 4 ) == 0 && strTok[4] == _T('=') )
    {
        size_t stLength = 0;

        if( FAILED(StringCchLength( strTok + 5, MAX_PATH, &stLength ) ) )
        {
            _tprintf( _T("Unable to write MBR code[%d].\n"), __LINE__ );
            fResult = false;
            goto exit_writembrcode;
        }

        if( !strFileName.AllocateString( stLength + 1 ) )
        {
            _tprintf( _T("Out of memory.\n" ) );
            fResult = false;
            goto exit_writembrcode;
        }

        if( FAILED(StringCchCopy( strFileName.GetString(),
                                  stLength + 1,
                                  strTok + 5 ) ) )
        {
            _tprintf( _T("Unable to write MBR code[%d].\n"), __LINE__  );
            fResult = false;
            goto exit_writembrcode;
        }
    }
    else
    {
        _tprintf( _T("The arguments you specified for this command are not valid.\n") );
        return false;
    }

    if( !ReadMBR( &pSector, &StoreInfo ) )
    {
        _tprintf( _T("Unable to write MBR code[%d].\n"), __LINE__  );
        fResult = false;
        goto exit_writembrcode;
    }

    hFile = CreateFile( strFileName.GetString(),
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

    if( hFile == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("Unable to open %s: %d\n"), strFileName.GetString(), GetLastError() );
        fResult = false;
        goto exit_writembrcode;
    }

    dwSizeToWrite = StoreInfo.dwBytesPerSector - 2
                      - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    if( GetFileSize( hFile, NULL ) > dwSizeToWrite )
    {
        _tprintf( _T("The file is too large to fit in the code area of the MBR.\n") );
        fResult = false;
        goto exit_writembrcode;
    }
    else
    {
        dwSizeToWrite = GetFileSize( hFile, NULL );
    }

    if( !ReadFile( hFile,
                   pSector,
                   dwSizeToWrite,
                   &dwBytesWritten,
                   NULL ) )
    {
        _tprintf( _T("Unable to read the file: %d\n"), GetLastError() );
        fResult = false;
        goto exit_writembrcode;
    }

    if( !(fResult = WriteMBR( pSector, StoreInfo )) )
    {
        goto exit_writembrcode;
    }

exit_writembrcode:
    if( hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( hFile );
    }

    if( pSector )
    {
        delete [] pSector;
    }

    return fResult;
}

bool WriteBootSector( CSimpleString* strParams )
{
    bool fResult = true;
    TCHAR* strTok = NULL;
    const TCHAR* Delimiters = _T(" ");
    size_t stLength = 0;
    bool fSector = false;
    DWORD dwSector = 0;
    CSimpleString strFileName;
    STOREINFO StoreInfo = { 0 };
    HANDLE hFile = INVALID_HANDLE_VALUE;
    BYTE* pSector = NULL;
    DWORD dwByteCount = 0;
    SG_REQ IoInfo = { 0 };

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a disk before using BOOTSEC.\n") );
        fResult = false;
        goto exit_writebootsector;
    }

    strTok = _tcstok( strParams->GetString(), Delimiters );
    while( strTok != NULL )
    {
        if( _tcsnicmp( strTok, _T("file"), 4 ) == 0 && strTok[4] == _T('=') )
        {
            size_t stLength = 0;

            if( FAILED(StringCchLength( strTok + 5, MAX_PATH, &stLength ) ) )
            {
                _tprintf( _T("Unable to write MBR code[%d].\n"), __LINE__ );
                fResult = false;
                goto exit_writebootsector;
            }

            if( !strFileName.AllocateString( stLength + 1 ) )
            {
                _tprintf( _T("Out of memory.\n" ) );
                fResult = false;
                goto exit_writebootsector;
            }

            if( FAILED(StringCchCopy( strFileName.GetString(),
                                      stLength + 1,
                                      strTok + 5 ) ) )
            {
                _tprintf( _T("Unable to write boot sector[%d].\n"), __LINE__  );
                fResult = false;
                goto exit_writebootsector;
            }
        }
        else if( _tcsnicmp( strTok, _T("sector"), 6 ) == 0 && strTok[6] == _T('=') )
        {
            dwSector = _tcstoul( strTok + 7, NULL, 0 );

            if( !dwSector && !SizeIsZero( strTok ) )
            {
                fResult = false;
                _tprintf( _T("The arguments you specified for this command are not valid.\n") );
                goto exit_writebootsector;
            }

            fSector = true;
        }
        else
        {
            PrintWriteBootSectorHelp();
            fResult = false;
            goto exit_writebootsector;
        }

        strTok = _tcstok( NULL, Delimiters );
    }

    if( !fSector || !strFileName.GetString() )
    {
        PrintWriteBootSectorHelp();
        fResult = false;
        goto exit_writebootsector;
    }

    StoreInfo.cbSize = sizeof(StoreInfo);
    if( !GetStoreInfo( g_hDisk, &StoreInfo ) )
    {
        fResult = false;
        _tprintf( _T("Unable to write boot sector: %d\n"), GetLastError() );
        goto exit_writebootsector;
    }

    if( dwSector >= StoreInfo.snNumSectors )
    {
        fResult = false;
        _tprintf( _T("The specified sector does not exist.\n") );
        goto exit_writebootsector;
    }

    pSector = new BYTE[StoreInfo.dwBytesPerSector];
    if( !pSector )
    {
        fResult = false;
        _tprintf( _T("Out of memory.\n") );
        goto exit_writebootsector;
    }

    //
    // First we read in the sector that we're going to write to.  We don't want
    // to overwrite any data that was already there other than what is in the
    // boot sector file.
    //
    IoInfo.sr_start = dwSector;
    IoInfo.sr_num_sec = 1;
    IoInfo.sr_num_sg = 1;
    IoInfo.sr_sglist[0].sb_buf = pSector;
    IoInfo.sr_sglist[0].sb_len = StoreInfo.dwBytesPerSector;

    if( !DeviceIoControl( g_hDisk,
                          IOCTL_DISK_READ,
                          &IoInfo,
                          sizeof(IoInfo),
                          NULL,
                          0,
                          &dwByteCount,
                          NULL ) )
    {
        _tprintf( _T("Unable to write sector: %d\n"), GetLastError() );
        fResult = false;
        goto exit_writebootsector;
    }

    hFile = CreateFile( strFileName.GetString(),
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

    if( hFile == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("Unable to open %s: %d\n"), strFileName.GetString(), GetLastError() );
        fResult = false;
        goto exit_writebootsector;
    }

    dwByteCount = GetFileSize( hFile, NULL );

    if( dwByteCount > StoreInfo.dwBytesPerSector )
    {
        _tprintf( _T("The file is larger than a sector.\n") );
        fResult = false;
        goto exit_writebootsector;
    }

    if( !ReadFile( hFile,
                   pSector + StoreInfo.dwBytesPerSector - dwByteCount,
                   dwByteCount,
                   &dwByteCount,
                   NULL ) )
    {
        _tprintf( _T("Unable to read the file: %d\n"), GetLastError() );
        fResult = false;
        goto exit_writebootsector;
    }

    //
    // We need to change the first 3 bytes.  These bytes are used to jump to the
    // offset from which to start executing.  We can determine this on the size
    // of the file we are using.
    //
    // It should look like this:
    // EB xx - JMP offset
    // 90    - NOP
    //
    // Since the BPB is 59 bytes for FAT16 and there is one byte for the NOP,
    // the offset will be 0x3C for FAT16.  Or 512 - 450 - 2 = 60 = 0x3C.
    // Where 450 is the file size for bsect.img on FAT16.
    //
    // Since the BPB is 87 bytes on FAT32 and there is one byte for the NOP,
    // the offset will be 0x58 for FAT32.  Or 512 - 422 - 2 = 88 = 0x58.
    // Where 422 is the file size for bsect.img on FAT32.
    //
    pSector[0] = 0xEB;
    pSector[1] = (BYTE)(512 - dwByteCount - 2);
    pSector[2] = 0x90;

    IoInfo.sr_start = dwSector;
    IoInfo.sr_num_sec = 1;
    IoInfo.sr_num_sg = 1;
    IoInfo.sr_sglist[0].sb_buf = pSector;
    IoInfo.sr_sglist[0].sb_len = StoreInfo.dwBytesPerSector;

    if( !DeviceIoControl( g_hDisk,
                          IOCTL_DISK_WRITE,
                          &IoInfo,
                          sizeof(IoInfo),
                          NULL,
                          0,
                          &dwByteCount,
                          NULL ) )
    {
        _tprintf( _T("Unable to write boot sector: %d\n"), GetLastError() );
        fResult = false;
        goto exit_writebootsector;
    }

exit_writebootsector:
    if( pSector )
    {
        delete [] pSector;
    }

    if( hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( hFile );
    }

    return fResult;
}

bool FixPartOffset()
{
    bool fResult = false;
    BYTE* pSector = NULL;
    STOREINFO StoreInfo = { 0 };
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    DWORD dwPartOffset = 0;
    SG_REQ IoInfo = { 0 };
    DWORD dwByteCount = 0;
    char strFileSys[6] = { 0 };

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a disk and partition before using FIXPARTOFFSET.\n") );
        goto exit_fixhiddensectors;
    }

    if( g_hPart == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a partition before using FIXPARTOFFSET.\n") );
        goto exit_fixhiddensectors;
    }

    //
    // We need to confirm that we can create a partition of the given type.
    //
    if( !(fResult = ReadMBR( &pSector, &StoreInfo )) )
    {
        goto exit_fixhiddensectors;
    }

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                        - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pSector + dwTableOffset);
    dwPartOffset = pTable[g_Partition].Part_StartSector;

    IoInfo.sr_start = dwPartOffset;
    IoInfo.sr_num_sec = 1;
    IoInfo.sr_num_sg = 1;
    IoInfo.sr_sglist[0].sb_buf = pSector;
    IoInfo.sr_sglist[0].sb_len = StoreInfo.dwBytesPerSector;

    if( !DeviceIoControl( g_hDisk,
                          IOCTL_DISK_READ,
                          &IoInfo,
                          sizeof(IoInfo),
                          NULL,
                          0,
                          &dwByteCount,
                          NULL ) )
    {
        _tprintf( _T("Unable to read sector: %d\n"), GetLastError() );
        goto exit_fixhiddensectors;
    }

    dwByteCount = StoreInfo.dwBytesPerSector;

    strncpy( strFileSys, (char*)&pSector[3], 5 );

    if( _strnicmp( strFileSys, "EXFAT", 5 ) == 0 )
    {
        PBPBEX pBPB = (PBPBEX)pSector;
        if( pBPB->PartitionOffset == dwPartOffset )
        {
            _tprintf( _T("The partition offset is already correct.\n") );
            fResult = true;
            goto exit_fixhiddensectors;
        }


        if( pBPB->PartitionOffset != 0 )
        {
            _tprintf( _T("Warning: The partition offset was not zero: %d\n"), pBPB->PartitionOffset );
        }

        pBPB->PartitionOffset = dwPartOffset;
    }
    else
    {
        PBASE_BPB pBPB = (PBASE_BPB)pSector;
        if( pBPB->BPB_HiddenSectors == dwPartOffset )
        {
            _tprintf( _T("The number of hidden sectors is already correct.\n") );
            fResult = true;
            goto exit_fixhiddensectors;
        }

        if( pBPB->BPB_HiddenSectors != 0 )
        {
            _tprintf( _T("Warning: The hidden sectors was not zero: %d\n"), pBPB->BPB_HiddenSectors );
        }

        pBPB->BPB_HiddenSectors = dwPartOffset;
    }

    if( !DeviceIoControl( g_hDisk,
                          IOCTL_DISK_WRITE,
                          &IoInfo,
                          sizeof(IoInfo),
                          NULL,
                          0,
                          &dwByteCount,
                          NULL ) )
    {
        _tprintf( _T("Unable to write new BPB: %d\n"), GetLastError() );
        goto exit_fixhiddensectors;
    }

    fResult = true;
    _tprintf( _T("The partition offset is set to %d\n"), dwPartOffset );

exit_fixhiddensectors:
    if( pSector )
    {
        delete [] pSector;
    }

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// FixCRC
//
// This is used for exFAT partitions.  This will calculate a CRC based on the
// first 11 sectors and store that value in the twelth sector.
//
bool FixCRC()
{
    bool fResult = false;
    BYTE* pSector = NULL;
    STOREINFO StoreInfo = { 0 };
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    DWORD dwPartOffset = 0;
    SG_REQ IoInfo = { 0 };
    DWORD dwByteCount = 0;
    char strFileSys[6] = { 0 };
    BYTE* pBuffer = NULL;
    DWORD dwBufferSize = 0;
    DWORD dwChecksum = 0;

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a disk and partition before using FIXPARTOFFSET.\n") );
        goto exit_fixcrc;
    }

    if( g_hPart == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a partition before using FIXPARTOFFSET.\n") );
        goto exit_fixcrc;
    }

    //
    // We need to confirm that we can create a partition of the given type.
    //
    if( !(fResult = ReadMBR( &pSector, &StoreInfo )) )
    {
        goto exit_fixcrc;
    }

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                        - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pSector + dwTableOffset);
    dwPartOffset = pTable[g_Partition].Part_StartSector;

    if( FAILED(UIntMult( EXFAT_SECTORS_TO_CRC, StoreInfo.dwBytesPerSector, (UINT*)&dwBufferSize ) ) )
    {
        _tprintf( _T("Overflow while calculating buffer size.\n") );
        goto exit_fixcrc;
    }
//    dwBufferSize = EXFAT_SECTORS_TO_CRC * StoreInfo.dwBytesPerSector;

    pBuffer = new BYTE[ dwBufferSize ];
    if( !pBuffer )
    {
        _tprintf( _T("Unable to allocate space for the CRC data.\n") );
        goto exit_fixcrc;
    }

    IoInfo.sr_start = dwPartOffset;
    IoInfo.sr_num_sec = EXFAT_SECTORS_TO_CRC;
    IoInfo.sr_num_sg = 1;
    IoInfo.sr_sglist[0].sb_buf = pBuffer;
    IoInfo.sr_sglist[0].sb_len = dwBufferSize;

    if( !DeviceIoControl( g_hDisk,
                          IOCTL_DISK_READ,
                          &IoInfo,
                          sizeof(IoInfo),
                          NULL,
                          0,
                          &dwByteCount,
                          NULL ) )
    {
        _tprintf( _T("Unable to read sectors to CRC: %d\n"), GetLastError() );
        goto exit_fixcrc;
    }

    strncpy( strFileSys, (char*)&pBuffer[3], 5 );

    if( _strnicmp( strFileSys, "EXFAT", 5 ) != 0 )
    {
        _tprintf( _T("FixCRC is used to fix the CRC on exFAT volumes only.\n") );
        goto exit_fixcrc;
    }

    dwChecksum = ComputeBootSectorChecksum( pBuffer, dwBufferSize );
    ZeroMemory( pBuffer, StoreInfo.dwBytesPerSector );
    ((DWORD*)pBuffer)[0] = dwChecksum;

    IoInfo.sr_start = dwPartOffset + EXFAT_SECTORS_TO_CRC;
    IoInfo.sr_num_sec = 1;
    IoInfo.sr_num_sg = 1;
    IoInfo.sr_sglist[0].sb_buf = pBuffer;
    IoInfo.sr_sglist[0].sb_len = StoreInfo.dwBytesPerSector;

    if( !DeviceIoControl( g_hDisk,
                          IOCTL_DISK_WRITE,
                          &IoInfo,
                          sizeof(IoInfo),
                          NULL,
                          0,
                          &dwByteCount,
                          NULL ) )
    {
        _tprintf( _T("Unable to write new CRC: %d\n"), GetLastError() );
        goto exit_fixcrc;
    }

    fResult = true;
    _tprintf( _T("The CRC has been set.\n") );

exit_fixcrc:
    if( pSector )
    {
        delete [] pSector;
    }

    if( pBuffer )
    {
        delete [] pBuffer;
    }

    return fResult;
}

// /////////////////////////////////////////////////////////////////////////////
// SetBackupBootSection
//
// This is used for exFAT partitions.  This will duplicate the first 12 sectors
// into the BackupBoot section of the volume.
//
bool SetBackupBootSection()
{
    bool fResult = false;
    BYTE* pSector = NULL;
    STOREINFO StoreInfo = { 0 };
    DWORD dwTableOffset = 0;
    PPARTENTRY pTable = NULL;
    DWORD dwPartOffset = 0;
    SG_REQ IoInfo = { 0 };
    DWORD dwByteCount = 0;
    char strFileSys[6] = { 0 };
    BYTE* pBuffer = NULL;
    DWORD dwBufferSize = 0;

    if( g_hDisk == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a disk and partition before using FIXPARTOFFSET.\n") );
        goto exit_setbackupbootsection;
    }

    if( g_hPart == INVALID_HANDLE_VALUE )
    {
        _tprintf( _T("SELECT a partition before using FIXPARTOFFSET.\n") );
        goto exit_setbackupbootsection;
    }

    //
    // We need to confirm that we can create a partition of the given type.
    //
    if( !(fResult = ReadMBR( &pSector, &StoreInfo )) )
    {
        goto exit_setbackupbootsection;
    }

    dwTableOffset = StoreInfo.dwBytesPerSector - 2
                        - MAX_PARTTABLE_ENTRIES * sizeof(PARTENTRY);

    pTable = (PPARTENTRY)(pSector + dwTableOffset);
    dwPartOffset = pTable[g_Partition].Part_StartSector;

    if( FAILED(UIntMult( EXFAT_SECTORS_TO_CRC, StoreInfo.dwBytesPerSector, (UINT*)&dwBufferSize ) ) )
    {
        _tprintf( _T("Overflow while calculating buffer size.\n") );
        goto exit_setbackupbootsection;
    }
//    dwBufferSize = EXFAT_SECTORS_TO_CRC * StoreInfo.dwBytesPerSector;

    pBuffer = new BYTE[ dwBufferSize ];
    if( !pBuffer )
    {
        _tprintf( _T("Unable to allocate space for the boot sectors.\n") );
        goto exit_setbackupbootsection;
    }

    IoInfo.sr_start = dwPartOffset;
    IoInfo.sr_num_sec = EXFAT_BOOT_SECTORS;
    IoInfo.sr_num_sg = 1;
    IoInfo.sr_sglist[0].sb_buf = pBuffer;
    IoInfo.sr_sglist[0].sb_len = dwBufferSize;

    if( !DeviceIoControl( g_hDisk,
                          IOCTL_DISK_READ,
                          &IoInfo,
                          sizeof(IoInfo),
                          NULL,
                          0,
                          &dwByteCount,
                          NULL ) )
    {
        _tprintf( _T("Unable to read sectors to CRC: %d\n"), GetLastError() );
        goto exit_setbackupbootsection;
    }

    strncpy( strFileSys, (char*)&pSector[3], 5 );

    if( _strnicmp( strFileSys, "EXFAT", 5 ) != 0 )
    {
        _tprintf( _T("SetBackupBootSectors is used on exFAT volumes only.\n") );
        goto exit_setbackupbootsection;
    }

    IoInfo.sr_start = dwPartOffset + EXFAT_BOOT_SECTORS;
    IoInfo.sr_num_sec = EXFAT_BOOT_SECTORS;
    IoInfo.sr_num_sg = 1;
    IoInfo.sr_sglist[0].sb_buf = pBuffer;
    IoInfo.sr_sglist[0].sb_len = dwBufferSize;

    if( !DeviceIoControl( g_hDisk,
                          IOCTL_DISK_WRITE,
                          &IoInfo,
                          sizeof(IoInfo),
                          NULL,
                          0,
                          &dwByteCount,
                          NULL ) )
    {
        _tprintf( _T("Unable to write to backup boot section: %d\n"), GetLastError() );
        goto exit_setbackupbootsection;
    }

    fResult = true;
    _tprintf( _T("The boot sectors have been duplicated.\n") );

exit_setbackupbootsection:
    if( pSector )
    {
        delete [] pSector;
    }

    if( pBuffer )
    {
        delete [] pBuffer;
    }

    return fResult;
}


