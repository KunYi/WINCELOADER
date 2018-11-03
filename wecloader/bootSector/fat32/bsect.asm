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
;
;-------------------------------------------------------------------------------

        .model tiny
        .code
        .stack
        .startup
        .386

LOAD_ADDRESS    EQU     1000h       ; Boot loader load address
READ_LENGTH     EQU     68          ; Size of boot loader image in sectors

;-------------------------------------------------------------------------------

OpPrefix MACRO
        db      66h
        ENDM

AddrPrefix MACRO
        db      67h
        ENDM

;-------------------------------------------------------------------------------
;
; Even though we declare 0xFA00 as a starting address, the BIOS will
; load us at 0x7C00. But just at the beginning of our code, we'll
; copy ourselves to 0xFA00 and jump in the right place.
;
; This file contains the boot sector code for the Windows CE BIOS Boot Loader.
; A quick overview of the boot process:
;
; * At power-on, the BIOS performs POST.
; * The BIOS seeks out the boot disk and on a fixed disk, loads the first
;   sector(the master boot record - MBR) and jumps to it (the MBR contains
;   code and data). Note that for a floppy, the first sector on the disk is
;   the boot sector (no MBR).
; * The MBR code, amongst other things, locates the active boot partition
;   and loads the boot sector code (this code).
; * The boot sector code is the first filesystem-specific bit of code and
;   it's job is to locate, load, and bootstrap the OS (or in our case,
;   the boot loader).
;
;
        org     0FA00h

        jmp     Start
        nop

; BIOS parameter block (BPB)
;
; This structure is filled when volume is formated for FAT file system.
; There is multiple BPB formats (one for FAT12 and FAT16, second for FAT32
; and another for exFat). This is also reason why we have different boot
; sector code for each FAT flavor.
;
;
;                                               Offset  Size
;                                           0   3
VerId           db  'WINDOWCE'          ; 7C03  3       8
BytePerSect     dw  0000                ; 7C0B  11      2
SectPerClust    db  00                  ; 7C0D  13      1
RsvdSects       dw  0000                ; 7C0E  14      2
NumFATs         db  00                  ; 7C10  16      1
Num32bEntry     dw  0000                ; 7C11  17      2
SectPerPart     dw  0000                ; 7C13  19      2
MediaDesc       db  00                  ; 7C15  21      1
SectPerFAT      dw  0000                ; 7C16  22      2
SectPerTrack    dw  0000                ; 7C18  24      2
NumHeads        dw  0000                ; 7C1A  26      2
NumHiddenSectL  dw  0000                ; 7C1C  28      4
NumHiddenSectH  dw  0000                ; 7C1E
TotalSectorsL   dw  0000                ; 7C20  32      4
TotalSectorsH   dw  0000                ; 7C22
SectPerFATL     dw  0000                ; 7C24  36      4
SectPerFATH     dw  0000                ; 7C26
FAT32Flags      dw  0000                ; 7C28  40      2
FSVersion       dw  0000                ; 7C2A  42      2
FirstClusterL   dw  0000                ; 7C2C  44      4
FirstClusterH   dw  0000                ; 7C2E
FSInfo          dw  0000                ; 7C30  48      2
BackupBootSect  dw  0000                ; 7C32  50      2
Reserved        db  '            '      ; 7C34  52      12
DriveId         db  00                  ; 7C40  64      1
Reserved1       db  00                  ; 7C41  65      1
ExtRecordSig    db  00                  ; 7C42  66      1
VolSerNumL      dw  0000                ; 7C43  67      4
VolSerNumH      dw  0000                ; 7C45
VolLabel        db  '           '       ; 7C47  71      11
TypeFAT         db  '        '          ; 7C52  82      8
; Diskette Parameter Table              ; 7C5A  90      11
; Int13Ext      db  00                  ; 7C65  101     1
; DataSectL     dw  0000                ; 7C66  102     2
; DataSectH     dw  0000                ; 7C68  104     2
;                                       ; 7C6A  106

