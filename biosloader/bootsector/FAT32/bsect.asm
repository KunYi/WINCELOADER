;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this sample source code is subject to the terms of the Microsoft
; license agreement under which you licensed this sample source code. If
; you did not accept the terms of the license agreement, you are not
; authorized to use this sample source code. For the terms of the license,
; please see the license agreement between you and Microsoft or, if applicable,
; see the LICENSE.RTF on your install media or the root of your tools installation.
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
;


.model tiny
.code
.stack
.startup

STACK_START EQU 0FA00H  ; Starting location of stack.
INT1E_VECTOR    EQU 0078H   ; INT 01Eh vector location.
READ_LENGTH EQU     68  ; Number size of bootloader image in sectors.
LOAD_ADDRESS    EQU     1000H   ; Boot loader load address.

;
; Even though we declare 0xFA00 as a starting address, the BIOS will
; load us at 0x7C00. But just at the beginning of our code, we'll 
; copy ourselves to 0xFA00 and jump in the right place.
;
org 0FA00h

;
; This file contains the boot sector code for the Windows CE BIOS bootloader.
; A quick overview of the boot process:
;
; * At power-on, the BIOS performs POST.
; * The BIOS seeks out the boot disk and on a fixed disk, loads the first sector
;   (the master boot record - MBR) and jumps to it (the MBR contains code and data).
;   Note that for a floppy, the first sector on the disk is the boot sector (no MBR).
; * The MBR code, amongst other things, locates the active boot partition and loads
;   the boot sector code (this code).
; * The boot sector code is the first filesystem-specific bit of code and it's job
;   is to locate, load, and bootstrap the OS (or in our case, the bootloader).
;

;
; Start of boot sector code.
;
    JMP     Start
    NOP

    ;
    ; BIOS parameter block (BPB).
    ; Note: we're going to rely on the format program to set this up for us.  When our
    ; boot sector is written to the storage device, we'll leave the BIOS parameter block
    ; in tact.
    ;
    ;                                           Offset  Size
    ;                                           0   3
    VerId           db  'WINDOWCE'  ; 7C03  3       8
    BytePerSect     dw  0000        ; 7C0B  11  2
    SectPerClust    db  00      ; 7C0D  13  1
    RsvdSects       dw  0000        ; 7C0E  14  2
    NumFATs         db  00      ; 7C10  16  1
    Num32bEntry     dw  0000        ; 7C11  17  2
    SectPerPart     dw  0000        ; 7C13  19  2
    MediaDesc       db  00      ; 7C15  21  1
    SectPerFAT      dw  0000        ; 7C16  22  2
    SectPerTrack    dw  0000        ; 7C18  24  2
    NumHeads        dw  0000        ; 7C1A  26  2
    NumHiddenSectL  dw  0000        ; 7C1C  28  4
    NumHiddenSectH  dw  0000        ; 7C1E  
    TotalSectorsL   dw  0000        ; 7C20  32  4
    TotalSectorsH   dw  0000        ; 7C22
    SectPerFATL     dw  0000        ; 7C24  36  4
    SectPerFATH     dw  0000        ; 7C26
    FAT32Flags      dw  0000        ; 7C28  40      2
    FSVersion       dw  0000        ; 7C2A  42      2
    FirstClusterL   dw  0000        ; 7C2C  44      4
    FirstClusterH   dw  0000        ; 7C2E
    FSInfo      dw  0000        ; 7C30  48  2
    BackupBootSect  dw  0000        ; 7C32  50  2
    Reserved        db  '            '  ; 7C34  52  12
    DriveId         db  00      ; 7C40  64  1
    Reserved1       db  00      ; 7C41  65  1
    ExtRecordSig    db  00      ; 7C42  66  1
    VolSerNumL      dw  0000        ; 7C43  67  4
    VolSerNumH      dw  0000        ; 7C45  
    VolLabel        db  '           '   ; 7C47  71  11
    TypeFAT         db  '        '  ; 7C52  82  8
                        ;               90
;
; So the size of the file should be SECTOR_SIZE - 90.  This will be 422
; on a 512 byte/sector hard disk.
;


