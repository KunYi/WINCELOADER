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

//
// On the top of FAT functionality provides support for reading
// both regular and compressed files
//

#include <windows.h>

#ifdef CECOMPRESSION

#include <halether.h>
#include "bldr.h"
#include "decoder.h"
#include "debug.h"

// signature
CHAR BINCOMPRESS_SIGNATURE [] = BINCOMPRESS_SIGNATURE_VALUE;

// Work buffers
CHAR * originalData = BINCOMPRESS_ORIGINAL_DATA_BUFFER;
CHAR * encodedData = BINCOMPRESS_ENCODED_DATA_BUFFER;

// Shows how much data can be read from originalData
// without decompressin the next chunk
ULONG originalDataBytesLeft;
ULONG originalDataPos;

// Length of the file data (after decompression for compressed files)
BLOCKLENGTH originalFileLength;

// TRUE when dealing with a compressed file
BOOL compressedFile;


// decoder handle
CeCompressDecodeStream decodeStream;

/**
 * Mem alloc callback for cecompress
 */
void * CompressAlloc(void * Context, DWORD AllocSize)
{
    // We don't have any memory management, we just return a memory addres
    // See bldr.h
    return BINCOMPRESS_WORK_AREA_BUFFER;
}

/**
 * Initializes the decoder and underlying file system
 */
BOOL CeCompressInit(void)
{
    if( encodedData != BINCOMPRESS_ENCODED_DATA_BUFFER )
    {
        //
        // This means that the CopyDataSections() function failed.  This was
        // caused because memcpy() was failing, meaning there is probably a
        // linking problem.
        //
        return FALSE;
    }

    // Initialize the underlying FAT
    if (!FSInit())
    {
        return FALSE;
    }

    // Initialize the decoder
    decodeStream = CeCompressDecodeCreate(NULL, CompressAlloc);
    if (decodeStream == NULL)
    {
        ERRMSG(MSG_DECOMPRESS_INIT_ERROR, ("CeCompress init failed\n"));
        return FALSE;
    }

    return TRUE;
}

/**
 * Opens a compressed/uncompressed file for reading
 */
ULONG CeCompressOpenFile(PCHAR pFileName)
{
    // Open the file using FAT layer
    originalFileLength = FSOpenFile(pFileName);
    if (originalFileLength == 0)
    {
        return 0;
    }

    // Check whether the file is compressed or not
    // Compressed files are always larger than
    // BINCOMPRESS_SIGNATURE_SIZE + sizeof(BLOCKLENGTH)
    compressedFile = FALSE;
    if (originalFileLength >= BINCOMPRESS_SIGNATURE_SIZE + sizeof(BLOCKLENGTH))
    {
        // Read the first BINCOMPRESS_SIGNATURE_SIZE + sizeof(BLOCKLENGTH)
        // bytes to find out
        if (!FSReadFile(encodedData, BINCOMPRESS_SIGNATURE_SIZE + sizeof(BLOCKLENGTH)))
        {
            return 0;
        }

        // Compare the signature
        if (memcmp(BINCOMPRESS_SIGNATURE, encodedData, BINCOMPRESS_SIGNATURE_SIZE) == 0)
        {
            // Compressed file
            // Read original file length from the header
            originalFileLength = *((BLOCKLENGTH *)(encodedData + BINCOMPRESS_SIGNATURE_SIZE));
            compressedFile = TRUE;
        }
        else
        {
            // For a regular file leave the originalFileLength untouched
            // but we need to "rewind" the bytes just read
            if (!FSRewind(BINCOMPRESS_SIGNATURE_SIZE + sizeof(BLOCKLENGTH)))
            {
                return 0;
            }
        }
    }

    originalDataBytesLeft = 0;

    return originalFileLength;
}

/**
 * Closes a compressed/uncompressed file
 */
void CeCompressCloseFile(void)
{
    originalFileLength = 0;
    FSCloseFile();
}


/**
 * Reads a compressed/uncompressed file.
 *
 * To make the code shorter we assume that the caller does not
 * attempt to read more than the file length returned by
 * CeCompressOpenFile()
 */
BOOL CeCompressReadFile(PUCHAR pAddress, ULONG Length)
{
    // File open?
    if (originalFileLength == 0)
    {
        return FALSE;
    }

    if (compressedFile)
    {
        // Decompress as many CECOMPRESS_MAX_BLOCK chunks as
        // needed to satisfy the request.
        while (Length > 0)
        {
            BLOCKLENGTH blockLength;
            BLOCKLENGTH decompressedLength;
            BOOL storedBlock;
            BOOL partialBlock;

            // Can we satisfy at least a part of the request
            // with what we have in the buffer?
            if (originalDataBytesLeft > 0)
            {
                if (Length <= originalDataBytesLeft)
                {
                    // We can satisfy the whole remaining request
                    memcpy(pAddress, originalData + originalDataPos, Length);

                    originalDataPos       += Length;
                    originalDataBytesLeft -= Length;
                    return TRUE;
                }
                else
                {
                    // We can satisfy a part of the request
                    memcpy(pAddress, originalData + originalDataPos, originalDataBytesLeft);

                    pAddress              += originalDataBytesLeft;
                    Length                -= originalDataBytesLeft;
                    originalDataPos       += originalDataBytesLeft;
                    originalDataBytesLeft  = 0;
                }
            }

            // Read and decompress a new chunk
            // First read the blockLength
            if (!FSReadFile((PUCHAR)&blockLength, sizeof(BLOCKLENGTH)))
            {
                return FALSE;
            }
            storedBlock = blockLength & NO_COMPRESSION_BIT;
            partialBlock = blockLength & PARTIAL_BLOCK_BIT;
            blockLength &= ~NO_COMPRESSION_BIT;
            blockLength &= ~PARTIAL_BLOCK_BIT;

            // Read decompressed block length if it's a partial block
            if (partialBlock)
            {
                if (!FSReadFile((PUCHAR)&decompressedLength, sizeof(decompressedLength)))
                {
                    return FALSE;
                }
            }
            else
            {
                decompressedLength = CECOMPRESS_MAX_BLOCK;
            }

            // Compressed or just stored?
            if (storedBlock)
            {
                // Read the stored block
                if (!FSReadFile(originalData, blockLength))
                {
                    return FALSE;
                }
                originalDataBytesLeft = decompressedLength;
            }
            else
            {
                // Read the stored block
                if (!FSReadFile(encodedData, blockLength))
                {
                    return FALSE;
                }

                // Decode the data
                originalDataBytesLeft = CeCompressDecode(
                    decodeStream,
                    originalData, CECOMPRESS_MAX_BLOCK,
                    decompressedLength,
                    encodedData, blockLength);

                if (originalDataBytesLeft == -1)
                {
                    ERRMSG(MSG_DECOMPRESSION_ERROR, ("CeCompress decompression failed\n"));
                    return FALSE;
                }
            }

            originalDataPos = 0;
        }

        return TRUE;
    }
    else
    {
        // As easy as that!
        return FSReadFile(pAddress, Length);
    }
}

#endif
