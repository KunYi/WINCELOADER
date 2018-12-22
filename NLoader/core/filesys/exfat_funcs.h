
#ifndef _EXFAT_FUNC_H_
#define _EXFAT_FUNC_H_

#ifdef __cplusplus
extern "C" {
#endif

//
// Function prototypes
//
BOOL ExFAT_FSInit(void);
ULONG ExFAT_FSOpenFile(PCHAR pFileName);
void ExFAT_FSCloseFile(void);
BOOL ExFAT_FSReadFile(PUCHAR pAddress, ULONG Length);
BOOL ExFAT_FSRewind(ULONG offset);


#ifdef __cplusplus
}
#endif

#endif	// _EXFAT_FUNC_H_
