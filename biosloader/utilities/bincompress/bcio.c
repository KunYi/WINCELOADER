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
#include <stdio.h>
#include <string.h>

//#include "io.h"

//
// Opens a file for reading and determines the file length
//
// Returns
//      0 on success
//     >0 on failure
//
int OpenForReading(PCHAR fileName, FILE ** ppFile, ULONG * pLength)
{
    fpos_t length;
    
    // Check the caller's arguments
    if (fileName == NULL || ppFile == NULL || pLength == NULL)
    {
        printf("OpenForReading: bad arguments\n");
        return 1;
    }
    
    // Try to open the file
    *ppFile = fopen(fileName, "rb");
    if (*ppFile == NULL)
    {
        printf("OpenForReading: fopen() failed\n");
        return 2;
    }

    // Determine the size of the file
    if (fseek(*ppFile, 0, SEEK_END) != 0)
    {
        printf("OpenForReading: fseek() failed\n");
        fclose(*ppFile);
        return 3;
    }
    
    if (fgetpos(*ppFile, &length) != 0)
    {
        printf("OpenForReading: fgetpos() failed\n");
        fclose(*ppFile);
        return 4;
    }
    
    if (length > ((ULONG)-1))
    {
        printf("Files longer than 4GB are not supported\n");
        fclose(*ppFile);
        return 5;
    }

    *pLength = (ULONG)length;
    
    if (fseek(*ppFile, 0, SEEK_SET) != 0)
    {
        printf("OpenForReading: fseek() failed\n");
        fclose(*ppFile);
        return 3;
    }

    return 0;
}

//
// Opens a file for writing
//
// Returns
//      0 on success
//     >0 on failure
//
int OpenForWriting(PCHAR fileName, FILE ** ppFile)
{
    // Check the caller's arguments
    if (fileName == NULL || ppFile == NULL)
    {
        printf("OpenForWriting: bad arguments\n");
        return 1;
    }
    
    // Try to open the file
    *ppFile = fopen(fileName, "wb");
    if (*ppFile == NULL)
    {
        printf("OpenForWriting: fopen() failed\n");
        return 2;
    }

    return 0;
}


