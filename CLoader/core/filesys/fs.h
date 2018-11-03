#pragma once

//#include <windows.h>

#include <FileSysAPI.h>

#ifdef __cplusplus
extern "C" {
#endif


BOOLEAN ReadSectors(UCHAR DriveID, ULONG LBA, USHORT nSectors, PUCHAR pBuffer);

//////////////////////////////////////////
//
// BIN image and record headers
//
#pragma pack(1)
typedef struct			// Image header (one per BIN image)
{
    CHAR SyncBytes[7];
    ULONG ImageAddr;
    ULONG ImageLen;
} IMAGEHDR, *PIMAGEHDR;
#pragma pack()

#pragma pack(1)
typedef struct			// Record header (one per section in image)
{
    ULONG RecordAddr;
    ULONG RecordLen;
    ULONG RecordChksum;
} RECORDHDR, *PRECORDHDR;
#pragma pack()

typedef struct
{
	BOOL useEdd;
    UCHAR DriveId;
	USHORT usNumCyl;
    USHORT usNumHeads;
    USHORT usSectorsPerTrack;
	USHORT usSectorSize;
	DWORD dwTotalSectors;
} DRIVE_INFO, *PDRIVE_INFO;

// Helper macros
#define TO_UPPER(a)     ((a >= 0x61) && (a <=0x7A) ? a - 0x20 : a)
#define MIN(a, b)       (a < b ? a : b)

#ifdef __cplusplus
}
#endif
