//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//------------------------------------------------------------------------------

//
// BIOS loader debug support
//

#ifndef _BOOTDEBUG_H_
#define _BOOTDEBUG_H_

// Which function to use to output debug information
#define SERPRINT      Print

// Debug mask global variable
extern DWORD dwZoneMask;

// Helps define debug masks
#define DBGZONE(n)    (dwZoneMask&(0x00000001<<(n)))

// Default debug zones
#define DBGZONE_ERROR      DBGZONE(0)
#define DBGZONE_WARNING    DBGZONE(1)
#define DBGZONE_INFO       DBGZONE(2)

// Control how much info to print out
// By default, turn off for faster boot
#define PRINT_INFO 0

//#ifndef DEBUG
//#define DEBUG
//#endif
//#ifndef FULLMESSAGES
//#define FULLMESSAGES
//#endif

// In case someone combined SHIP and DEBUG options
#ifdef DEBUG
#ifdef SHIP_BUILD
#undef SHIP_BUILD
#pragma message (__FILE__ ":WARNING: SHIP_BUILD turned off since DEBUG defined")
#endif
#endif

// Define SHIP_BUILD to turn off output at all (except for RTLMSG)
#ifdef SHIP_BUILD

#define RTLMSG(cond,printf_exp)   \
   ((void)((cond)?(SERPRINT printf_exp),1:0))

#define DBGMSG(cond,printf_exp) ((void)0)

#define INFOMSG(code, printf_exp) ((void)0)
#define WARNMSG(code, printf_exp) ((void)0)
#define ERRMSG(code, printf_exp) ((void)0)
#endif

// Define DEBUG to turn on the debug output
#ifdef DEBUG

#ifndef FULLMESSAGES
#define FULLMESSAGES
#endif

#define DBGMSG(cond,printf_exp)   \
   ((void)((cond)?(SERPRINT printf_exp),1:0))
#define RTLMSG(cond,printf_exp)   \
   ((void)((cond)?(SERPRINT printf_exp),1:0))

//
// Define FULLMESSAGES to include full messages
// in the binary file.
//
// If FULLMESSAGES is not defined only message
// codes are included in the binary file.
//
#ifdef FULLMESSAGES

#define INFOMSG(code, printf_exp) \
   ((void)((DBGZONE_INFO)?(SERPRINT(L"INFO: "), SERPRINT printf_exp),1:0))
#define WARNMSG(code, printf_exp) \
   ((void)((DBGZONE_WARNING)?(SERPRINT(L"WARNING: "), SERPRINT printf_exp),1:0))
#define ERRMSG(code, printf_exp) \
   ((void)((DBGZONE_ERROR)?(SERPRINT(L"ERROR: "), SERPRINT printf_exp),1:0))

#else   // FULLMESSAGES

#define INFOMSG(code, printf_exp) \
   ((void)((DBGZONE_INFO)?(SERPRINT(L"INFO: code=0x%x.\n", code)),1:0))
#define WARNMSG(code, printf_exp) \
   ((void)((DBGZONE_WARNING)?(SERPRINT(L"WARNING: code=0x%x.\n", code)),1:0))
#define ERRMSG(code, printf_exp) \
   ((void)((DBGZONE_ERROR)?(SERPRINT(L"ERROR: code=0x%x.\n", code)),1:0))

#endif // FULLMESSAGES
#endif // DEBUG


#ifndef DEBUG
#define DBGMSG(cond,printf_exp) ((void)0)

#define RTLMSG(cond,printf_exp)   \
   ((void)((cond)?(SERPRINT printf_exp),1:0))
#define INFOMSG(cond,printf_exp)   \
   ((void)((cond)?(SERPRINT printf_exp),1:0))
#define WARNMSG(cond,printf_exp)   \
   ((void)((cond)?(SERPRINT printf_exp),1:0))
#define ERRMSG(cond,printf_exp)   \
   ((void)((cond)?(SERPRINT printf_exp),1:0))
#endif


#endif // _BOOTDEBUG_H_
