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

#include "bcio.h"
#include "bcxpress.h"
#include "bincompress.h"

// Work buffers
CHAR originalData[BINCOMPRESS_MAX_BLOCK];
CHAR encodedData[BINCOMPRESS_MAX_BLOCK];

// Usage information
PCHAR USAGE =   "Compresses/Decompresses BIN images to be used with BIOS loader\n"
                "\n"
                "Usage: bincompress /C|/D infile outfile\n"
                "\n"
                "/C     compresses <infile> and writes the results to <outfile>\n"
                "/D     decompressed <infile> and writes the results to <outfile>\n"
                "\n";

#define ACTION_COMPRESS     1
#define ACTION_DECOMPRESS   2
                
//
// A dispatcher function
//
int Process(int action, int algorithm, PCHAR in, PCHAR out)
{
    switch (action)
    {
        case ACTION_COMPRESS:
            return CeCompressEncodeFile(in, out);

        case ACTION_DECOMPRESS:
            return CeCompressDecodeFile(in, out);

        default:
            return 10;
    }
}

//
//
//
void CompressionStartMessage()
{
    printf("Compressing     :    0%");
}

//
//
//
void CompressionProgressMessage(double progress)
{
    printf("\b\b\b\b%3d%%", (int)progress);
}

//
//
//
void CompressionEndMessage(int inFileLength, int outFileLength)
{
    printf("\b\b\b\b100%%\n");

    printf("Input file size : %d bytes\n", inFileLength);
    printf("Output file size: %d bytes\n", outFileLength);
    
    // Display some statistics
    if (inFileLength == 0)
    {
        printf("Gain            : 0.00\n");
    }
    else
    {
        printf("Gain            : %.2f%%\n", (1 - outFileLength/(double)inFileLength)*100);
    }

}

//
//
//
void DecompressionStartMessage()
{
    printf("Decompressing   :    0%");
}

//
//
//
void DecompressionProgressMessage(double progress)
{
    printf("\b\b\b\b%3d%%", (int)progress);
}

//
//
//
void DecompressionEndMessage(int inFileLength, int outFileLength)
{
    printf("\b\b\b\b100%%\n");
   
    printf("Input file size : %d bytes\n", inFileLength);
    printf("Output file size: %d bytes\n", outFileLength);
}

//
// bincompress main 
//
int main(int argc, char ** argv)
{
    int algorithm = 0;
    int action = -1;
    int status = 0;
    
    PCHAR in;
    PCHAR out;

    if (argc == 4)
    {
        // We'll use the default compression algorithm
        
        // Action
        if (strcmp(argv[1], "/C") == 0 || strcmp(argv[1], "/c") == 0)
        {
            action = ACTION_COMPRESS;
        }
        else if (strcmp(argv[1], "/D") == 0 || strcmp(argv[1], "/d") == 0)
        {
            action = ACTION_DECOMPRESS;
        }
        else
        {
            puts(USAGE);
            return 0;
        }

        in = argv[2];
        out = argv[3];
    }
    else
    {
        puts(USAGE);
        return 0;
    }

    if ((status = Process(action, algorithm, in, out)) != 0)
    {
        puts("bincompress: command failed");
    }
    
    return status;
}
