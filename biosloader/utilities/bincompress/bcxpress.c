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
#include <cecompress.h>

#include "bincompress.h"
#include "bcio.h"
#include "bcxpress.h"

/**
 * Mem alloc callback for cecompress
 */
void * CompressAlloc(void * Context, DWORD AllocSize)
{
    return malloc(AllocSize); 
}

/**
 * Mem free callback for cecompress
 */
void CompressFree(void * Context, void * Address)
{
    free(Address);
}

/**
 * Encodes data contained in the input file, writes the result to the output file
 */
int CeCompressEncodeFile(PCHAR inFileName, PCHAR outFileName)
{
    FILE * inFile;
    BLOCKLENGTH inFileLength;
    
    FILE * outFile;
    ULONG outFileLength;

    // A counter for processed bytes
    ULONG processedLength;  
    BLOCKLENGTH readLength;
    ULONG encodedLength;
    BLOCKLENGTH blockLength;

    CeCompressEncodeStream encodeStream;
   
    // Check the arguments
    if (inFileName == NULL || outFileName == NULL)
    {
        printf("\nCeCompressEncodeFile: bad arguments\n");
        return 1;
    }
    
    // Initialze the encoder
    encodeStream = CeCompressEncodeCreate(CECOMPRESS_MAX_BLOCK, NULL, CompressAlloc);
    if (encodeStream == NULL)
    {
        printf("\nCeCompressEncodeFile: CeCompressEncodeCreate() failed\n");
        return 20;
    }

    // Open the input file
    if (OpenForReading(inFileName, &inFile, &inFileLength) != 0)
    {
        printf("\nCeCompressEncodeFile: OpenForReading() failed\n");
        CeCompressEncodeClose(encodeStream, NULL, CompressFree);    
        return 1;
    }

    // Open the output file
    if (OpenForWriting(outFileName, &outFile))
    {
        printf("\nCeCompressEncodeFile: OpenForWriting() failed\n");
        CeCompressEncodeClose(encodeStream, NULL, CompressFree);    
        fclose(inFile);
        return 2;
    }
    outFileLength = 0;

    // Write signature
    if (fwrite(BINCOMPRESS_SIGNATURE_VALUE, 1, BINCOMPRESS_SIGNATURE_SIZE, outFile) != BINCOMPRESS_SIGNATURE_SIZE)
    {
        printf("\nCeCompressEncodeFile: fwrite() failed\n");
        CeCompressEncodeClose(encodeStream, NULL, CompressFree);    
        fclose(inFile);
        fclose(outFile);
        return 4;
    }
    outFileLength += BINCOMPRESS_SIGNATURE_SIZE;
    
    // Write decompressed size
    if (fwrite(&inFileLength, sizeof(BLOCKLENGTH), 1, outFile) != 1)
    {
        printf("\nCeCompressEncodeFile: fwrite() failed\n");
        CeCompressEncodeClose(encodeStream, NULL, CompressFree);    
        fclose(inFile);
        fclose(outFile);
        return 4;
    }
    outFileLength += sizeof(BLOCKLENGTH);
    
    // Encode CECOMPRESS_MAX_BLOCK-sized blocks
    processedLength = 0;
    CompressionStartMessage();
    
    while (processedLength < inFileLength)
    {
        // Read CECOMPRESS_MAX_BLOCK of input data
        readLength = fread(originalData, 1, CECOMPRESS_MAX_BLOCK, inFile);
        if (ferror(inFile) != 0)
        {
            printf("\nCeCompressEncodeFile: fread() failed\n");
            CeCompressEncodeClose(encodeStream, NULL, CompressFree);
            fclose(inFile);
            fclose(outFile);
            return 3;
        }
        processedLength += readLength;

        // Encode 
        encodedLength = CeCompressEncode(
            encodeStream, 
            encodedData, CECOMPRESS_MAX_BLOCK, 
            originalData, readLength);

        if (encodedLength == 0)
        {
            printf("\nCeCompressEncodeFile: CeCompressEncode() failed\n");
            CeCompressEncodeClose(encodeStream, NULL, CompressFree);    
            fclose(inFile);
            fclose(outFile);
            return 7;
        }
        
        // If encodedLength happens to be == readLength we'll be writing the original data
        if (encodedLength >= readLength)
        {
            blockLength = readLength | NO_COMPRESSION_BIT;
        }
        else
        {
            blockLength = encodedLength;
        }

        // Mark the last block with PARTIAL_BLOCK_BIT
        if (processedLength == inFileLength)
        {
            blockLength |= PARTIAL_BLOCK_BIT;
        }
        
        // Write block length, TODO: different endians?
        if (fwrite(&blockLength, sizeof(blockLength), 1, outFile) != 1)
        {
            printf("\nCeCompressEncodeFile: fwrite() failed\n");
            CeCompressEncodeClose(encodeStream, NULL, CompressFree);
            fclose(inFile);
            fclose(outFile);
            return 4;
        }
        blockLength &= ~NO_COMPRESSION_BIT;
        blockLength &= ~PARTIAL_BLOCK_BIT;
        outFileLength += sizeof(blockLength);

        // Write uncompressed block length with the last block 
        if (processedLength == inFileLength)
        {
            if (fwrite(&readLength, sizeof(readLength), 1, outFile) != 1)
            {
                printf("\nCeCompressEncodeFile: fwrite() failed\n");
                CeCompressEncodeClose(encodeStream, NULL, CompressFree);
                fclose(inFile);
                fclose(outFile);
                return 4;
            }
            outFileLength += sizeof(readLength);
        }
       
        // Write data
        if (encodedLength >= readLength)
        {
            // Write original data
            if (fwrite(originalData, 1, readLength, outFile) != readLength)
            {
                CeCompressEncodeClose(encodeStream, NULL, CompressFree);
                fclose(inFile);
                fclose(outFile);
                return 4;
            }
        }
        else
        {
            // Write compressed data
            if (fwrite(encodedData, 1, encodedLength, outFile) != encodedLength)
            {
                CeCompressEncodeClose(encodeStream, NULL, CompressFree);
                fclose(inFile);
                fclose(outFile);
                return 4;
            }
        }
        outFileLength += blockLength;

        // Update progress display
        CompressionProgressMessage((int)(processedLength/(inFileLength/100.0)));
    }
    
    // Clean things up
    CeCompressEncodeClose(encodeStream, NULL, CompressFree);
    fclose(inFile);
    fclose(outFile);

    CompressionEndMessage(inFileLength, outFileLength);
    
    return 0;
}