;-------------------------------------------------------------------------------
;
; Start of boot sector code
;
Start:

        xor     cx, cx
        mov     ss, cx                      ; Zero stack segment
        mov     sp, 0FA00h                  ; Stack Pointer to 0xFA00
        mov     es, cx                      ; Zero extra segment
        mov     ds, cx                      ; Zero data segment

        mov     bp, sp                      ; Set base register to 0xFA00

        mov     si, 7C00h                   ; Copy code from 0x7C00 to 0xFA00
        mov     di, sp
        mov     cx, 0200h
        cld
        repz    movsb

        db      00E9H                       ; Make a relative jump
        dw      (0FA00h - 7C00h)

        mov     [bp + 101], cl              ; Clear Int13Ext flag

        mov     bx, 4 * 1Eh                 ; Update diskette parameter table
        lds     si, dword ptr [bx]
        push    ds
        push    si
        push    ss
        push    bp
        mov     di, 0FA5Ah                  ; Diskette Parameter Table
        mov     [bx], di
        mov     [bx + 2], cx
        mov     cl, 11
        cld
        repz    movsb
        push    es
        pop     ds

        mov     byte ptr [di - 2], 0Fh      ; Timeout 15 ms
        mov     ax, [bp + 24]               ; SectPerTrack
        mov     [di - 7], al                ; Update sectors per track

        mov     dl, [bp + 64]               ; DriveId
        cmp     dl, 80h
        jb      NoFixedDisk

        mov     ah, 41h                     ; Check for DDS extension
        mov     bx, 55AAh
        int     13h
        jc      NoInt13Ext
        cmp     bx, 0AA55h
        jnz     NoInt13Ext
        test    cl, 01h
        jnz     NoInt13Ext

        mov     [bp + 101], cl              ; Set Int13Ext flag
        jmp     NoFixedDisk

NoInt13Ext:
        mov     dl, [bp + 64]               ; DriveId
        mov     ah, 08h
        int     13h
        jc      DiskError
        xchg    dh, dl
        xor     dh, dh
        inc     dx
        mov     [bp + 26], dx               ; NumHeads
        and     cx, 003Fh
        mov     [bp + 24], cx               ; SectPerTrack

NoFixedDisk:

        xor     ah, ah
        mov     al, [bp + 16]               ; NumFATs
        mul     word ptr [bp + 36]          ; SectPerFATL
        mov     bx, ax
        mov     cx, dx
        mov     al, [bp + 16]               ; NumFATs
        mul     word ptr [bp + 38]          ; SectPerFATH
;       cmp     dx, 0
;       jnz     DiskError
        add     cx, ax
        mov     ax, bx
        mov     dx, cx
        add     ax, [bp + 28]               ; NumHiddenSectL
        adc     dx, [bp + 30]               ; NumHiddenSectH
        add     ax, [bp + 14]               ; RsvdSects
        adc     dx, 0
        mov     [bp + 102], ax              ; DataSectL
        mov     [bp + 104], dx              ; DataSectL

        mov     ax, [bp + 44]               ; FirstClusterL
        dec     ax
        dec     ax
        mul     byte ptr [bp + 13]          ; SectPerClust
        add     ax, [bp + 102]              ; DataSectL
        adc     dx, [bp + 104]              ; DataSectL

        mov     bx, 0500h                   ; Sector Buffer
        mov     cx, 1
        call    ReadSectors
        jc      DiskError

        mov     bx, 0500h                   ; Sector Buffer
        mov     ax, [bp + 11]               ; BytePerSect
        mov     cl, 05h
        shr     ax, cl
        mov     cx, ax                      ; Number of entries in sector

FindEntry:
        push    cx
        mov     di, bx
        mov     cx, 000Bh
        mov     si, offset ImageName
        repz    cmpsb
        jz      FoundFiles                  ; Found it?
        add     bx, 0020h                   ; Move to the next entry
        pop     cx
        loop    FindEntry

        ; A disk error has occurred - usually this is because
        ; we couldn't find the boot loader file on the disk.
        ;
DiskError:
        mov     si, offset ErrorMsg
        call    PrintMessage
        xor     ax, ax
        int     16h
        pop     bp                          ; Restore diskette parameter table
        pop     ds
        pop     [bp]
        pop     [bp + 2]
        int     19h

