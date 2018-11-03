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

        extrn       _SwitchRM:near
        extrn       _SwitchPM:near

        .code

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
        

        call    _SwitchRM                ; Switch to real mode

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
        call    _SwitchPM                ; Switch back to protected mode

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
        

        call    _SwitchRM                ; Switch to real mode

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
        call    _SwitchPM                ; Switch back to protected mode

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

        call    _SwitchRM                ; Switch to real mode

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
        call    _SwitchPM                ; Switch back to protected mode

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

        call    _SwitchRM                ; Switch to real mode

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
        call    _SwitchPM                ; Switch back to protected mode

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

        call    _SwitchRM                ; Switch to real mode

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
        call    _SwitchPM                ; Switch back to protected mode

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

        call    _SwitchRM                ; Switch to real mode
        
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
        call    _SwitchPM                ; Switch back to protected mode

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

        END
