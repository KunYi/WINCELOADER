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
#include <bootarg.h>
#include "bldr.h"
#include "debug.h"
#include "ini.h"
#include "fs.h"
#include "video.h"
#include "decoder.h"

#include "splash.h"

// Video memory
PBYTE FlatFrameBuffer;

// Need this to draw progress bar
WORD ProgressBarXOffset, ProgressBarYOffset;
BYTE ProgressBarColourIndex;
BYTE BlackColourIndex;
BOOL ProgressBarFrameDrawn;

// BIOS functions interface. See bios.asm for implementation.
ULONG BIOS_SetDACPaletteFormat(ULONG requestedBitsOfColour, ULONG * currentBitsOfColour);
ULONG BIOS_SetDACPaletteData(ULONG start, ULONG count, ULONG * paletteData);
void BIOS_SetCursorPosition(ULONG page, ULONG row, ULONG column);
void BIOS_WriteCharacterAtCursor(ULONG page, ULONG color, CHAR character);

// Private function prototypes
void ShowSplashImage();
BYTE FindColour(DWORD colour, DWORD * colourTable, ULONG length);
void DrawVLine(int x, int yStart, int yStop, BYTE colour);
void DrawHLine(int y, int xStart, int xStop, BYTE colour);
void DrawBox(int x, int y, int w, int h, BYTE colour);

/**
 * Setup splash screen (video mode, frame buffer)
 */
BLSTATUS InitSplashScreen()
{
    BOOT_ARGS TempBootArgs;
    ULONG bitsOfColor = 0;

    TempBootArgs.cxDisplayScreen     = SPLASH_SCREEN_WIDTH;
    TempBootArgs.cyDisplayScreen     = SPLASH_SCREEN_HEIGHT;
    TempBootArgs.cxPhysicalScreen    = SPLASH_SCREEN_WIDTH;
    TempBootArgs.cyPhysicalScreen    = SPLASH_SCREEN_HEIGHT;
    TempBootArgs.bppScreen           = 8;

    FlatFrameBuffer = NULL;

    // Find best mode
    if (FindVESAMode(&TempBootArgs) != BLSTATUS_OK)
    {
        WARNMSG(MSG_CANNOT_FIND_SPLASH_MODE, ("Cannot find splash mode\n"));
        return BLSTATUS_ERROR;
    }

    // Set video mode
    if (SetVESAMode(TempBootArgs.vesaMode) != BLSTATUS_OK)
    {
        WARNMSG(MSG_CANNOT_SET_SPLASH_MODE, ("Cannot set splash mode\n"));
        return BLSTATUS_ERROR;
    }

    FlatFrameBuffer = (PBYTE)TempBootArgs.pvFlatFrameBuffer;

    // Draw splash image and progress bar frame
    ShowSplashImage();

    return BLSTATUS_OK;
}

/**
 * Draw splash bitmap (if available) and progress bar frame
 */