;
; We found our bootloader on the disk - now we need to load it.
; There are a couple of important things here:
;
; * The boot loader image must be in a contiguous grouping of sectors
;   on the disk.  This is because the bootloader doesn't contain
;   code to load the rest of itself, like DOS's IO.SYS does.
;   This requirement imposes a retriction which must be addressed
;   by the utility that writes the bootloader image to disk.
;   On a newly-formatted disk, empty disk, the main problem would
;   come from bad disk sectors.
;
; * The boot sector loads the bootloader at a predetermined address
;   (0x1000h). This location is compatible with the XLDR configuration
;   in xldr.bib.  As well, it allows for the BIOS interrupt vector
;   table to remain unaffected.
;
; * When the XLDR image is loaded, the boot sector code loads
;   a predetermined number of sectors from the storage device.
;   The number of sectors to be loaded must be set at build time
;   and if the XLDR image size changes, this number will need to be
;   updated (READ_LENGTH above).

FoundFiles:
        mov     ax, [bx + 1Ah]              ; Cluster
        dec     ax
        dec     ax
        mov     bl, [bp + 13]               ; SectPerClust
        xor     bh, bh
        mul     bx
        add     ax, [bp + 102]              ; DataSectL
        adc     dx, [bp + 104]              ; DataSectL
        mov     bx, LOAD_ADDRESS            ; Load to 0000:1000
        mov     cx, READ_LENGTH             ; This number of sectors
        call    ReadSectors
        JB      DiskError

        ; Jump to the bootloader image.
        DB      00EAh
        DW      LOAD_ADDRESS
        DW      0000h

        ;
        ; Display a text message using the BIOS INT 10h call.
        ;
PrintMessage:
        lodsb
        or      al, al
        jz      Return
        mov     ah, 0Eh
        mov     bx, 0007h
        int     10h
        jmp     PrintMessage

Return:
        RET

;-------------------------------------------------------------------------------
;
; Read disk sector(s).
;
;   Inputs:  DX:AX == logical sector #
;            CL == # sectors (CH == 0)
;            ES:BX == transfer address
;            BP == boot sector code
;
;   Outputs: DX:AX next logical sector #
;            CX == 0 (assuming no errors)
;            ES:BX -> byte after last byte of read
;            Carry set if error, else clear if success
;
;   Preserves: BP, SI, DI
;
ReadSectors:

        pusha                           ; save all registers

        OpPrefix                        ; push packet on stack
        push    0
        push    dx
        push    ax                      ; block number
        push    es
        push    bx                      ; transfer address
        push    1                       ; count of one, because we're looping
        push    16                      ; packet size

        xchg    ax, cx                  ; AX <-> CX

        mov     ax, [bp + 24]           ; Sectors per track
        xchg    ax, si                  ; save for divides

        xchg    ax, dx                  ; DX -> AX
        xor     dx, dx                  ; divide 0:AX
        div     si                      ; AX = high word of track

        xchg    ax, cx                  ; save AX in CX and restore old AX
        div     si                      ; CX:AX = track, DX = sector

        inc     dx                      ; sector is 1-based
        xchg    cx, dx                  ; CX = sector, DX = high word of track
        div     word ptr [bp + 26]      ; heads -> AX = cylinder, DX = head

        mov     dh, dl                  ; head # < 255

        mov     ch, al                  ; CH = cyl #
        ror     ah, 2                   ; move bits 8,9 of AX to bits 14,15
                                        ; (the rest has to be zero, since
                                        ;  cyls cannot be more than 10 bits)
        or      cl, ah                  ; CL = sector # plus high bits of cyl #
        mov     ax, 0201h               ; disk read 1 sector

        cmp     byte ptr [bp + 73], 0    ; should standard call be used?
        je      DoIo                    ; use standard calls
        mov     ah, 42h                 ; x13, we're ready to rock
        mov     si,sp                   ; DS:SI -> X13 packet

DoIo:
        mov     dl, [bp + 64]           ; DL == drive #
        int     13h

        popa                            ; throw away packet on stack (8 words)
        popa                            ; get real registers back
        jc      DoReadExit              ; disk error

        inc     ax
        jnz     DoReadNext
        inc     dx

DoReadNext:                             ; Adjust buffer address
        add     bx, [bp + 11]           ; BytesPerSector
        dec     cx
        jnz     ReadSectors

DoReadExit:
        ret

;-------------------------------------------------------------------------------

ImageName   db  'WCELDR     '
ErrorMsg    db  0Dh, 0Ah, 'Invalid system disk', 0Dh, 0Ah, 0
Padding     db  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            db  0,0,0,0,0,0,0,0,0;,0,0,0,0,0,0
Signature   db  55H, 0AAH

;-------------------------------------------------------------------------------

        .exit

        END

