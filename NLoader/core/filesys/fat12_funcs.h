
#ifndef _FAT12_H_
#define _FAT12_H_

#ifdef __cplusplus
extern "C" {
#endif

//
// Function prototypes
//
BOOL FAT12_FSInit(void);
ULONG FAT12_FSOpenFile(PCHAR pFileName);
void FAT12_FSCloseFile(void);
BOOL FAT12_FSReadFile(PUCHAR pAddress, ULONG Length);
BOOL FAT12_FSRewind(ULONG offset);

#ifdef __cplusplus
}
#endif

#endif	// _FAT12_H_
