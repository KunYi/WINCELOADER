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
INCLUDE     bldr.inc
            
extrn       _SwitchRM:near
extrn       _SwitchPM:near

;
; BIOS_ReadSectorEx
;
; Parameters
;     UCHAR DriveId
;     READ_PACKET* pPacket
;
; Note that the stack is in the first 64k and so referencing stack addresses
; with 16 bits is ok.
;
;
align 4
BIOS_ReadSectorsEx  PROC NEAR C PUBLIC

    ; Set EBP to allow access to callers arguments
    push    ebp
    mov     ebp, esp
    add     ebp, 8                  ; Don't step on RA or EBP

    ; When we switch to real mode, the callee doesn't save registers - do
    ; that here. We won't save eax since it's used for the return value to the
    ; caller.
    push    ebx
    push    ecx
    push    edx
    push    edi
    push    esi

    ; Switch to real mode
    call    _SwitchRM

    ; The assembler thinks that we are in 32-bit mode and will generate 
    ; instructions without prefixes for any 32-bit register that we use.
    ; Since the EU is running in 16-bit mode and we don't have prefixes,
    ; it will use 16-bit values.  Use the OpPrefix and AddrPrefix macros
    ; to generate 32-bit values where appropriate.
    OpPrefix
    AddrPrefix
    mov     ebx, [ebp+4]                ; This is the address of the packet
    OpPrefix
    add     ebx, 4                      ; This is the offset from the packet to the buffer.
    OpPrefix
    AddrPrefix
    mov     edx, [ebx]                  ; Get the original buffer location from the packet
    OpPrefix
    push    edx                         ; Save the buffer location

    OpPrefix
    AddrPrefix
    mov     [ebx], RW_BUFFER_START      ; We will read into this buffer to keep
                                        ; it under 64k at all times for real mode.

    OpPrefix
    xor     eax, eax

    AddrPrefix                          ; The assembler still thinks in 32-bit mode, so this will
    mov     esi, [ebp+4]                ; only move 16 bits correctly.

    AddrPrefix
    mov     dl, [ebp]               ; drive number into DL
    mov     ah, 42H
    int     13H

    ; Save return status.
    OpPrefix
    push    eax

    ; Switch back to protected mode
    OpPrefix
    call    _SwitchPM

    ; Restore results (from stack) of BIOS call HERE
    pop     eax
    pop     edi                         ; The saved buffer location
    ror     eax, 8
    and     eax, 0ffh
    cmp     eax, 0
    ; If an error condition occurred - return error (in AL).
    ; Note - don't touch al after this point - it's returned to the caller...
    jnz      RSX_Done

RSX_Copy:    
    ; Copy the results to the callers buffer.
    mov     esi, RW_BUFFER_START
    cmp     edi, esi
    je      RSX_Done                    ; Optimization - don't copy if we don't have to.
    mov     ecx, SECTOR_SIZE
    rep     movsb
    
RSX_Done:   
    pop     esi
    pop     edi
    pop     edx
    pop     ecx
    pop     ebx
    pop     ebp 
    
    ret     0
BIOS_ReadSectorsEx ENDP

;
; BIOS_WriteSectorEx
;
; Parameters
;     UCHAR DriveId
;     READ_PACKET* pPacket
;
; Note that the stack is in the first 64k and so referencing stack addresses
; with 16 bits is ok.
;
;
align 4
BIOS_WriteSectorsEx  PROC NEAR C PUBLIC

    ; Set EBP to allow access to callers arguments
    push    ebp
    mov     ebp, esp
    add     ebp, 8                      ; Don't step on RA or EBP

    ; When we switch to real mode, the callee doesn't save registers - do
    ; that here. We won't save eax since it's used for the return value to the
    ; caller.
    push    ebx
    push    ecx
    push    edx
    push    edi
    push    esi

    ; Switch to real mode
    call    _SwitchRM

    OpPrefix
    xor     eax, eax

    AddrPrefix                          ; The assembler still thinks in 32-bit mode, so this will
    mov     esi, [ebp+4]                ; only move 16 bits correctly.

    AddrPrefix
    mov     dl, [ebp]                   ; drive number into DL
    mov     ah, 43H
    int     13H

    ; Save return status.
    OpPrefix
    push    eax

    ; Switch back to protected mode
    OpPrefix
    call    _SwitchPM

    ; Restore results (from stack) of BIOS call HERE
    pop     eax
    ror     eax, 8
    and     eax, 0ffh
    cmp     eax, 0
    ; If an error condition occurred - return error (in AL).
    ; Note - don't touch al after this point - it's returned to the caller
    pop     esi
    pop     edi
    pop     edx
    pop     ecx
    pop     ebx
    pop     ebp 
    
    ret     0
