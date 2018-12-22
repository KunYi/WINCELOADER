//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
#ifndef __BOOT_IO_H
#define __BOOT_IO_H

#include <bootTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// Undefine conflicting macros in case where oal_io.h was previously included
#ifdef __OAL_IO_H
#undef INPORT8
#undef INPORT16
#undef INPORT32
#undef INREG8
#undef INREG16
#undef INREG32
#undef OUTPORT8
#undef OUTPORT16
#undef OUTPORT32
#undef OUTREG8
#undef OUTREG16
#undef OUTREG32
#else
#define __OAL_IO_H
#endif

//------------------------------------------------------------------------------
//
//  Macros:  INPORTx/OUTPORTx/SETPORTx/CLRPORTx
//
//  This macros encapsulates basic I/O operations. Depending on BOOT_IO_NOMACRO
//  definition they will expand to direct memory operation or function call.
//  On x86 platform IO address space operation is generated. On other platforms
//  operation is identical with INREGx/OUTREGx/SETREGx/CLRREGx.
//
#ifndef BOOT_NO_IOMACRO

#if defined(x86)

int 
__cdecl 
_inp(
    unsigned short
    );

unsigned short
__cdecl 
_inpw(
    unsigned short
    );

unsigned long 
__cdecl 
_inpd(
    unsigned short
    );

int 
__cdecl 
_outp(
    unsigned short, 
    int
    );

unsigned short 
__cdecl 
_outpw(
    unsigned short, 
    unsigned short
    );

unsigned long 
 __cdecl
_outpd(
    unsigned short, 
    unsigned long
    );

__inline
uint8_t
INPORT8(
    uint8_t *port
    )
{
    return (uint8_t)_inp((uint16_t)(DWORD)port);
}

__inline
void
OUTPORT8(
    uint8_t *port,
    uint8_t value
    )
{
    _outp((USHORT)(DWORD)port, (value));
}

__inline
uint16_t
INPORT16(
    uint16_t *port
    )
{
    return _inpw((USHORT)(DWORD)port);
}

void
__inline
OUTPORT16(
    uint16_t *port,
    uint16_t value
    )
{
    _outpw((USHORT)(DWORD)port, (value));
}

__inline
uint32_t
INPORT32(
    uint32_t* port
    )
{
    return _inpd((USHORT)(DWORD)port);
}

void
__inline
OUTPORT32(
    uint32_t *port,
    uint32_t value
    )
{
    _outpd((USHORT)(DWORD)port, (value));
}

#define INREG8(x)           (*(volatile uint8_t * const)(x))
#define OUTREG8(x, y)       (*(volatile uint8_t * const)(x)) = (y)
#define INREG16(x)          (*(volatile uint16_t * const)(x))
#define OUTREG16(x, y)      (*(volatile uint16_t * const)(x)) = (y)
#define INREG32(x)          (*(volatile uint32_t * const)(x))
#define OUTREG32(x, y)      (*(volatile uint32_t * const)(x)) = (y)

#elif defined(MIPS) || defined(ARM)

#define INPORT8(x)          (*(volatile uint8_t * const)(x))
#define OUTPORT8(x, y)      (*(volatile uint8_t * const)(x)) = (y)
#define INPORT16(x)         (*(volatile uint16_t * const)(x))
#define OUTPORT16(x, y)     (*(volatile uint16_t * const)(x)) = (y)
#define INPORT32(x)         (*(volatile uint32_t * const)(x))
#define OUTPORT32(x, y)     (*(volatile uint32_t * const)(x)) = (y)

#define INREG8(x)           (*(volatile uint8_t * const)(x))
#define OUTREG8(x, y)       (*(volatile uint8_t * const)(x)) = (y)
#define INREG16(x)          (*(volatile uint16_t * const)(x))
#define OUTREG16(x, y)      (*(volatile uint16_t * const)(x)) = (y)
#define INREG32(x)          (*(volatile uint32_t * const)(x))
#define OUTREG32(x, y)      (*(volatile uint32_t * const)(x)) = (y)

#elif defined(SHx)

