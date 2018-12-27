;
;***********************************************************************************************************
;
;       Init.Asm - Boot Loader initialization code for x86 PC.
;


%define OpPrefix  db 66h

[BITS 16]

SECTION .TEXT

times      0x4000 db  0 
FLAT_STACK_START    equ     $        

;
;       This static GDT contains 2 selectors - A flat code selector, and a flat data selector.
;
align 4
GDT_Data:
        db      0,    0,   0,   0,   0,         0,         0,   0 ; First GDT entry always unused
CS_FLAT_SEL EQU ($-GDT_Data)
        db      0FFh, 0FFh, 00h, 00h, 00h, 10011010b, 11001111b, 00h ; Code
DS_FLAT_SEL EQU ($-GDT_Data)
        db      0FFh, 0FFh, 00h, 00h, 00h, 10010010b, 11001111b, 00h ; Data
GDT_TABLE_SIZE EQU ($ - GDT_Data)

;
;       Limit + Pointer to the GDT
;
align 8
GDTPtr:
        dw      GDT_TABLE_SIZE - 1  ; Limit of 0 = 1 byte
        dd      GDT_Data

PModeEntrySeg   dw      SEG InPModeNow

PModeEntryOff   dd      InPModeNow

        

;******************************************************************************
;
;       Launch
;
;       At this point, we are running in a 16-bit code segment.  
;       Since this code is written in a 16-bit code segment, we need to put an
;       OpPrefix in front of all 32-bit instructions.
;                       
;       This function jumps to InPModeNow to force CS to be reloaded with a
;       valid PMode selector.
;
;******************************************************************************
_Launch:
        cli                         ; Make sure we don't get any more interrupts

        mov     al, 0FFh            ; Disable all PIC interrupts
        out     021h, al

        ;
        ; Since we were loaded by the DOS 16bit real-mode loader we need to
        ; manually relocate all references to linear addresses before we switch
        ; to protect mode.
        ;

        xor     ebx, ebx            ; Clear upper word
        mov     bx, cs              ; Our segment 
        shl     ebx, 4              ; Convert segment to linear address

        mov     eax, DWORD [GDTPtr + 2]
        add     eax, ebx
        mov     DWORD [GDTPtr + 2], eax

        xor     eax, eax
        mov     ax, [PModeEntrySeg]
        shl     eax, 4
        add     eax, [PModeEntryOff]
        mov     [PModeLbl], eax

        pop     ax                  ; Remove return address from stack
        pop     edx                 ; Load entry point from arguments
        pop     esi                 ; Linear address of arguments

        OpPrefix
        lgdt    [GDTPtr]  ; Load the GDTR

        mov     ecx, FLAT_STACK_START
        add     ecx, ebx

    ;
    ;   Don't need OpPrefix on mov to/from CR0 -- It's always 32-bit
    ;
        mov     eax, cr0            ; Get the current CR0
        or      al, 1               ; Set the PE bit to enable protected mode
        mov     cr0, eax            ; NOW WE'RE IN PMODE!

        OpPrefix
        db      0EAh                ; Far jump forces a selector lookup
PModeLbl        dd      0
        dw      CS_FLAT_SEL

[BITS 32]
SECTION _TEXT32 
;***********************************************************************************************************
;
;       InPModeNow
;
;       This function is responsible for setting up the data selectors and the stack and then jumping to main.
;
;***********************************************************************************************************

InPModeNow:

        mov     eax, DS_FLAT_SEL
        mov     ds, eax
        mov     es, eax
        mov     fs, eax
        mov     gs, eax
        mov     ss, eax
        mov     esp, ecx

        push    edx

        mov     edx, 001FFFFCh
        mov     dword [edx], esi ; Save linear ptr to args in known location

        ret                         ; Jump to entry point    