BIOS_WriteSectorsEx ENDP
;
; Reset disk system.
;
; Returns error status in eax: zero for success, non-zero for failure.
;
align 4
BIOS_ResetDisk PROC NEAR C PUBLIC

    ; Set EBP to allow access to callers arguments
    push    ebp
    mov     ebp, esp
    add     ebp, 8                  ; Don't step on RA or EBP

    ; When we switch to real mode, the callee doesn't save registers - do
    ; that here. We won't save eax since it's used for the return value 
    ; to the caller.
    push    edx
    
    ; Switch to real mode
    call    _SwitchRM

    ; Make BIOS call HERE
    OpPrefix
    xor     eax, eax
    mov     ah, 00                  ; reset disk command
    mov     dl, [ebp]               ; drive number to reset
    mov     al, dl      
    int     13h
    
    ; Save return status.
    OpPrefix
    push    eax
    
    ; Switch back to protected mode.
    OpPrefix
    call    _SwitchPM

    ; Restore results (from stack) of BIOS call HERE.
    pop     eax
    and     eax, 0ffh
    ; Note - don't touch al after this point - it's error status to be  returned to the caller...
 
    ; Restore registers from stack. 
    pop     edx  
    pop     ebp 
    
    ret     0
BIOS_ResetDisk ENDP

;
; Read VBE Controller Information
;
; Returns error status in eax: zero for success, non-zero for failure.
;
align 4
BIOS_VBEControllerInfo PROC NEAR C PUBLIC

    ; Save registers (except for eax which will contain the return value)
    push    ecx
    push    edi

;VCI_NoSrcCopy:

    ; Switch to real mode.
    call    _SwitchRM

    ; Call function 00h
    OpPrefix
    mov     eax, 4F00h 
    OpPrefix
    mov     edi, VESA_GENINFO_START
    int     10h                       ; check for VESA

    ; Save return status.
    OpPrefix
    push    eax
    
    ; Switch back to protected mode.
    OpPrefix
    call    _SwitchPM

;VCI_NoDstCopy:   

    ; Restore results (from stack) of BIOS call
    pop     eax
    and     eax, 0000ffffh

    pop     edi
    pop     ecx
    
    ret     0
    
BIOS_VBEControllerInfo ENDP

;
; Read VBE Mode Information
;
; Returns error status in eax: zero for success, non-zero for failure.
;
align 4
BIOS_VBEModeInfo PROC NEAR C PUBLIC

    ; Set EBP to allow access to callers arguments
    push    ebp
    mov     ebp, esp
    add     ebp, 8                  ; Don't step on RA or EBP

    ; Save registers (except for eax which will contain the return value)
    push    ecx
    push    edi
    push    esi

    ; Switch to real mode.
    call    _SwitchRM

    ; Call function 01h
    OpPrefix
    mov     eax, 4F01h 
    AddrPrefix
    mov     ecx, [ebp + 4]
    OpPrefix
    mov     edi, VESA_WORK_RAM_START     ; Pointer to info buffer
    int     10h                     ; check for VESA

    ; Save return status.
    OpPrefix
    push    eax
    
    ; Switch back to protected mode.
    OpPrefix
    call    _SwitchPM

    ; Copy the results to the callers buffer.
    mov     esi, VESA_WORK_RAM_START
    mov     edi, [ebp]
    cmp     edi, esi
    je      VMI_NoDstCopy           ; Optimization - don't copy if we don't have to
    mov     ecx, 100h
    rep     movsb
    
VMI_NoDstCopy:   

    ; Restore results (from stack) of BIOS call
    pop     eax
    and     eax, 0000ffffh

    pop     esi
    pop     edi
    pop     ecx

    pop     ebp

    ret     0
    
BIOS_VBEModeInfo ENDP

;
; Set VBE Mode
;
; Returns error status in eax: zero for success, non-zero for failure.
;
align 4
BIOS_SetVBEMode PROC NEAR C PUBLIC

    ; Set EBP to allow access to callers arguments
    push    ebp
    mov     ebp, esp
    add     ebp, 8                  ; Don't step on RA or EBP

    ; Save registers (except for eax which will contain the return value)
    push    ebx

    ; Switch to real mode
    call    _SwitchRM

    ; Call function 02h
    OpPrefix
    mov     eax, 4F02h 
    AddrPrefix
    mov     ebx, [ebp]
    int     10h      

    ; Save return status
    OpPrefix
    push    eax
    
    ; Switch back to protected mode
    OpPrefix
    call    _SwitchPM

    ; Restore results (from stack) of BIOS call
    pop     eax
    and     eax, 0000ffffh

    pop     ebx

    pop     ebp

    ret     0
    
BIOS_SetVBEMode ENDP