#define INPORT8(x)          BootInPort8(x)
#define OUTPORT8(x, y)      BootOutPort8(x, y)
#define INPORT16(x)         BootInPort16(x)
#define OUTPORT16(x, y)     BootOutPort16(x, y)
#define INPORT32(x)         BootInPort32(x)
#define OUTPORT32(x, y)     BootOutPort32(x, y)

#define INREG8(x)           (*(volatile uint8_t * const)(x))
#define OUTREG8(x, y)       (*(volatile uint8_t * const)(x)) = (y)
#define INREG16(x)          (*(volatile uint16_t * const)(x))
#define OUTREG16(x, y)      (*(volatile uint16_t * const)(x)) = (y)
#define INREG32(x)          (*(volatile uint32_t * const)(x))
#define OUTREG32(x, y)      (*(volatile uint32_t * const)(x)) = (y)

#endif

#else

#define INPORT8(x)          BootInPort8(x)
#define OUTPORT8(x, y)      BootOutPort8(x, y)
#define INPORT16(x)         BootInPort16(x)
#define OUTPORT16(x, y)     BootOutPort16(x, y)
#define INPORT32(x)         BootInPort32(x)
#define OUTPORT32(x, y)     BootOutPort32(x, y)
#define INREG8(x)           BootInReg8(x)
#define OUTREG8(x, y)       BootOutReg8(x, y)
#define INREG16(x)          BootInReg16(x)
#define OUTREG16(x, y)      BootOutReg16(x, y)
#define INREG32(x)          BootInReg32(x)
#define OUTREG32(x, y)      BootOutReg32(x, y)

uint8_t
BootInPort8(
    uint8_t *port
    );

void
BootOutPort8(
    uint8_t *port,
    uint8_t value
    );

uint16_t
BootInPort16(
    uint16_t *port
    );

void
BootOutPort16(
    uint16_t *port,
    uint16_t value
    );

uint32_t
BootInPort32(
    uint32_t *port
    );

void
BootOutPort32(
    uint32_t *port,
    uint32_t value
    );

uint8_t
BootInReg8(
    uint8_t *port
    );

void
BootOutReg8(
    uint8_t *port,
    uint8_t value
    );

uint16_t
BootInReg16(
    uint16_t *port
    );

void
BootOutReg16(
    uint16_t *port,
    uint16_t value
    );

uint32_t
BootInReg32(
    uint32_t *port
    );

void
BootOutReg32(
    uint32_t *port,
    uint32_t value
    );

#endif

//------------------------------------------------------------------------------

#define SETPORT8(x, y)          OUTPORT8(x, INPORT8(x)|(y))
#define CLRPORT8(x, y)          OUTPORT8(x, INPORT8(x)&~(y))
#define MASKPORT8(x, y, z)      OUTPORT8(x, (INPORT8(x)&~(y))|(z))

#define SETPORT16(x, y)         OUTPORT16(x, INPORT16(x)|(y))
#define CLRPORT16(x, y)         OUTPORT16(x, INPORT16(x)&~(y))
#define MASKPORT16(x, y, z)     OUTPORT16(x, (INPORT16(x)&~(y))|(z))

#define SETPORT32(x, y)         OUTPORT32(x, INPORT32(x)|(y))
#define CLRPORT32(x, y)         OUTPORT32(x, INPORT32(x)&~(y))
#define MASKPORT32(x, y, z)     OUTPORT32(x, (INPORT32(x)&~(y))|(z))

#define SETREG8(x, y)           OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)           OUTREG8(x, INREG8(x)&~(y))
#define MASKREG8(x, y, z)       OUTREG8(x, (INREG8(x)&~(y))|(z))

#define SETREG16(x, y)          OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)          OUTREG16(x, INREG16(x)&~(y))
#define MASKREG16(x, y, z)      OUTREG16(x, (INREG16(x)&~(y))|(z))

#define SETREG32(x, y)          OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y)          OUTREG32(x, INREG32(x)&~(y))
#define MASKREG32(x, y, z)      OUTREG32(x, (INREG32(x)&~(y))|(z))

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_IO_H
