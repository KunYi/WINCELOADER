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

OpPrefix MACRO
        db      66h
        ENDM

AddrPrefix MACRO
        db      67h
        ENDM

;-------------------------------------------------------------------------------

        .486p
        .model FLAT

        .code

;-------------------------------------------------------------------------------

STACK_BASE  EQU     0C8FCh      ; Reserved in boot.bib
STACK_SIZE  EQU     1500h

;-------------------------------------------------------------------------------
;
; Global defines
;
; Static GDT mapping:
;  - 0-4GB code region
;  - 0-64KB code region
;  - 0-4GB data address space
;  - 0-64KB data address space
;
GDT_Data LABEL DWORD
        db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h          ; Unused
CS_FLAT_SEL EQU ($ - GDT_Data)
        db      0FFh, 0FFh, 00h, 00h, 00h, 9Ah, 0CFh, 00h       ; Code (4GB)
CS_64K_SEL  EQU ($ - GDT_Data)
        db      0FFh, 0FFh, 00h, 00h, 00h, 9Ah, 00h, 00h        ; Code (64KB)
DS_FLAT_SEL EQU ($ - GDT_Data)
        db      0FFh, 0FFh, 00h, 00h, 00h, 92h, 0CFh, 00h       ; Data (4GB)
DS_64K_SEL  EQU ($ - GDT_Data)
        db      0FFh, 0FFh, 00h, 00h, 00h, 92h, 00h, 00h        ; Data (64KB)
GDT_TABLE_SIZE  = $ - OFFSET GDT_Data

;
; GDTR contents (size and GDT pointer)
;
GDTPtr LABEL FWORD
        dw      GDT_TABLE_SIZE - 1                  ; Limit of 0 = 1 byte
        dd      OFFSET GDT_Data

;-------------------------------------------------------------------------------
;
;  Function: StartUp
;
        align 4

StartUp PROC NEAR C PUBLIC

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
        ; We need to switch the CPU to protected mode in order to access all of
        ; the available RAM.  At a minimum, this involves loading the base address
        ; of the Global Descriptor Table (GDT), enabling the PE bit in CR0, and
        ; updating the selectors to offsets within the GDT.
        ;

        ;
        ; Configure CPU to use static GDT
        ;
        OpPrefix
        mov     eax, OFFSET GDTPtr          ; EAX -> Flat GDT Ptr address

        AddrPrefix
        OpPrefix
        lgdt    FWORD PTR cs:[eax]          ; load the GDTR

        ;
        ; Set the A20 line if it isn't already
        ;
;LP1:
;        in     al, 064h                    ; wait not busy
;        test   al, 2
;        jnz    LP1
;        mov    al, 0D1h
;        out    064h, al

;LP2:
;        in     al, 064h                    ; wait not busy
;        test   al, 2
;        jnz    LP2
;        mov    al, 0DFh
;        out    060h, al

        ;
        ; Switch to protected mode.
        ;
        OpPrefix
        call    SwitchPM

        ;
        ; Configure cache (enable and set write-through mode)
        ;
        mov     ebx, OFFSET CacheEnabled
        mov     eax, cr0
        and     eax, not 060000000h     ; clear cache disable and write-through
        mov     cr0, eax
        jmp     ebx                     ; jump to clear prefetch queue

        align   4

CacheEnabled:
        wbinvd                          ; clear out the cache

        ;
        ; Jump to C main routine...
        ;
        EXTRN   _XldrMain:FAR
        mov     eax, OFFSET _XldrMain
        jmp     eax

StartUp ENDP

;-------------------------------------------------------------------------------
;
;  Function: SwitchPM
;
;  This function is entered in real mode and when we return, we'll be
;  in protected mode.  It's the responsibility of the caller to ensure that
;  any registers used in this function are saved.
;
;  We're assuming that the GDT base address has already been loaded in the GDTR.
;
;  Affected registers: EAX and segment registers.
;
        align 4

SwitchPM PROC NEAR C PUBLIC

        ; BIOS calls turn interrupts on, so we need to disable them here.
        cli

        ; Enable PE
        mov     eax, cr0
        OpPrefix
        or      eax, 000000001h
        mov     cr0, eax

        ; Far jump to clear the instruction queue and load the selector.
        OpPrefix
        db      0EAh
        dd      OFFSET PM_Cont
        dw      0008h           ;CS_FLAT_SEL

PM_Cont:

        ; Initialize the selectors.
        xor     eax, eax
        mov     ax, 0018h       ;DS_FLAT_SEL
        mov     ss, ax
        mov     ds, ax
        mov     es, ax
        mov     fs, ax
        mov     gs, ax

        ret    0

SwitchPM ENDP

;-------------------------------------------------------------------------------
;
; Function: SwitchPM
;
; This function is entered in protected mode and when we return, we'll be
; in real mode.  It's the responsibility of the caller to ensure that any
; registers used in this function are saved.
;
; Affected registers: EAX and segment registers.
;
        align 4

SwitchRM PROC NEAR C PUBLIC

        ; Disable interrupts
        cli

        ; Load other selectors with 64KB-limited descriptors
        ; (this will load base and limits which are compatible with real
        ; mode).
        mov     ax, 0020h        ;DS_64K_SEL
        mov     ss, ax
        mov     ds, ax
        mov     es, ax
        mov     fs, ax
        mov     gs, ax

        ; Transfer control to a 64KB-limited, 16-bit code segment.
        db      0EAh
        dd      OFFSET RM_Cont
        dw      00000010h       ;CS_64K_SEL

RM_Cont:

        ; Clear PE
        mov     eax, cr0
        OpPrefix            ; We've changed to a 16-bit code segment.
        and     eax, not 00000001h
        mov     cr0, eax

        ; Immediatly execute a far jump to flush the instruction pipeline.
        db      0EAh
        dd      OFFSET RM_Cont2
        dw      0000            ; CS = 0

RM_Cont2:

        ; Fix segment registers
        xor    eax, eax
        mov    ss, eax
        mov    ds, eax
        mov    es, eax
        mov    fs, eax
        mov    gs, eax

        ; Return (we're now back in real-mode). Do a 32-bit near return
        ; since we didn't fix up the stack on entry.
        OpPrefix
        ret    0

SwitchRM ENDP

;-------------------------------------------------------------------------------

        END
