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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//------------------------------------------------------------------------------

#include <windows.h>

/**
 * Copies at most n characters from the source string to the 
 * destination string.
 *
 * Inputs
 *      dest   - destination string buffer
 *      src    - source string buffer
 *      length - the number of characters to be copied
 *
 * Outputs
 *
 * Return value
 */
void StrNCpy(PUCHAR dest, PUCHAR src, ULONG length)
{
    if (dest == NULL || src == NULL || length == 0)
    {
        return;
    }

    while (length-- > 0 && *src != '\0')
    {
        *dest = *src;
        dest++;
        src++;
    }

    while (length-- > 0)
    {
        *dest = '\0';
    }
}

/**
 * Parses an unsigned decimal integer number
 *
 * Inputs
 *      str  - string representation of the intenger number
 *             to be parsed
 *
 * Outputs
 *      pVal - pointer to a memory bufer in which the integer
 *             value will be stored. In case of parse error
 *             (return value = 0) the buffer will not be written.
 *
 * Return value
 *      The number of the character at which parsing stopped
 *      (i.e. the index of the first non-digit character).
 *      0 indicates parse error.
 */
ULONG ParseDec(PCHAR str, ULONG * pVal)
{
    ULONG charsProcessed = 0;
    
    // Check inputs
    if (str == NULL || pVal == NULL)
    {
        return 0;
    }

    *pVal = 0;
    while (*str >= '0' && *str <= '9')
    {
        *pVal = (*pVal * 10) + (*str - '0');
        str++;
        charsProcessed++;
    }

    return charsProcessed;
}

/**
 * Parses an unsigned hexadecimal integer number
 *
 * Inputs
 *      str  - string representation of the intenger number
 *             to be parsed
 *
 * Outputs
 *      pVal - pointer to a memory bufer in which the integer
 *             value will be stored. In case of parse error
 *             (return value = 0) the buffer will not be written.
 *
 * Return value
 *      The number of the character at which parsing stopped
 *      (i.e. the index of the first non-digit character).
 *      0 indicates parse error.
 */
ULONG ParseHex(PCHAR str, ULONG * pVal)
{
    ULONG charsProcessed = 0;
    
    // Check inputs
    if (str == NULL || pVal == NULL)
    {
        return 0;
    }

    *pVal = 0;
    while (1)
    {
        if (*str >= '0' && *str <= '9')
        {
            *pVal = (*pVal * 16) + (*str - '0');
        }
        else
        if (*str >= 'A' && *str <= 'F')
        {
            *pVal = (*pVal * 16) + (*str - 'A' + 10);
        }
        else
        if (*str >= 'a' && *str <= 'f')
        {
            *pVal = (*pVal * 16) + (*str - 'a' + 10);
        }
        else
        {
            break;
        }
        
        str++;
        charsProcessed++;
    }

    return charsProcessed;
}

/**
 * Parses an integer number (hexadecimal or decimal)
 *
 * Inputs
 *      str  - string representation of the intenger number
 *             to be parsed
 *
 * Outputs
 *      pVal - pointer to a memory bufer in which the integer
 *             value will be stored. In case of parse error
 *             (return value = 0) the buffer will not be written.
 *
 * Return value
 *      The number of the character at which parsing stopped
 *      (i.e. the index of the first non-digit character).
 *      0 indicates parse error.
 */
ULONG ParseInteger(PCHAR str, ULONG * pVal)
{
    // Check inputs
    if (str == NULL || pVal == NULL)
    {
        return 0;
    }

    // Check for "0x"
    if (str[0] == '0' && str[1] == 'x')
    {
        return ParseHex(str+2, pVal);
    }
    else
    {
        return ParseDec(str, pVal);
    }
}
