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


CODE_BASE       EQU     1000h       ; Where code will be copied
DATA_BASE       EQU     9000h       ; Use as buffer for some calls
STACK_BASE      EQU     0C8FCh      ; Stack reserved area is 0xB400 -> 0xC8FC

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

        PUBLIC     _BiosCallTable

_BiosCallTable LABEL DWORD

        DD      CODE_BASE                                               ; 0
        DD      DATA_BASE                                               ; 1
        DD      OFFSET BiosCodeEnd - OFFSET _BiosCallTable              ; 2
        DD      OFFSET BiosInt10 - OFFSET _BiosCallTable + CODE_BASE    ; 3
        DD      OFFSET BiosInt13 - OFFSET _BiosCallTable + CODE_BASE    ; 4
        DD      OFFSET BiosInt14 - OFFSET _BiosCallTable + CODE_BASE    ; 5
        DD      OFFSET BiosInt15 - OFFSET _BiosCallTable + CODE_BASE    ; 6
        DD      OFFSET BiosInt16 - OFFSET _BiosCallTable + CODE_BASE    ; 7
        DD      OFFSET BiosInt1A - OFFSET _BiosCallTable + CODE_BASE    ; 8

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
        dd      OFFSET PM_Cont - OFFSET _BiosCallTable + CODE_BASE
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
        dd      OFFSET RM_Cont - OFFSET _BiosCallTable + CODE_BASE
        dw      00000010h       ;CS_64K_SEL

RM_Cont:

        ; Clear PE
        mov     eax, cr0
        OpPrefix            ; We've changed to a 16-bit code segment.
        and     eax, not 00000001h
        mov     cr0, eax

        ; Immediatly execute a far jump to flush the instruction pipeline.
        db      0EAh
        dd      OFFSET RM_Cont2 - OFFSET _BiosCallTable + CODE_BASE
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
;
;  Function:  BiosInt10
;
        align 4

BiosInt10 PROC NEAR C PUBLIC

        push    ebp                     ; Set EBP to access arguments
        mov     ebp, esp
        add     ebp, 8                  ; Don't step on RA or EBP

        push    eax                     ; Save registers
        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi

        mov     edx, esp                ; Get actual SP
        mov     esp, STACK_BASE         ; SP must be in lower 64K
        push    edx                     ; Save old SP

        mov     edi, [ebp + 0]          ; EAX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 4]          ; EBX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 8]          ; ECX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 12]         ; EDX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 16]         ; ESI
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 20]         ; EDI
        mov     eax, [edi]
        push    eax
        

        call    SwitchRM                ; Switch to real mode

        OpPrefix                        ; Get parameters from stack
        pop     edi
        OpPrefix
        pop     esi
        OpPrefix
        pop     edx
        OpPrefix
        pop     ecx
        OpPrefix
        pop     ebx
        OpPrefix
        pop     eax
        
        int     10h                     ; Call BIOS

        OpPrefix                        ; Save results on stack
        push    eax                     
        OpPrefix
        push    ebx
        OpPrefix
        push    ecx
        OpPrefix
        push    edx
        OpPrefix
        push    esi
        OpPrefix
        push    edi
        
        OpPrefix
        call    SwitchPM                ; Switch back to protected mode

        mov     edi, [ebp + 20]         ; EDI
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 16]         ; ESI
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 12]         ; EDX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 8]          ; ECX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 4]          ; EBX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 0]          ; EAX
        pop     eax
        mov     [edi], eax

        pop     edx                     ; Restore stack
        mov     esp, edx

        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax

        pop     ebp

        ret     0

BiosInt10 ENDP

;-------------------------------------------------------------------------------
;
;  Function:  BiosInt13
;
        align 4