;
; Set Palette Data
;
; Returns error status in eax: zero for success, non-zero for failure.
;
align 4
BIOS_SetDACPaletteData PROC NEAR C PUBLIC

    ; Set EBP to allow access to callers arguments
    push    ebp
    mov     ebp, esp
    add     ebp, 8                  ; Don't step on RA or EBP

    ; Save registers (except for eax which will contain the return value)
    push    ebx
    push    ecx
    push    edx
    push    edi
    push    esi

    ; Copy colour table to our work memory
    mov     esi, [ebp+8]
    mov     edi, VESA_WORK_RAM_START
    cmp     edi, esi
    je      SDP_NoSrcCopy           ; Optimization - don't copy if we don't have to
    mov     ecx, [ebp+4]
    rep     movsd
    
SDP_NoSrcCopy:

    ; Switch to real mode
    call    _SwitchRM

    ; Call function 02h
    OpPrefix
    mov     eax, 4F09h 
    OpPrefix
    mov     ebx, 00h                ; Set DAC palette data
    AddrPrefix
    mov     ecx, [ebp+4]            ; count
    AddrPrefix
    mov     edx, [ebp]              ; start
    OpPrefix
    mov     edi, VESA_WORK_RAM_START     ; Pointer to info buffer
    int     10h      

    ; Save return status
    OpPrefix
    push    eax
    
    ; Switch back to protected mode
    OpPrefix
    call    _SwitchPM

    ; Restore results (from stack) of BIOS call
    pop     eax
    and     eax, 0000ffffh

    pop     esi
    pop     edi
    pop     edx
    pop     ecx
    pop     ebx

    pop     ebp

    ret     0
    
BIOS_SetDACPaletteData ENDP

;
; Set VGA video mode
;
align 4
BIOS_SetVideoMode PROC NEAR C PUBLIC

    ; Set EBP to allow access to callers arguments
    push    ebp
    mov     ebp, esp
    add     ebp, 8                  ; Don't step on RA or EBP

    ; Switch to real mode
    call    _SwitchRM

    ; Call function 00h
    mov     ah, 00h
    AddrPrefix
    mov     al, [ebp]               ; Video mode
    int     10h      

    ; Save return status
    OpPrefix
    push    eax
    
    ; Switch back to protected mode
    OpPrefix
    call    _SwitchPM

    ; Restore results (from stack) of BIOS call
    pop     eax
    and     eax, 0000ffffh

    pop     ebp

    ret     0
    
BIOS_SetVideoMode ENDP

;
; Read keyboard character. Read keyboard status first. If there is a
; character in the buffer read it.
;
align 4
BIOS_ReadKeyboardCharacter PROC NEAR C PUBLIC

    ; Switch to real mode
    call    _SwitchRM

    ; Call function 01h to get the status
    mov     ah, 01h
    int     16h      

    ; zero flag set? if so - no key waiting
    jz      no_key

    ; Call function 00h to get the key from the buffer
    mov     ah, 00h
    int     16h      
    jmp     key_waiting

no_key:
    OpPrefix
    xor     eax,eax

key_waiting:

    ; Save return status
    OpPrefix
    push    eax
    
    ; Switch back to protected mode
    OpPrefix
    call    _SwitchPM

    ; Restore results (from stack) of BIOS call
    pop     eax

    ret     0
    
BIOS_ReadKeyboardCharacter ENDP

;
; Writes a character in teletype mode
;
align 4
BIOS_WriteCharacterTeletype PROC NEAR C PUBLIC

    ; Set EBP to allow access to callers arguments
    push    ebp
    mov     ebp, esp
    add     ebp, 8                  ; Don't step on RA or EBP

    push    ebx
    push    ecx
    push    edx

    ; Switch to real mode
    call    _SwitchRM

    ; Call function 0eh to write a character
    mov     ah, 0eh
    AddrPrefix
    mov     bh, [ebp]               ; page
    AddrPrefix
    mov     bl, [ebp+4]             ; color
    AddrPrefix
    mov     al, [ebp+8]             ; character
    int     10h

    ; Switch back to protected mode
    OpPrefix
    call    _SwitchPM

    pop     edx
    pop     ecx
    pop     ebx

    pop     ebp

    ret     0
    
BIOS_WriteCharacterTeletype ENDP

;
; Delay - suspends program execution for 100ms
;
align 4
BIOS_Delay PROC NEAR C PUBLIC

    push    ecx
    push    edx

    ; Switch to real mode
    call    _SwitchRM

    ; Call function 86h for delay
    OpPrefix
    mov     edx, 86a0h
    OpPrefix
    mov     ecx, 0001h  ; 100ms
    mov     ah, 86h
    int     15h

    ; Switch back to protected mode
    OpPrefix
    call    _SwitchPM

    pop     edx
    pop     ecx

    ret     0
    
BIOS_Delay ENDP

END