void ShowSplashImage()
{
    ULONG SplashFileLen;

    // BMP file data
    BMP_HEADER * BmpHeader = (BMP_HEADER *)BMP_RAW_DATA_BUFFER;
    DWORD * Colours = (DWORD *)(BMP_RAW_DATA_START + sizeof(BMP_HEADER));
    PBYTE Img;

    WORD WidthMod;
    WORD y, xOffset = 0, yOffset = 0;

    // Try to load splash BMP
#ifndef NO_COMPRESSION
    if ((SplashFileLen = BINCOMPRESS_OPEN_FILE(COMPRESSED_SPLASH_IMAGE_FILE_NAME)) == 0)
    {
#endif
        SplashFileLen = BINCOMPRESS_OPEN_FILE(SPLASH_IMAGE_FILE_NAME);
#ifndef NO_COMPRESSION
    }
#endif

    if (SplashFileLen != 0)
    {
        if (SplashFileLen > BMP_RAW_DATA_LENGTH)
        {
            WARNMSG(MSG_TOO_LONG_BMP_FILE, ("BMP longer than %d bytes\n", BMP_RAW_DATA_LENGTH));
            return;
        }

        // Load splash BMP
        if (!BINCOMPRESS_READ_FILE(BMP_RAW_DATA_BUFFER, SplashFileLen))
        {
            WARNMSG(MSG_CANNOT_LOAD_SPLASH_IMAGE, ("Cannot load splash image\n"));
            BINCOMPRESS_CLOSE_FILE();
            return;
        }
        BINCOMPRESS_CLOSE_FILE();

        // Some BMPs which use 256 colours have 0 in ColorsUsed field.
        // Correct this here
        if (BmpHeader->ColorsUsed == 0)
        {
            BmpHeader->ColorsUsed = 256;
        }

        // Convert colour table from 8bit per primary to 6bit per primary
        for (y = 0; y < BmpHeader->ColorsUsed * 4; y++)
        {
            ((BYTE*)Colours)[y] = ((BYTE *)Colours)[y] >> 2;
        }

        Img = (PBYTE)(Colours + BmpHeader->ColorsUsed);
    }
    else
    {
        WARNMSG(MSG_CANNOT_FIND_SPLASH_IMAGE, ("Cannot find splash image\n"));

        // Set some custom palette
        Colours[0] = 0x00000000;
        Colours[1] = PROGRESS_BAR_COLOUR;
        BmpHeader->ColorsUsed = 2;
    }

    // Set DAC palette data
    if (BIOS_SetDACPaletteData(0, BmpHeader->ColorsUsed, Colours) != 0x004F)
    {
        WARNMSG(MSG_CANNOT_SET_PALETTE, ("Cannot set palette\n"));
    }

    // Find colours we need
    ProgressBarColourIndex = FindColour(PROGRESS_BAR_COLOUR, Colours, BmpHeader->ColorsUsed);
    BlackColourIndex = FindColour(0x00000000, Colours, BmpHeader->ColorsUsed);

    // Clear the screen to black
    memset(FlatFrameBuffer, BlackColourIndex, SPLASH_SCREEN_WIDTH * SPLASH_SCREEN_HEIGHT);

    // Draw the bitmap
    if (SplashFileLen != 0)
    {
        // Blit the image on the screen
        if (BmpHeader->Width % 4 == 0)
        {
            WidthMod = (WORD)BmpHeader->Width;
        }
        else
        {
            WidthMod = (WORD)(BmpHeader->Width + (4 - (BmpHeader->Width % 4)));
        }

        xOffset = (WORD)((SPLASH_SCREEN_WIDTH - BmpHeader->Width) / 2);
        yOffset = (WORD)((SPLASH_SCREEN_HEIGHT - (BmpHeader->Height + PROGRESS_BAR_HEIGHT + PROGRESS_BAR_TOP_MARGIN)) / 2);

        for (y = 0; y < BmpHeader->Height; y++)
        {
            memcpy(FlatFrameBuffer + SPLASH_SCREEN_WIDTH*(yOffset+BmpHeader->Height-y) + xOffset,
                   Img + WidthMod*y,
                   BmpHeader->Width);
        }
    }
    else
    {
        yOffset = (WORD)((SPLASH_SCREEN_HEIGHT - (PROGRESS_BAR_HEIGHT + PROGRESS_BAR_TOP_MARGIN)) / 2);
    }

    ProgressBarYOffset = yOffset + (WORD)(BmpHeader->Height + PROGRESS_BAR_HEIGHT + PROGRESS_BAR_TOP_MARGIN);
    ProgressBarXOffset = (SPLASH_SCREEN_WIDTH - PROGRESS_BAR_WIDTH)/2;

    ProgressBarFrameDrawn = FALSE;
}

/**
 * Set new progress value and update progress bar accordingly.
 * Progress value must be between 0 (0%) and 1000 (100%)
 */