/**
 * Decodes data contained in the input file and saves the results to the output file
 */
int CeCompressDecodeFile(PCHAR inFileName, PCHAR outFileName)
{
    FILE * inFile;
    ULONG inFileLength;
    
    FILE * outFile;
    ULONG outFileLength;
    BLOCKLENGTH expectedOutFileLength;

    CeCompressDecodeStream decodeStream;

    // Counters
    ULONG processedLength;
    ULONG writeLength;
    BLOCKLENGTH decompressedLength;
    BOOL storedBlock;
    BOOL partialBlock;
    BLOCKLENGTH blockLength;

    // Check the arguments
    if (inFileName == NULL || outFileName == NULL)
    {
        printf("\nCeCompressDecodeFile: bad arguments\n");
        return 1;
    }
    
    // Initialize the decoder
    decodeStream = CeCompressDecodeCreate(NULL, CompressAlloc); 
    if (decodeStream == NULL)
    {
        printf("\nCeCompressDecodeFile: CeCompressDecodeCreate() failed\n");
        return 20;
    }
    
    // Open the input file
    if (OpenForReading(inFileName, &inFile, &inFileLength) != 0)
    {
        printf("\nCeCompressDecodeFile: OpenForReading() failed\n");
        CeCompressDecodeClose(decodeStream, NULL, CompressFree);    
        return 1;
    }
    processedLength = 0;

    // Read signature
    if (fread(encodedData, BINCOMPRESS_SIGNATURE_SIZE, 1, inFile) != 1)
    {
        printf("\nCeCompressDecodeFile: fread() failed\n");
        CeCompressDecodeClose(decodeStream, NULL, CompressFree);    
        fclose(inFile);
        return 3;
    }
    processedLength += BINCOMPRESS_SIGNATURE_SIZE;

    // Check signature value
    if (memcmp(encodedData, BINCOMPRESS_SIGNATURE_VALUE, BINCOMPRESS_SIGNATURE_SIZE) != 0)
    {
        printf("\nCeCompressDecodeFile: bad file signature\n");
        CeCompressDecodeClose(decodeStream, NULL, CompressFree);
        fclose(inFile);
        return 9;
    }

    // Read decompressed file length
    if (fread(&expectedOutFileLength, sizeof(BLOCKLENGTH), 1, inFile) != 1)
    {
        printf("\nCeCompressDecodeFile: fread() failed\n");
        CeCompressDecodeClose(decodeStream, NULL, CompressFree);
        fclose(inFile);
        return 3;
    }
    processedLength += sizeof(BLOCKLENGTH);

    // Open the output file
    if (OpenForWriting(outFileName, &outFile))
    {
        printf("\nCeCompressDecodeFile: OpenForWriting() failed\n");
        CeCompressDecodeClose(decodeStream, NULL, CompressFree);
        fclose(inFile);
        return 2;
    }
    outFileLength = 0;

    // Decode input blocks into CECOMPRESS_MAX_BLOCK-sized blocks
    DecompressionStartMessage();
    while (processedLength < inFileLength)
    {
        // Read block length
        if (fread(&blockLength, sizeof(BLOCKLENGTH), 1, inFile) != 1)
        {
            printf("\nCeCompressDecodeFile: fread() failed\n");
            CeCompressDecodeClose(decodeStream, NULL, CompressFree);
            fclose(inFile);
            fclose(outFile);
            return 3;
        }
        processedLength += sizeof(BLOCKLENGTH);
        storedBlock = blockLength & NO_COMPRESSION_BIT;
        partialBlock = blockLength & PARTIAL_BLOCK_BIT;
        blockLength &= ~NO_COMPRESSION_BIT;
        blockLength &= ~PARTIAL_BLOCK_BIT;

        // Read decompressed block length if it's a partial block
        if (partialBlock)
        {
            if (fread(&decompressedLength, sizeof(decompressedLength), 1, inFile) != 1)
            {
                printf("\nCeCompressDecodeFile: fread() failed\n");
                CeCompressDecodeClose(decodeStream, NULL, CompressFree);
                fclose(inFile);
                fclose(outFile);
                return 3;
            }
            processedLength += sizeof(decompressedLength);
        }
        else
        {
            decompressedLength = CECOMPRESS_MAX_BLOCK;
        }

        // Read compressed data
        if (fread(encodedData, 1, blockLength, inFile) != blockLength)
        {
            printf("\nCeCompressDecodeFile: fread() failed\n");
            CeCompressDecodeClose(decodeStream, NULL, CompressFree);
            fclose(inFile);
            fclose(outFile);
            return 3;
        }
        processedLength += blockLength;

        // Compressed or just stored?
        if (storedBlock)
        {
            // Uncompressed block - just copy
            memcpy(originalData, encodedData, blockLength);
            writeLength = decompressedLength;
        }
        else
        {
            // Decode the data
            writeLength = CeCompressDecode(
                decodeStream,
                originalData, CECOMPRESS_MAX_BLOCK,
                decompressedLength,
                encodedData, blockLength);
        
            if (writeLength == -1)
            {
                printf("\nCeCompressDecodeFile: CeCompressDecode() failed\n");
                CeCompressDecodeClose(decodeStream, NULL, CompressFree);
                fclose(inFile);
                fclose(outFile);
                return 4;
            }
        }

        // Write to the output file
        if (fwrite(originalData, 1, writeLength, outFile) != writeLength)
        {
            printf("\nCeCompressDecodeFile: fwrite() failed\n");
            CeCompressDecodeClose(decodeStream, NULL, CompressFree);  
            fclose(inFile);
            fclose(outFile);
            return 5;
        }
        outFileLength += writeLength;
        
        // Update progress display
        DecompressionProgressMessage(processedLength/(inFileLength/100.0));
    }
   
    // Clean things up
    CeCompressDecodeClose(decodeStream, NULL, CompressFree);
    fclose(inFile);
    fclose(outFile);
    
    // End message
    DecompressionEndMessage(inFileLength, outFileLength);
    
    if (expectedOutFileLength != outFileLength)
    {
        printf("\nCeCompressDecodeFile: expectedOutFileLength != outFileLength (%d != %d)\n", 
                expectedOutFileLength, outFileLength);
        return 6;
    }
    
    return 0;
}
