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
#ifndef __BOOT_SERIAL_H
#define __BOOT_SERIAL_H
#include <bootIO.h>

#ifdef __cplusplus
extern "C" {
#endif

// Some Serial port defines used for the debug serial routines
#define COM1_BASE           0x03F8
#define COM2_BASE           0x02F8
#define COM3_BASE           0x03E8
#define COM4_BASE           0x02E8

#define comTxBuffer         0x00
#define comRxBuffer         0x00
#define comDivisorLow       0x00
#define comDivisorHigh      0x01
#define comIntEnable        0x01
#define comIntId            0x02
#define comFIFOControl      0x02
#define comLineControl      0x03
#define comModemControl     0x04
#define comLineStatus       0x05
#define comModemStatus      0x06

enum LS 
{
    LS_TSR_EMPTY       = 0x40,
    LS_THR_EMPTY       = 0x20,
    LS_RX_BREAK        = 0x10,
    LS_RX_FRAMING_ERR  = 0x08,
    LS_RX_PARITY_ERR   = 0x04,
    LS_RX_OVERRUN      = 0x02,
    LS_RX_DATA_READY   = 0x01,

    LS_RX_ERRORS       = ( LS_RX_FRAMING_ERR | LS_RX_PARITY_ERR | LS_RX_OVERRUN )
};
//------------------------------------------------------------------------------
__inline
void
BootBiosInitializeSerialPort(
    uint16_t IoPortBase
    )
{
    if ( IoPortBase ) 
    {
        _outp(IoPortBase+comLineControl, 0x80);   // Access Baud Divisor
        _outp(IoPortBase+comDivisorLow, 0x03);    // 38400
        _outp(IoPortBase+comDivisorHigh, 0x00);
        _outp(IoPortBase+comFIFOControl, 0x01);   // Enable FIFO if present
        _outp(IoPortBase+comLineControl, 0x03);   // 8 bit, no parity
        _outp(IoPortBase+comIntEnable, 0x00);     // No interrupts, polled
        _outp(IoPortBase+comModemControl, 0x03);  // Assert DTR, RTS
    }
}

//------------------------------------------------------------------------------

__inline
void
BootBiosWriteByteToSerialPort(
    uint16_t IoPortBase, 
    uchar ucChar
    )
{
    if ( IoPortBase ) 
    {
        while ( !(_inp(IoPortBase+comLineStatus) & LS_THR_EMPTY) )
        {
            ;
        }

        _outp(IoPortBase+comTxBuffer, ucChar);
    }
}

//------------------------------------------------------------------------------
__inline
uint8_t
BootBiosReadByteFromSerialPort(
    uint16_t IoPortBase
    )
{
    unsigned char   ucStatus;
    unsigned char   ucChar;

    if ( IoPortBase ) 
    {
        ucStatus = (UCHAR)_inp(IoPortBase+comLineStatus);

        if ( ucStatus & LS_RX_DATA_READY ) 
        {
            ucChar = (UCHAR)_inp(IoPortBase+comRxBuffer);

            if ( ucStatus & LS_RX_ERRORS ) 
            {
                return (uint8_t)-1;
            } 
            else 
            {
                return (ucChar);
            }

        }
    }

    return (uint8_t)-1;
}


#ifdef __cplusplus
}
#endif

#endif __BOOT_SERIAL_H