BiosInt13 PROC NEAR C PUBLIC

        push    ebp                     ; Set EBP to access arguments
        mov     ebp, esp
        add     ebp, 8                  ; Don't step on RA or EBP

        push    eax                     ; Save registers
        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi

        mov     edx, esp                ; Get actual SP
        mov     esp, STACK_BASE         ; SP must be in lower 64K
        push    edx                     ; Save old SP

        mov     edi, [ebp + 0]          ; EAX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 4]          ; EBX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 8]          ; ECX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 12]         ; EDX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 16]         ; ESI
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 20]         ; EDI
        mov     eax, [edi]
        push    eax
        

        call    SwitchRM                ; Switch to real mode

        OpPrefix                        ; Get parameters from stack
        pop     edi
        OpPrefix
        pop     esi
        OpPrefix
        pop     edx
        OpPrefix
        pop     ecx
        OpPrefix
        pop     ebx
        OpPrefix
        pop     eax
        
        int     13h                     ; Call BIOS

        OpPrefix                        ; Save results on stack
        push    eax                     
        OpPrefix
        push    ebx
        OpPrefix
        push    ecx
        OpPrefix
        push    edx
        OpPrefix
        push    esi
        OpPrefix
        push    edi
        
        OpPrefix
        call    SwitchPM                ; Switch back to protected mode

        mov     edi, [ebp + 20]         ; EDI
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 16]         ; ESI
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 12]         ; EDX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 8]          ; ECX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 4]          ; EBX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 0]          ; EAX
        pop     eax
        mov     [edi], eax

        pop     edx                     ; Restore stack
        mov     esp, edx

        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax

        pop     ebp

        ret     0

BiosInt13 ENDP

;-------------------------------------------------------------------------------
;
;  Function:  BiosInt14
;
        align 4

BiosInt14 PROC NEAR C PUBLIC

        push    ebp                     ; Set EBP to access arguments
        mov     ebp, esp
        add     ebp, 8                  ; Don't step on RA or EBP

        push    eax                     ; Save registers
        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi

        mov     edx, esp                ; Get actual SP
        mov     esp, STACK_BASE         ; SP must be in lower 64K
        push    edx                     ; Save old SP

        mov     edi, [ebp + 0]          ; EAX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 4]          ; EBX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 8]          ; ECX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 12]         ; EDX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 16]         ; ESI
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 20]         ; EDI
        mov     eax, [edi]
        push    eax
        

        call    SwitchRM                ; Switch to real mode

        OpPrefix                        ; Get parameters from stack
        pop     edi
        OpPrefix
        pop     esi
        OpPrefix
        pop     edx
        OpPrefix
        pop     ecx
        OpPrefix
        pop     ebx
        OpPrefix
        pop     eax
        
        int     14h                     ; Call BIOS

        OpPrefix                        ; Save results on stack
        push    eax                     
        OpPrefix
        push    ebx
        OpPrefix
        push    ecx
        OpPrefix
        push    edx
        OpPrefix
        push    esi
        OpPrefix
        push    edi
        
        OpPrefix
        call    SwitchPM                ; Switch back to protected mode

        mov     edi, [ebp + 20]         ; EDI
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 16]         ; ESI
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 12]         ; EDX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 8]          ; ECX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 4]          ; EBX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 0]          ; EAX
        pop     eax
        mov     [edi], eax

        pop     edx                     ; Restore stack
        mov     esp, edx

        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax

        pop     ebp

        ret     0

BiosInt14 ENDP
        
;-------------------------------------------------------------------------------
;
;  Function:  BiosInt15
;
        align 4

BiosInt15 PROC NEAR C PUBLIC

        push    ebp                     ; Set EBP to access arguments
        mov     ebp, esp
        add     ebp, 8                  ; Don't step on RA or EBP

        push    eax                     ; Save registers
        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi

        mov     edx, esp                ; Get actual SP
        mov     esp, STACK_BASE         ; SP must be in lower 64K
        push    edx                     ; Save old SP

        mov     edi, [ebp + 0]          ; EAX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 4]          ; EBX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 8]          ; ECX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 12]         ; EDX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 16]         ; ESI
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 20]         ; EDI
        mov     eax, [edi]
        push    eax
        

        call    SwitchRM                ; Switch to real mode

        OpPrefix                        ; Get parameters from stack
        pop     edi
        OpPrefix
        pop     esi
        OpPrefix
        pop     edx
        OpPrefix
        pop     ecx
        OpPrefix
        pop     ebx
        OpPrefix
        pop     eax
        
        int     15h                     ; Call BIOS

        OpPrefix                        ; Save results on stack
        push    eax                     
        OpPrefix
        push    ebx
        OpPrefix
        push    ecx
        OpPrefix
        push    edx
        OpPrefix
        push    esi
        OpPrefix
        push    edi
        
        OpPrefix
        call    SwitchPM                ; Switch back to protected mode

        mov     edi, [ebp + 20]         ; EDI
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 16]         ; ESI
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 12]         ; EDX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 8]          ; ECX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 4]          ; EBX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 0]          ; EAX
        pop     eax
        mov     [edi], eax

        pop     edx                     ; Restore stack
        mov     esp, edx

        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax

        pop     ebp

        ret     0

