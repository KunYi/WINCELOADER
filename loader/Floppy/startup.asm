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

.486p
.model FLAT
.code

;
; Include supporting macros
;
INCLUDE    bldr.inc

;
; Global defines
;
;
; Static GDT mapping:
;  - 0-4GB code region
;  - 0-64KB code region
;  - 0-4GB data address space
;  - 0-64KB data address space
;
GDT_Data LABEL DWORD
    db         0,    0,   0,   0,   0,         0,         0,   0    ; Unused
    CS_FLAT_SEL EQU ($-GDT_Data)
    db      0FFh, 0FFh, 00h, 00h, 00h, 10011010b, 11001111b, 00h    ; Code (4GB)
    CS_64K_SEL EQU ($-GDT_Data)
    db      0FFh, 0FFh, 00h, 00h, 00h, 10011010b, 00000000b, 00h    ; Code (64KB)
    DS_FLAT_SEL EQU ($-GDT_Data)
    db      0FFh, 0FFh, 00h, 00h, 00h, 10010010b, 11001111b, 00h    ; Data (4GB)
    DS_64K_SEL EQU ($-GDT_Data)
    db      0FFh, 0FFh, 00h, 00h, 00h, 10010010b, 00000000b, 00h    ; Data (64KB)
GDT_TABLE_SIZE = $ - OFFSET GDT_Data

;
; GDTR contents (size and GDT pointer)
;
GDTPtr LABEL FWORD
    dw      GDT_TABLE_SIZE - 1  ; Limit of 0 = 1 byte
    dd      OFFSET GDT_Data

;
; Mode switch functions.
;
PUBLIC     SwitchRM
PUBLIC     SwitchPM

;
; BIOS Parameter block storage.
;
_BPB BIOSPB <>
PUBLIC _BPB

;
; Address for storing FAT Data Area start LBA value.
;
_DataStartLBA LABEL DWORD
    dd      00000000h
PUBLIC _DataStartLBA

ifndef NOVIDEO
;
; VESA video mode information block
;
ModeInfoBlock   STRUCT
    wModeAttributes     dw ?
    ucWindowAAttributes     db ?
    ucWindowBAttributes     db ?
    wWindowGranularity      dw ?
    wWindowSize         dw ?
    wWindowASegment     dw ?
    wWindowBSegment     dw ?
    pWindowSchemeFunction   dd ?
    wBytesPerScanLine       dw ?
    wXResolution        dw ?
    wYResolution        dw ?
    ucXCharSize         db ?
    ucYCharSize         db ?
    ucNumberOfPlanes        db ?
    ucBitsPerPixel      db ?
    ucNumberOfBanks         db ?
    ucMemoryModel       db ?
    ucBankSize          db ?
    ucNumberOfImagePages    db ?
    ucReserved1         db ?
    ucRedMaskSize       db ?
    ucRedFieldPosition      db ?
    ucGreenMaskSize         db ?
    ucGreenFieldPosition    db ?
    ucBlueMaskSize      db ?
    ucBlueFieldPosition     db ?
    ucRsvdMaskSize      db ?
    ucRsvdFieldPosition     db ?
    ucDirectColorModeInfo   db ?
    dwPhysBasePtr       dd ?;
    ucReserved2         db 212 dup(?)
ModeInfoBlock   ENDS

;_minfo ModeInfoBlock <>
;PUBLIC _minfo

;
; Constants
;
VESAMODE480x200x256      EQU       0101h ; 480x200x256 emulated in 640x480
VESAMODE640x480x256      EQU       0101h ; 640x480x256
VESAMODE800x600x256      EQU       0103h ; 800x600x256
VESAMODE1024x768x16      EQU       0104h ; 1024x768x16
VESAMODE1024x768x256     EQU       0105h ; 1024x768x256
VESAMODE1024x768x64K     EQU       0111h ; 1024x768x16bpp

VESAMODE1024_SCANLINES   EQU       1024  ; Number of scan lines

VESALINEAR               EQU       4000h ; Linear frame buffer
endif    ; NOVIDEO


;
; Bootloader entrypoint
;
align 4

StartUp PROC NEAR C PUBLIC

    ;
    ; Note that the BIOS passes in the drive number and LBA of the root directory in CPU registers dl and ax & bx, so
    ; we need to be careful not to change these registers until we've had a chance to save their contents.
    ;
    ; DL - DriveID
    ; AX:BX - First Data Sector
    ;
    
    ;
    ; Set up the segments and initial stack value.
    ;
    xor    cx, cx
    mov    ss, cx
    OpPrefix
    mov    esp, STACK_BASE
    mov    ds, cx
    mov    es, cx
    mov    fs, cx
    mov    gs, cx

    ;
    ; Initialize the serial UART for debug output messages - needed as early as possible.
    ;
    OpPrefix
    call   InitDebugSerial

    ;
    ; The boot sector code gives us the data area start LBA in CPU registers AX & BX - save the value for later use.
    ;
    OpPrefix
    shl    eax, 10h
    OpPrefix
    and    ebx, 0FFFFh
    OpPrefix
    or     eax, ebx
    OpPrefix
    mov    ebx, OFFSET _DataStartLBA
    AddrPrefix
    mov    [ebx], eax

    ;
    ; We need to switch the CPU to protected mode in order to access all of
    ; the available RAM.  At a minimum, this involves loading the base address
    ; of the Global Descriptor Table (GDT), enabling the PE bit in CR0, and
    ; updating the selectors to offsets within the GDT.
    ;

    ;
    ; Configure CPU to use static GDT
    ;
    OpPrefix
    mov     eax, OFFSET GDTPtr      ; EAX -> Flat GDT Ptr address

    AddrPrefix
    OpPrefix
    lgdt    FWORD PTR cs:[eax]      ; load the GDTR

    ;
    ; Set the A20 line if it isn't already
    ;