void SetProgressValue(int progress)
{
    static int prevProgress = 0;

    if (FlatFrameBuffer == NULL || progress < 0 || progress > 1000)
    {
        return;
    }

    // Draw the progress bar frame if it's not there
    if (!ProgressBarFrameDrawn)
    {
        ProgressBarFrameDrawn = TRUE;

        // Horizontal lines
        DrawHLine(ProgressBarYOffset, ProgressBarXOffset+1, ProgressBarXOffset+PROGRESS_BAR_WIDTH-1, ProgressBarColourIndex);
        DrawHLine(ProgressBarYOffset+PROGRESS_BAR_HEIGHT, ProgressBarXOffset+1, ProgressBarXOffset+PROGRESS_BAR_WIDTH-1, ProgressBarColourIndex);

        // Vertical lines
        DrawVLine(ProgressBarXOffset, ProgressBarYOffset+1, ProgressBarYOffset+PROGRESS_BAR_HEIGHT-1, ProgressBarColourIndex);
        DrawVLine(ProgressBarXOffset+PROGRESS_BAR_WIDTH, ProgressBarYOffset+1, ProgressBarYOffset+PROGRESS_BAR_HEIGHT-1, ProgressBarColourIndex);
    }

    if (progress == 0)
    {
        DrawBox(ProgressBarXOffset+2, ProgressBarYOffset+2,
                PROGRESS_BAR_WIDTH-4, PROGRESS_BAR_HEIGHT-4,
                BlackColourIndex);
        prevProgress = 0;
        return;
    }
    else
    {
        if (prevProgress + PROGRESS_BAR_UPDATE_THRESHOLD < progress)
        {
            // Progress value increased
            DrawBox(ProgressBarXOffset+2 + prevProgress*(PROGRESS_BAR_WIDTH-4)/1000, ProgressBarYOffset+2,
                    (progress - prevProgress)*(PROGRESS_BAR_WIDTH-4)/1000 + 1, PROGRESS_BAR_HEIGHT-4,
                    ProgressBarColourIndex);
            prevProgress = progress;
        }
        else
        if (prevProgress - PROGRESS_BAR_UPDATE_THRESHOLD > progress)
        {
            // Progress value decreased
            DrawBox(ProgressBarXOffset+2 + progress*(PROGRESS_BAR_WIDTH-4)/1000 + 1, ProgressBarYOffset+2,
                    (prevProgress - progress)*(PROGRESS_BAR_WIDTH-4)/1000 + 1, PROGRESS_BAR_HEIGHT-4,
                    BlackColourIndex);
            prevProgress = progress;
        }
    }
}

/**
 * Draw a filled box
 */
void DrawBox(int x, int y, int w, int h, BYTE colour)
{
    for (; h >= 0; h--)
    {
        DrawHLine(y+h, x, x+w, colour);
    }
}

/**
 * Draw horizontal line
 */
void DrawHLine(int y, int xStart, int xStop, BYTE colour)
{
    int x;

    for (x = xStart; x < xStop; x++)
    {
        FlatFrameBuffer[SPLASH_SCREEN_WIDTH*y + x] = colour;
    }
}

/**
 * Draw vertical line
 */
void DrawVLine(int x, int yStart, int yStop, BYTE colour)
{
    int y;

    for (y = yStart; y < yStop; y++)
    {
        FlatFrameBuffer[SPLASH_SCREEN_WIDTH*y + x] = colour;
    }
}

#define abs(a) (a > 0 ? a : -a)

/**
 * Find index of a colour in the palette closest to the
 * colour given.
 */
BYTE FindColour(DWORD colour, DWORD * colourTable, ULONG length)
{
    BYTE currentColour = 0;
    LONG currentDiff = 0x7FFFFFFF, diff;

    while (length-- > 0)
    {
        diff =
            abs(((int)((colourTable[length] >> 16) & 0xFF) - (int)((colour >> 16) & 0xFF))) +
            abs(((int)((colourTable[length] >>  8) & 0xFF) - (int)((colour >>  8) & 0xFF))) +
            abs(((int)((colourTable[length]      ) & 0xFF) - (int)((colour      ) & 0xFF)));

        if (diff < currentDiff)
        {
            currentDiff = diff;
            currentColour = (BYTE)length;

            if (diff == 0)
            {
                break;
            }
        }
    }

    return currentColour;
}