BiosInt15 ENDP
        
;-------------------------------------------------------------------------------
;
;  Function:  BiosInt16
;
        align 4

BiosInt16 PROC NEAR C PUBLIC

        push    ebp                     ; Set EBP to access arguments
        mov     ebp, esp
        add     ebp, 8                  ; Don't step on RA or EBP

        push    eax                     ; Save registers
        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi

        mov     edx, esp                ; Get actual SP
        mov     esp, STACK_BASE         ; SP must be in lower 64K
        push    edx                     ; Save old SP

        mov     edi, [ebp + 0]          ; EAX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 4]          ; EBX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 8]          ; ECX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 12]         ; EDX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 16]         ; ESI
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 20]         ; EDI
        mov     eax, [edi]
        push    eax
        

        call    SwitchRM                ; Switch to real mode

        OpPrefix                        ; Get parameters from stack
        pop     edi
        OpPrefix
        pop     esi
        OpPrefix
        pop     edx
        OpPrefix
        pop     ecx
        OpPrefix
        pop     ebx
        OpPrefix
        pop     eax
        
        int     16h                     ; Call BIOS

        OpPrefix                        ; Save results on stack
        push    eax                     
        OpPrefix
        push    ebx
        OpPrefix
        push    ecx
        OpPrefix
        push    edx
        OpPrefix
        push    esi
        OpPrefix
        push    edi
        
        OpPrefix
        call    SwitchPM                ; Switch back to protected mode

        mov     edi, [ebp + 20]         ; EDI
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 16]         ; ESI
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 12]         ; EDX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 8]          ; ECX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 4]          ; EBX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 0]          ; EAX
        pop     eax
        mov     [edi], eax

        pop     edx                     ; Restore stack
        mov     esp, edx

        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax

        pop     ebp

        ret     0

BiosInt16 ENDP

;-------------------------------------------------------------------------------
;
;  Function:  BiosInt1A
;
        align 4

BiosInt1A PROC NEAR C PUBLIC

        push    ebp                     ; Set EBP to access arguments
        mov     ebp, esp
        add     ebp, 8                  ; Don't step on RA or EBP

        push    eax                     ; Save registers
        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi

        mov     edx, esp                ; Get actual SP
        mov     esp, STACK_BASE         ; SP must be in lower 64K
        push    edx                     ; Save old SP

        mov     edi, [ebp + 0]          ; EAX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 4]          ; EBX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 8]          ; ECX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 12]         ; EDX
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 16]         ; ESI
        mov     eax, [edi]
        push    eax
        mov     edi, [ebp + 20]         ; EDI
        mov     eax, [edi]
        push    eax
        

        call    SwitchRM                ; Switch to real mode

        
        OpPrefix                        ; Get parameters from stack
        pop     edi
        OpPrefix
        pop     esi
        OpPrefix
        pop     edx
        OpPrefix
        pop     ecx
        OpPrefix
        pop     ebx
        OpPrefix
        pop     eax

        OpPrefix
        cmp     eax, 0B10Eh
        OpPrefix
        jnz     _1

        OpPrefix
        push    eax
        OpPrefix
        mov     eax, 0F000h
        mov     ds, eax
        OpPrefix
        pop     eax

_1:
        
        int     1Ah                     ; Call BIOS

        OpPrefix                        ; Save results on stack
        push    eax                     
        OpPrefix
        push    ebx
        OpPrefix
        push    ecx
        OpPrefix
        push    edx
        OpPrefix
        push    esi
        OpPrefix
        push    edi
        
        OpPrefix
        call    SwitchPM                ; Switch back to protected mode

        mov     edi, [ebp + 20]         ; EDI
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 16]         ; ESI
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 12]         ; EDX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 8]          ; ECX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 4]          ; EBX
        pop     eax
        mov     [edi], eax
        mov     edi, [ebp + 0]          ; EAX
        pop     eax
        mov     [edi], eax

        pop     edx                     ; Restore stack
        mov     esp, edx

        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax

        pop     ebp

        ret     0

BiosInt1A ENDP

;-------------------------------------------------------------------------------

BiosCodeEnd:

;-------------------------------------------------------------------------------

        END
