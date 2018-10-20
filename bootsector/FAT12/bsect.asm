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
                        ;       0   3
    VerId           db  'WINDOWCE'  ; 7C03  3   8
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
    NumHiddenSectL  dw  0000        ; 7C1C  28  2
    NumHiddenSectH  dw  0000        ; 7C1E  30  2
    TotalSectorsL   dw  0000        ; 7C20  32  2
    TotalSectorsH   dw  0000        ; 7C22  34  2
    DriveId         db  00      ; 7C24  36  1
    TempVal         db  00      ; 7C25  37  1
    ExtRecordSig    db  00      ; 7C26  38  1
    VolSerNumL      dw  0000        ; 7C27  39  2
    VolSerNumH      dw  0000        ; 7C29  41  2
    VolLabel        db  '           '   ; 7C2B  43  11
    TypeFAT         db  '        '  ; 7C36  54  8
                        ;       62

;
; So the size of the file should be SECTOR_SIZE - 62.  This will be 450
; on a 512 byte/sector hard disk.
;

;
; Start of boot sector code
;
Start:
_7C3E:
    ; Set up the stack and segments.
    CLI                 ; 7C3E
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
    DB      00E9H
    DW      (0FA00h - 7C00h)

NEW_START:
    ; From this point the bootsector code executes in the 0xFA00 block
    XOR     AX,AX

    ; Copy disk params table on top of the start of our code by using
    ; the INT 1E vector.
    MOV     BX,INT1E_VECTOR
    LDS     SI,SS:[BX]
    PUSH    DS
    PUSH    SI
    PUSH    SS
    PUSH    BX
    MOV     DI,Start
    MOV     CX,000BH
    CLD
    REPZ    MOVSB
    PUSH    ES
    POP     DS
   
    ; Update disk settle time and sectors per track.
    ; 
    MOV     BYTE PTR [DI-02],0FH
    MOV     CX,[SectPerTrack]
    MOV     [DI-07H],CL
    
    ; Update the INT 1E vector to point to our new table.
    ;
    MOV     [BX+02H],AX
    MOV     WORD PTR [BX],Start
    
    ; Reset the disk so it'll use our new table.
    ;
    STI
    INT     13h
    JB      DiskError   ; Error?
   
    ; Locate the starting LBA for the root directory. 
    ; 
    MOV     AL,[NumFATs]
    MUL     WORD PTR [SectPerFAT]
    ADD     AX,[NumHiddenSectL]
    ADC     DX,[NumHiddenSectH]
    ADD     AX,[RsvdSects]
    ADC     DX,+00
    MOV     WORD PTR [_7C3E+0BH],AX
    MOV     WORD PTR [_7C3E+0DH],DX
    PUSH    AX  ; Save the rootdir LBA
    PUSH    DX  ; Save the rootdir LBA
    
    ; Locate the starting LBA for the data area.
    ;
    MOV     AX,0020H
    MUL     WORD PTR [Num32bEntry]
    MOV     BX,[BytePerSect]
    ADD     AX,BX
    DEC     AX
    DIV     BX
    ADD     WORD PTR [_7C3E+0BH],AX
    ADC     WORD PTR [_7C3E+0DH],+00
    
    ; Begin to read from the root directory (into sector cache at 0x500).
    ; We're going to look for our bootloader file name in the directory.
    ;
    MOV     BX,0500H
    POP     DX          ; Retrieve the rootdir LBA
    POP     AX          ; Retrieve the rootdir LBA
    CALL    CHSConvert
    JB      DiskError
    MOV     AL,01H
    CALL    ReadSector
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
;    CALL    ToUpper             ; Converts the bytes to upper case before comparing
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
    MOV     AX,[BX+1AH]
    DEC     AX
    DEC     AX
    MOV     BL,[SectPerClust]
    XOR     BH,BH
    MUL     BX
    ADD     AX,WORD PTR [_7C3E+0BH]
    ADC     DX,WORD PTR [_7C3E+0DH]
    ; Load BLDR at 0000:1000h
    MOV     BX,LOAD_ADDRESS
    MOV     CX,READ_LENGTH
Loop1:
    PUSH    AX
    PUSH    DX
    PUSH    CX
    CALL    CHSConvert
    JB      ConvertFail
    MOV     AL,01H
    CALL    ReadSector
    POP     CX
    POP     DX
    POP     AX
    JB      DiskError
    ADD    AX,0001H
    ADC     DX,+00
    ADD     BX,[BytePerSect]
    LOOP    Loop1
    
    ; Put the data start LBA into registers that are passed to the bootloader.
    ;
    MOV     BX,WORD PTR [_7C3E+0BH]
    MOV     AX,WORD PTR [_7C3E+0DH]
    
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

    ; Convert an LBA address to a physical CHS address to be used by the BIOS.
    ;
CHSConvert:
    CMP     DX,[SectPerTrack]
    JNB     ReturnError
    DIV     WORD PTR [SectPerTrack]
    INC     DL
    MOV     BYTE PTR [_7C3E+11H],DL
    XOR     DX,DX
    DIV     WORD PTR [NumHeads]
    MOV     [TempVal],DL
    MOV     WORD PTR [_7C3E+0FH],AX
    CLC
    RET

; ToUPper
; Input
;   BX - Buffer
;   CX - Bytes to upcase
;
;ToUpper:
;    PUSH  AX
;    PUSH  BX
;    PUSH  CX
;ToUpper_Loop:
;    MOV   AL, BYTE PTR [BX]
;    CMP   AL, 97
;    JL    Next_Char
;    CMP   AL, 122
;    JG    Next_Char
;    SUB   BYTE PTR [BX], 32
;Next_Char:
;    INC   BX
;    LOOP  ToUpper_Loop;
;    POP   CX
;    POP   BX
;    POP   AX
;    RET

ReturnError:
    STC

Return:
    RET

    ; Read a sector from the storage device using the BIOS INT 13h call.
    ;
ReadSector:
    MOV     AH,02H
    MOV     DX,WORD PTR [_7C3E+0FH]
    MOV     CL,06H
    SHL     DH,CL
    OR      DH,BYTE PTR [_7C3E+11H]
    MOV     CX,DX
    XCHG    CH,CL
    MOV     DL,[DriveId]
    MOV     DH,[TempVal]
    INT     13h
    RET

    ;
    ; Data area.
    ; Note that the padding is used to ensure that the signature (which the MBR looks for to know
    ; it has a valid boot sector) falls at the end of the 512 byte sector.
    ;
ErrorMsg     DB  0DH, 0AH, 'Unable to load BLDR!', 0
ImageName    DB  'BLDR       '
Padding2     DB  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
             DB  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
             DB  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
             DB  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
             DB  0,0,0,0,0,0,0
Signature    DB  55H, 0AAH

.exit

END
