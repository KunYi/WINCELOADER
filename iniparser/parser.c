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
//------------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "util.h"
#include "parser.h"

// Boolean parsing
CHAR * BOOL_TRUE_TABLE [] =
{
    "on", "yes", "true", "1"
};
#define BOOL_TRUE_TABLE_SIZE (sizeof(BOOL_TRUE_TABLE)/sizeof(CHAR *))
#define MAX_BOOL_STRING_LEN     5

// End of line characters
#define CR  ((CHAR)0x0D)
#define LF  ((CHAR)0x0A)

PCHAR pIniFile;     // Start address of the INI file raw data
PCHAR pIniFileEnd;  // End address of the INI file raw data

// Helper function prototypes
PARSESTATUS LocateParameterValue(PCHAR pParameter, PCHAR * ppValue);

/**
 * Initializes the INI file parser. This function must be called with non-NULL
 * pIniRawData and non-zero ulLength prior to using any other function of the
 * INI file parser API.
 * 
 * Inputs
 *      pIniRawData  - Pointer to the entire INI file as stored on the device 
 *                     (e.g.including CR/LF characters). The buffer must be 
 *                     a zero-terminated string.
 *                     
 *                     NOTE: the supplied buffer must be valid thoughout the
 *                     whole operation period of the parser (i.e. during each 
 *                     subsequent API function call)
 *                     
 *      ulLength     - length of the INI file in bytes
 *
 * Outputs
 *
 * Return value
 */
void IniParserInit(PCHAR pIniRawData, ULONG ulLength)
{
    // Check arguments
    if (pIniRawData == NULL || ulLength == 0)
    {
        return;
    }
    
    pIniFile = pIniRawData;
    pIniFileEnd = pIniFile + ulLength;
}

/**
 * Internal function: locates the first character of the value of
 * a parameter
 *
 * Inputs
 *      pParameter   - a zero-terminated string containing the parameter's
 *                     name
 *                     
 * Outputs
 *      ppValue      - pointer to the first character of the value of 
 *                     the parameter
 * 
 * Return value
 *      INIPARSE_OK              - parameter value found
 *      INIPARSE_ARG_ERROR       - either of the arguments is NULL
 *      INIPARSE_NOT_INITED      - parser has not been initialized
 *      INIPARSE_PARAM_NOT_FOUND - parameter was not found
 *      INIPARSE_PARSE_ERROR     - parameter found, unable to determine parameter value
 */
PARSESTATUS LocateParameterValue(PCHAR pParameter, PCHAR * ppValue)
{
    PCHAR pIniFilePos = pIniFile;
    PCHAR pTempParameter = pParameter;
    
    // Check arguments
    if (pParameter == NULL || ppValue == NULL)
    {
        return INIPARSE_ARG_ERROR;
    }

    if (pIniFile == NULL)
    {
        return INIPARSE_NOT_INITED;
    }
  
    // Main parser loop
    while (1)
    {
        // Skip CRs & LFs
        while ((*pIniFilePos == CR || *pIniFilePos == LF) && pIniFilePos < pIniFileEnd)
        {
            pIniFilePos++;
        }

        if (pIniFilePos >= pIniFileEnd)
        {
            break;
        }

        // If the line starts with '[' or "#" - skip the whole line
        if (*pIniFilePos == '[' || *pIniFilePos == '#')
        {
            while (*pIniFilePos != CR && *pIniFilePos != LF && pIniFilePos < pIniFileEnd)
            {
                pIniFilePos++;
            }
            continue;
        }

        // Compare against the required parameter
        while (*pIniFilePos == *pTempParameter)
        {
            pIniFilePos++;
            pTempParameter++;
        }

        // Match?
        if (*pTempParameter == '\0')
        {
            if (*pIniFilePos == '=')
            {
                // Match
                *ppValue = pIniFilePos + 1;
                return INIPARSE_OK;
            }
            
            if (*pIniFilePos == '\0')
            {
                // No '='?
                return INIPARSE_PARSE_ERROR;
            }
        }
        pTempParameter = pParameter;
       
        // Scroll to the next INI entry
        while (*pIniFilePos != CR && *pIniFilePos != LF && pIniFilePos < pIniFileEnd)
        {
            pIniFilePos++;
        }
    }
    
    return INIPARSE_PARAM_NOT_FOUND;
}