;
; Start of boot sector code
;
Start:
_7C5A:
    ; Set up the stack and segments.
    CLI                 ; 7C5A
    XOR     AX,AX
    MOV     SS,AX
    MOV     SP,STACK_START
    PUSH    SS
    POP     ES
    PUSH    CS
    POP     DS

    ; To make more room for the BLDR code
    ; copy the bootsector code from 0x7C00 to 0xFA00
    MOV     SI,7C00H
    MOV     DI,0FA00H
    MOV     CX,0200H
    CLD
    REPZ    MOVSB

    ; Make a relative jump so that we actually execute from the new memory location (0xFA00)
    ; Yes - this is a jump short command
    DB      00E9H
    DW      (0FA00h - 7C00h)

    STI
    
    ; Locate the first sector of the data area
    ; For FAT32 this is as follows:
    ; FirstDataSector = BPB_ReservedSectors + (BPB_NumFATs * BPB_FATSz32)
    ;
    XOR     AX, AX
    MOV     AL,[NumFATs]
    MUL     WORD PTR [SectPerFATL]
    MOV     BX, AX
    MOV     CX, DX
    MOV     AL,[NumFATs]
    MUL     WORD PTR [SectPerFATH]

    ; TODO::DX must be zero or we will overflow.  Bail out if it isn't zero.
    ;      CX:BX
    ; + DX:AX:00
    ; ----------
    ADD     CX, AX
    MOV     AX, BX
    MOV     DX, CX


    ADD     AX,[NumHiddenSectL]
    ADC     DX,[NumHiddenSectH]
    ADD     AX,[RsvdSects]
    ADC     DX,+00

    ;
    ; BLDR is expecting to find the first data sector at this location, not
    ; the first sector of the root directory.  These two may not be equal
    ; in FAT32.
    ;
    MOV     WORD PTR [_7C5A+0BH],AX
    MOV     WORD PTR [_7C5A+0DH],DX

    ; Locate the first sector of the root directory
    ;
    XOR     AX, AX
    MOV     AX, WORD PTR [FirstClusterL]
    DEC     AX
    DEC     AX
    MUL     [SectPerClust]

    ; TODO::Assuming that [FirstClusterH] is zero.  This should be true when
    ; we format the media.

    ; Begin to read from the root directory (into sector cache at 0x500).
    ; We're going to look for our bootloader file name in the directory.
    MOV     BX,0500H                  ; Buffer Address
    ADD     AX, WORD PTR [_7C5A+0BH]
    ADC     DX, WORD PTR [_7C5A+0DH]
    CALL    ReadSectorEx
    JB      DiskError

    ;
    ; CX = BytesPerSector / BytesPerEntry = Number of entries in a sector
    ;
    MOV     AX,[BytePerSect]
    MOV     CL,05H
    SHR     AX,CL
    MOV     CX,AX
    
FindEntry:
    PUSH    CX
    MOV     DI,BX
    MOV     CX,000BH
    MOV     SI, Offset ImageName
    REPZ    CMPSB
    JZ      FoundFiles      ; Found it?
    ADD     BX,020H     ; Move to the next entry
    POP     CX
    LOOP    FindEntry

    ; A disk error has occurred - usually this is because we couldn't find the bootloader
    ; file on the disk.  Display the error text below.
    ;
DiskError:
    MOV     SI,Offset ErrorMsg
    CALL    PrintMessage
    XOR     AX,AX
    INT     16h
    POP     SI
    POP     DS
    POP     [SI]
    POP     [SI+02H]
    INT     19h

ConvertFail:
    POP     AX
    POP     AX
    POP     AX
    JMP     DiskError

    ;
    ; We found our bootloader on the disk - now we need to load it.  There are a couple of
    ; important things here:
    ;
    ; * The bootloader image must be in a contiguous grouping of sectors on the disk.  This is
    ;   because the bootloader doesn't contain code to load the rest of itself, like DOS's IO.SYS
    ;   does.  This requirement imposes a retriction which must be addressed by the utility that
    ;   writes the bootloader image to disk.  On a newly-formatted disk, empty disk, the main
    ;   problem would come from bad disk sectors.
    ;
    ; * The boot sector loads the bootloader at a predetermined address (0x1000h).  This location
    ;   is compatible with the bootloader configuration (in boot.bib).  As well, it allows for
    ;   the BIOS interrupt vector table to remain unaffected.
    ;
    ; * When the bootloader image is loaded, the boot sector code loads a predetermined number of
    ;   sectors from the storage device.  The number of sectors to be loaded is determined at build
    ;   time and if the loader image size changes, this number will need to be updated (READ_LENGTH above).
    ;
    ; * The boot sector puts information into CPU registers that are passed up to the bootloader.  These
    ;   values are important for the loader to operate and care needs to be taken in the early loader
    ;   initialization, to avoid losing this data.
    ;
FoundFiles:
    ; TODO::Assuming that the high word of the entry's first cluster [BX+14H] is zero.
    ;
    MOV     AX,[BX+1AH]
    DEC     AX
    DEC     AX
    MOV     BL,[SectPerClust]
    XOR     BH,BH
    MUL     BX
    ADD     AX,WORD PTR [_7C5A+0BH] ; LBA
    ADC     DX,WORD PTR [_7C5A+0DH] ; LBA
    ; Load BLDR at 0000:1000h
    MOV     BX,LOAD_ADDRESS
    MOV     CX,READ_LENGTH