LP1:
    in     al, 064h                    ; wait not busy
    test   al, 2                       ;
    jnz    LP1                         ;
    mov    al, 0D1h
    out    064h, al
LP2:
    in     al, 064h                    ; wait not busy
    test   al, 2                       ;
    jnz    LP2                         ;
    mov    al, 0DFh
    out    060h, al
    
    ;
    ; Switch to protected mode.
    ;
    OpPrefix
    call SwitchPM

    ;
    ; Configure cache (enable and set write-through mode)
    ;
    mov     ebx, OFFSET CacheEnabled
    mov     eax, cr0
    and     eax, not 060000000h     ; clear cache disable and write-through
    mov     cr0, eax
    jmp     ebx             ; jump to clear prefetch queue

align   4
CacheEnabled:
    wbinvd              ; clear out the cache

    ;
    ; Jump to C main routine...
    ;
    EXTRN   _blMain:FAR
    mov     eax, OFFSET _blMain
    jmp     eax

StartUp ENDP


;
; Initialize the COM1 UART for debug output messages (38400 N81).
;
align 4
InitDebugSerial PROC NEAR C PUBLIC
    push   ax
    push   dx

    mov    dh, 003h
    mov    dl, 0FBh
    mov    al, 080h                ; access Baud Divisor
    out    dx, al

    mov    dl, 0F8h
    mov    al, 003h                ; 38400 Baud
    out    dx, al

    mov    dl, 0F9h
    mov    al, 000h
    out    dx, al

    mov    dl, 0FAh
    mov    al, 007h                ; enable FIFO if present
    out    dx, al

    mov    dl, 0FBh
    mov    al, 003h                ; DLAB = 0, 8 bit, no parity
    out    dx, al

    mov    dl, 0F9h
    xor    al, al                  ; no interrupts, polled
    out    dx, al

    mov    dl, 0FCh
    mov    al, 003h                ; assert DTR, RTS
    out    dx, al

    pop    dx
    pop    ax

    ret    0
InitDebugSerial ENDP


; This function is entered in real mode and when we return, we'll be
; in protected mode.  It's the responsibility of the caller to ensure that any
; registers used in this function are saved.
;
; Affected registers: EAX and segment registers.
;
align 4
SwitchPM PROC NEAR C PUBLIC

    ; BIOS calls turn interrupts on, so we need to disable them here.
    cli

    ; Enable PE.
    mov     eax, cr0
    OpPrefix
    or      eax, 000000001h
    mov     cr0, eax

    ; ** We're assuming that the GDT base address has already been loaded in the GDTR.

    ; Far jump to clear the instruction queue and load the selector.
    OpPrefix
    db      0EAh
    dd      OFFSET PM_Cont
    dw      CS_FLAT_SEL

PM_Cont:

    ; Initialize the selectors.
    xor     eax, eax
    mov     ax, DS_FLAT_SEL
    mov     ss, ax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    ret    0
SwitchPM ENDP


; This function is entered in protected mode and when we return, we'll be
; in real mode.  It's the responsibility of the caller to ensure that any
; registers used in this function are saved.
;
; Affected registers: EAX and segment registers.
;
align 4
SwitchRM PROC NEAR C PUBLIC

    ; Disable interrupts.
    cli
    
    ; Load other selectors with 64KB-limited descriptors (this will load base and limits
    ; which are compatible with real mode).
    mov    ax, DS_64K_SEL
    mov    ss, ax
    mov    ds, ax
    mov    es, ax
    mov    fs, ax
    mov    gs, ax
    
    ; Transfer control to a 64KB-limited, 16-bit code segment.
    db      0EAh
    dd      OFFSET RM_Cont
    dw      CS_64K_SEL
    
RM_Cont:    

    ; Clear PE.
    mov    eax, cr0
    OpPrefix            ; Needed because we've changed to a 16-bit default code segment.
    and    eax, not 00000001h
    mov    cr0, eax

    ; Immediatly execute a far jump to flush the instruction pipeline.
    db      0EAh
    dd      OFFSET RM_Cont2
    dw      0000        ; CS = 0;
    
RM_Cont2:
   
    ; Fix the segment registers. 
    xor    eax, eax
    mov    ss, eax
    mov    ds, eax
    mov    es, eax
    mov    fs, eax
    mov    gs, eax
    
    ; Return (we're now back in real-mode).
    OpPrefix            ; Do a 32-bit near return (since we didn't fix up the stack on entry).
    ret    0
SwitchRM ENDP

END 