/**
 * Retrieves string representation of an INI parameter.
 *
 * Inputs
 *      pParameter   - a zero-terminated string containing the parameter's
 *                     name
 *
 *      ulLength     - length of pBuf
 *
 * Outputs
 *      pBuf         - character buffer in which the retrieved string will
 *                     be stored.
 *                     If parameter value is shorter than ulLength, the remainig
 *                     bytes of the buffer will be filled with '\0' characters.
 *                     If parameter value length is ulLength then no '\0' character
 *                     will be written.
 *                     If parameter value is longer than ulLength only the first
 *                     ulLength characters will be stored in the buffer. No '\0'
 *                     character will be written.
 *
 * Return values
 *      INIPARSE_OK              - parameter value found
 *      INIPARSE_ARG_ERROR       - bad arguments
 *      INIPARSE_NOT_INITED      - parser has not been initialized
 *      INIPARSE_PARAM_NOT_FOUND - parameter was not found
 *      INIPARSE_PARSE_ERROR     - parameter found, unable to determine parameter value
 */
PARSESTATUS GetIniString(PCHAR pParameter, PCHAR pBuf, ULONG ulLength)
{
    PARSESTATUS status;
    PCHAR pValue;
   
    // Check arguments
    if (pBuf == NULL || ulLength == 0)
    {
        return INIPARSE_ARG_ERROR;
    }
    
    // Try to find the value
    status = LocateParameterValue(pParameter, &pValue);
    if (status != INIPARSE_OK)
    {
        return status;
    }

    // Copy until we reach CR/LF or buffer end
    while (ulLength-- > 0 && *pValue != CR && *pValue != LF)
    {
        *(pBuf++) = *(pValue++);
    }

    // Fill the rest of the buffer with '\0's
    while (ulLength-- > 0)
    {
        *(pBuf++) = '\0';
    }

    return INIPARSE_OK;
}

/**
 * Returns the length of the string representation of an INI parameter.
 *
 * Inputs
 *      pParameter   - a zero-terminated string containing the parameter's
 *                     name
 * 
 * Outputs
 *
 * Return value
 *      length of the parameter's value. If the requested parameter is
 *      not found 0 will be returned.
 */
ULONG GetIniStringLen(PCHAR pParameter)
{
    ULONG ulLength = 0;
    PCHAR pValue;
   
    // Check arguments
    if (pParameter == NULL)
    {
        return INIPARSE_ARG_ERROR;
    }
    
    // Try to find the value
    if (LocateParameterValue(pParameter, &pValue) != INIPARSE_OK)
    {
        return 0;
    }

    // Get the length
    while (*pValue != CR && *pValue != LF)
    {
        pValue++;
        ulLength++;
    }

    return ulLength;
}

/**
 * Returns integer (BYTE) representaion of an INI parameter.
 *
 * Inputs
 *      pParameter   - a zero-terminated string containing the parameter's
 *                     name
 *
 * Outputs
 *      pBuf         - Pointer to a memory buffer in which the byte value of
 *                     the parameter's value will be stored. In case of an 
 *                     error, the buffer will not be written.
 *
 * Return values
 *      INIPARSE_OK              - parameter value found
 *      INIPARSE_ARG_ERROR       - bad arguments
 *      INIPARSE_NOT_INITED      - parser has not been initialized
 *      INIPARSE_PARAM_NOT_FOUND - parameter was not found
 *      INIPARSE_PARSE_ERROR     - parameter found, unable to determine parameter value
 */
PARSESTATUS GetIniByte(PCHAR pParameter, PBYTE pBuf)
{
    PARSESTATUS status;
    DWORD dwValue;
   
    // Do the parsing
    status = GetIniDword(pParameter, &dwValue);
    if (status == INIPARSE_OK)
    {
        *pBuf = (BYTE)dwValue;
    }

    return status;
}

