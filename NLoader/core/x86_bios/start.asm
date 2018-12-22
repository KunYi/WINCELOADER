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

        .486p

        ASSUME  CS: FLAT, DS: FLAT, SS: FLAT
 
;-------------------------------------------------------------------------------

_TEXT   SEGMENT para public 'TEXT'

        extrn   _BootMain:near
        extrn   _pTOC:DWORD

        public  _BootStart

;-------------------------------------------------------------------------------

_BootStart PROC NEAR PUBLIC

        cli

        ; Set stack to top of data space
        mov     eax, DWORD PTR _pTOC
        mov     esp, DWORD PTR [eax + 28]

        ; Jump to common code
        jmp     _BootMain

_BootStart ENDP

;-------------------------------------------------------------------------------

_TEXT   ENDS

;-------------------------------------------------------------------------------

        END 
