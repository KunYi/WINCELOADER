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
#include <bootIo.h>

//------------------------------------------------------------------------------

uint8_t
READ_PORT_UCHAR(
    uint8_t *port
    )
{
    return (uint8_t)_inp((uint16_t)(DWORD)port);
}

//------------------------------------------------------------------------------

void
WRITE_PORT_UCHAR(
    uint8_t *port,
    uint8_t value
    )
{
    _outp((USHORT)(DWORD)port, (value));
}

//------------------------------------------------------------------------------

uint16_t
READ_PORT_USHORT(
    uint16_t *port
    )
{
    return _inpw((USHORT)(DWORD)port);
}

//------------------------------------------------------------------------------

void
WRITE_PORT_USHORT(
    uint16_t *port,
    uint16_t value
    )
{
    _outpw((USHORT)(DWORD)port, (value));
}

//------------------------------------------------------------------------------

uint32_t
READ_PORT_ULONG(
    uint32_t* port
    )
{
    return _inpd((USHORT)(DWORD)port);
}

//------------------------------------------------------------------------------

void
WRITE_PORT_ULONG(
    uint32_t *port,
    uint32_t value
    )
{
    _outpd((USHORT)(DWORD)port, (value));
}

//------------------------------------------------------------------------------

uint8_t
READ_REGISTER_UCHAR(
    volatile const uint8_t * const  Register
    )
{
    return (*(volatile uint8_t * const)Register);
}

//------------------------------------------------------------------------------

uint16_t
READ_REGISTER_USHORT(
    volatile const uint16_t * const Register
    )
{
    return (*(volatile uint16_t * const)Register);
}

//------------------------------------------------------------------------------

uint32_t
READ_REGISTER_ULONG(
    volatile const uint32_t * const  Register
    )
{
    return (*(volatile uint32_t * const)Register);
}

//------------------------------------------------------------------------------

void
WRITE_REGISTER_UCHAR(
    volatile uint8_t * const  Register,
    uint8_t   const Value
    )
{
    *(volatile uint8_t * const)Register = Value;
}

//------------------------------------------------------------------------------

void
WRITE_REGISTER_USHORT(
    volatile uint16_t * const Register,
    uint16_t  const Value
    )
{
    *(volatile uint16_t * const)Register = Value;
}

//------------------------------------------------------------------------------

void
WRITE_REGISTER_ULONG(
    volatile uint32_t * const  Register,
    uint32_t   const Value
    )
{
    *(volatile ULONG * const)Register = Value;
}

//------------------------------------------------------------------------------