Loop1:
    CALL    ReadSectorEx
    JB      DiskError
    ADD     AX,0001H
    ADC     DX,+00
    ADD     BX,[BytePerSect]
    LOOP    Loop1
    
    ; Put the media type, drive number, and data start LBA into registers that are passed to the bootloader.
    ;
    MOV     BX,WORD PTR [_7C5A+0BH]
    MOV     AX,WORD PTR [_7C5A+0DH]
    
    ; Jump to the bootloader image.
    ;
    DB      00EAH
    DW      LOAD_ADDRESS
    DW      0000H

    ; Display a text message using the BIOS INT 10h call.
    ;
PrintMessage:
    LODSB
    OR      AL,AL
    JZ      Return
    MOV     AH,0EH
    MOV     BX,0007H
    INT     10h
    JMP     PrintMessage

;PrintAX:
;    PUSH     BX
;    XOR      BX, BX
;    PUSH     CX
;    MOV      CL, 4
;StartPrinting:
;    PUSH     AX
;    CMP      BX, 1
;    JG       DoneExchange
;    MOV      AL, AH
;DoneExchange:
;    TEST     BX, 1
;    JNE      ConvertNumber
;    SHR      AL, CL
;ConvertNumber:
;    AND      AL, 0FH
;    ADD      AL, 030H
;    CMP      AL, 03AH
;    JB       PrintNum
;    ADD      AL, 07H
;PrintNum:
;    MOV      AH, 0EH
;    INT      10H
;    POP      AX
;    INC      BX
;    CMP      BX, 4
;    JB       StartPrinting
;    PUSH     AX
;    MOV      AH, 0EH
;    MOV      AL,0DH
;    INT      10h
;    MOV      AL,0AH
;    INT      10h
;    POP      AX
;    POP      CX
;    POP      BX
;    JMP      Return

; PrintBuffer
; Input
;   BX - Buffer
;   CX - Bytes to print

;PrintBuffer:
;    PUSH AX
;    PUSH BX
;PrepareByte:
;    PUSH  CX
;    MOV   CL, 4
;    MOV   AL, BYTE PTR [BX]
;    ROR   AL, CL
;    MOV   AH, 0EH
;PrintByte:
;    PUSH  AX
;    AND   AL, 0FH
;    ADD   AL, 030H
;    CMP   AL, 03AH
;    JB    PrintNum
;    ADD   AL, 07H
;PrintNum:
;    INT   10H
;    POP   AX
;    SHR   AL,CL
;    SUB   CL,2       ; This is just to determine if we should try to print this byte
;    JNZ   PrintByte  ; again.  4 -> 2 -> 0
;    INC   BX
;    POP   CX
;    Loop  PrepareByte
;    MOV      AL,0DH
;    INT      10h
;    MOV      AL,0AH
;    INT      10h
;    POP BX
;    POP AX

Return:
    RET

;
; ReadSectorEx
;
; This uses the extended BIOS INT 13H to read a sector using an LBA.
;
; Input:
;   DX:AX - The sector to read in LBA.
;   BX - Buffer to store data
;
ReadSectorEx:
    PUSH     SI
    PUSH     AX
    PUSH     DX

    MOV      BYTE PTR [Packet], 10h
    MOV      BYTE PTR [Packet + 2], 1
    MOV      WORD PTR [Packet + 4], BX

    MOV      WORD PTR [Packet + 8], AX
    MOV      WORD PTR[Packet + 10], DX

    MOV      SI, Offset Packet
    MOV      DL, [DriveId]
    MOV      AH, 42H
    INT      13H

    POP      DX
    POP      AX
    POP      SI
    RET

    ;
    ; Data area.
    ; Note that the padding is used to ensure that the signature (which the MBR looks for to know
    ; it has a valid boot sector) falls at the end of the 512 byte sector.
    ;
ErrorMsg     DB  0DH, 0AH, 'Cannot load BLDR!', 0
ImageName    DB  'BLDR       '
Packet       DB  10h ; Size of the Packet - 16
             DB  00h ; Reserved - 0
             DB  01h ; Blocks to read - 1
             DB  00h ; Reserved - 0
             DB  00h ; 32-bit Address  = 0x500
             DB  00h
             DB  00h
             DB  00h
             DB  00h ; 64-bit Sector = 4190=0x105e
             DB  00h
             DB  00h
             DB  00h
             DB  00h
             DB  00h
             DB  00h
             DB  00h
             
Padding      DB  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
             DB  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
             DB  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
             DB  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
             DB  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
             DB  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
             DB  0,0,0,0,0,0,0,0,0,0,0,0
Signature    DB  55H, 0AAH

.exit

END
