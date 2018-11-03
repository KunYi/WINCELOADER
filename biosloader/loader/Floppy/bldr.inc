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
;-------------------------------------------------------------------------------
;
;       THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
;       ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
;       THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
;       PARTICULAR PURPOSE.
;
;-------------------------------------------------------------------------------

;
; Constants
;
SECTOR_SIZE         EQU 200h    ; Sector size.

FIXED_DISK_BIOS_ID  EQU 80h     ; BIOS fixed disk number (first fixed disk).
FLOPPY_RETRY_NUM    EQU 3       ; Number of times to retry a disk operation for a floppy.

STACK_BASE          EQU 0C8FCh  ; Reserved in boot.bib (stack reserved area is 0xB400 -> 0xC8FC).
STACK_SIZE          EQU 1500h   ; Reserved in boot.bib

VESA_GENINFO_START  EQU 08000h  ; VBE Controller Info
VESA_GENINFO_SIZE   EQU 0200h
VESA_WORK_RAM_START EQU (VESA_GENINFO_START + VESA_GENINFO_SIZE)    ; Work RAM start
READ_BUFFER_START   EQU 0C900h  ; Sector read buffer (end is 0F000)

;
; Structures
;

; BIOS parameter block (part of boot sector).
;
BIOSPB STRUCT
    VersionId       db  '        '
    BytesPerSect    dw  0000
    SectsPerClust   db  00
    RsvdSects       dw  0000
    NumFATs         db  00
    NumRootEntries  dw  0000
    SectsPerPart    dw  0000
    MediaDesc       db  00
    SectsPerFAT     dw  0000
    SectsPerTrack   dw  0000
    NumHeads        dw  0000
    NumHiddenSectL  dw  0000
    NumHiddenSectH  dw  0000
    TotalSectorsL   dw  0000
    TotalSectorsH   dw  0000
    DriveId         db  00
    TempVal         db  00
    ExtRecordSig    db  00
    VolSerNumL      dw  0000
    VolSerNumH      dw  0000
    VolLabel        db  '           '
    TypeFAT         db  '        '
BIOSPB ENDS


;
; Macros
;
;
; Prefix macros ----------------------------

OpPrefix MACRO
    db      66h
    ENDM

AddrPrefix MACRO
    db      67h
    ENDM


; Debug output macros ----------------------
;
; Display character
;
WriteChar MACRO char
        mov     dh, 003h 
        mov     dl, 0fdh 
                            
@@:                      
        in      al, dx      
        test    al, 20h     
        je      short @B 

        mov     dl, 0f8h
        mov     al, char
        out     dx, al

        ENDM

;
; Display value
;
WriteVal MACRO val
        ror     val, 4
        mov     ah, val
        and     ah, 0fh
        cmp     ah, 9
        jng     short @F
        add     ah, 7
@@:        
        add     ah, '0'
        WriteChar ah
        ror     val, 4
        mov     ah, val
        and     ah, 0fh
        cmp     ah, 9
        jng     short @F
        add     ah, 7
@@:        
        add     ah, '0'
        WriteChar ah
        ENDM

;
; Display 32-bit hex value in register EBX
;
; Note that the OpPrefix instructions mean this is useful only in 16-bit (real) mode.
;
WriteHex_EBX MACRO
        WriteChar '0'
        WriteChar 'x'
        OpPrefix
        ror       ebx, 10h
        WriteVal  bh
        WriteVal  bl

        OpPrefix
        ror       ebx, 10h
        WriteVal  bh
        WriteVal  bl
        WriteChar 0ah
        WriteChar 0dh
        ENDM