/**
 * Returns integer (WORD) representaion of an INI parameter.
 *
 * Inputs
 *      pParameter   - a zero-terminated string containing the parameter's
 *                     name
 *
 * Outputs
 *      pBuf         - Pointer to a memory buffer in which the word value of
 *                     the parameter's value will be stored. In case of an 
 *                     error, the buffer will not be written.
 *
 * Return values
 *      INIPARSE_OK              - parameter value found
 *      INIPARSE_ARG_ERROR       - bad arguments
 *      INIPARSE_NOT_INITED      - parser has not been initialized
 *      INIPARSE_PARAM_NOT_FOUND - parameter was not found
 *      INIPARSE_PARSE_ERROR     - parameter found, unable to determine parameter value
 */
PARSESTATUS GetIniWord(PCHAR pParameter, PWORD pBuf)
{
    PARSESTATUS status;
    DWORD dwValue;
   
    // Do the parsing
    status = GetIniDword(pParameter, &dwValue);
    if (status == INIPARSE_OK)
    {
        *pBuf = (WORD)dwValue;
    }

    return status;
}

/**
 * Returns integer (DWORD) representaion of an INI parameter.
 *
 * Inputs
 *      pParameter   - a zero-terminated string containing the parameter's
 *                     name
 *
 * Outputs
 *      pBuf         - Pointer to a memory buffer in which the dword value of
 *                     the parameter's value will be stored. In case of an 
 *                     error, the buffer will not be written.
 *
 * Return values
 *      INIPARSE_OK              - parameter value found
 *      INIPARSE_ARG_ERROR       - bad arguments
 *      INIPARSE_NOT_INITED      - parser has not been initialized
 *      INIPARSE_PARAM_NOT_FOUND - parameter was not found
 *      INIPARSE_PARSE_ERROR     - parameter found, unable to determine parameter value
 */
PARSESTATUS GetIniDword(PCHAR pParameter, PDWORD pBuf)
{
    PARSESTATUS status;
    PCHAR pValue;
    DWORD dwValue;
    
    // Check arguments
    if (pParameter == NULL || pBuf == NULL)
    {
        return INIPARSE_ARG_ERROR;
    }
    
    // Try to find the value
    status = LocateParameterValue(pParameter, &pValue);
    if (status != INIPARSE_OK)
    {
        return status;
    }

    // Parse
    if (ParseInteger(pValue, &dwValue) == 0)
    {
        return INIPARSE_PARSE_ERROR;
    }
    else
    {
        *pBuf = dwValue;
        return INIPARSE_OK;
    }
}

/**
 * Returns Boolean representation of an INI parameter.
 *
 * Inputs
 *      pParameter   - a zero-terminated string containing the parameter's
 *                     name
 *
 * Outputs
 *      pBuf         - Pointer to a memory buffer in which the Boolean value of
 *                     the parameter's value will be stored. In case of an 
 *                     error, the buffer will not be written.
 *
 * Return values
 *      INIPARSE_OK              - parameter value found
 *      INIPARSE_ARG_ERROR       - bad arguments
 *      INIPARSE_NOT_INITED      - parser has not been initialized
 *      INIPARSE_PARAM_NOT_FOUND - parameter was not found
 *      INIPARSE_PARSE_ERROR     - parameter found, unable to determine parameter value
 */
PARSESTATUS GetIniBool(PCHAR pParameter, PBOOL pBuf)
{
    ULONG i;
    PARSESTATUS status;
    CHAR value[MAX_BOOL_STRING_LEN + 1];
    
    // Check arguments
    if (pParameter == NULL || pBuf == NULL)
    {
        return INIPARSE_ARG_ERROR;
    }
    
    // Try to find the value
    status = GetIniString(pParameter, value, MAX_BOOL_STRING_LEN + 1);
    if (status != INIPARSE_OK)
    {
        return status;
    }

    // Parse
    for (i = 0; i < BOOL_TRUE_TABLE_SIZE; i++)
    {
        if (strcmp(value, BOOL_TRUE_TABLE[i]) == 0)
        {
            *pBuf = TRUE;
            return INIPARSE_OK;
        }
    }

    *pBuf = FALSE;
    return INIPARSE_OK;
}

