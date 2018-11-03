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

// BIOS functions interface. See bios.asm for implementation.
ULONG BIOS_VBEControllerInfo();
ULONG BIOS_VBEModeInfo(VESA_MODE_INFO * pVesaInfo, WORD vesaMode);
ULONG BIOS_SetVBEMode(WORD vesaMode);
ULONG BIOS_SetVideoMode(BYTE mode);
void BIOS_WriteCharacterTeletype(ULONG page, ULONG color, CHAR character);

// Private function prototypes
BLSTATUS FindClosestVESAMode(BOOT_ARGS * pBootArgs);
BLSTATUS FindExactVESAMode(BOOT_ARGS * pBootArgs);
BLSTATUS FindDefaultVESAMode(BOOT_ARGS * pBootArgs);

// Far pointer to flat pointer conversion macro
#define FAR_TO_FLAT(farptr) ((farptr & 0x0000FFFF) + ((farptr & 0xFFFF0000) >> 12))

/**
 * Finds the best-matching VESA mode for the display screen properties
 * given in BOOT_ARGS. Adapted from loadcepc source code.
 */
BLSTATUS FindClosestVESAMode(BOOT_ARGS * pBootArgs)
{
    ULONG ulVbeStatus = 0;              // VBE calls return status
    VESA_GENERAL_INFO * vesaGeneralInfo
        = VESA_GENINFO_START;           // VBE controller information
    PWORD pVideoModeList;               // VBE video modes available
    VESA_MODE_INFO vesaModeInfo;        // VBE mode information

    ULONG i;
    ULONG ulError;
    ULONG ulCurrentError = 0xFFFFFFFF;

    // Init the signature
    vesaGeneralInfo->szSignature[0] = 'V';
    vesaGeneralInfo->szSignature[1] = 'B';
    vesaGeneralInfo->szSignature[2] = 'E';
    vesaGeneralInfo->szSignature[3] = '2';

    ulVbeStatus = BIOS_VBEControllerInfo();

    // Do we have VESA?
    if (ulVbeStatus != 0x004F || vesaGeneralInfo->wVersion < 0x0200)
    {
        ERRMSG(MSG_NO_VESA, ("No VESA-compliant video adapter\n"));
        return BLSTATUS_ERROR;
    }

    // vesaGeneralInfo.pModeList is a real-mode far pointer so
    // we need to convert it to protected-mode flat address
    pVideoModeList = (PWORD)FAR_TO_FLAT((DWORD)vesaGeneralInfo->pModeList);

    pBootArgs->vesaMode = 0;

    // Iterate over available video modes
    for (i = 0; pVideoModeList[i] != 0xFFFF; i++)
    {
        if ((ulVbeStatus = BIOS_VBEModeInfo(&vesaModeInfo, pVideoModeList[i]))
            != 0x004F)
        {
            WARNMSG(MSG_CANNOT_GET_MODE_INFO, ("Cannot get mode info (status=0x%x)\n", ulVbeStatus));
            break;
        }

        // The user specified dysplay dimensions and wants
        // us to find appropriate VESA mode
        if( (vesaModeInfo.ucNumberOfPlanes == 1)                          &&
            (vesaModeInfo.dwPhysBasePtr    != 0)                          &&
            (vesaModeInfo.ucBitsPerPixel   == pBootArgs->bppScreen)       &&
            (vesaModeInfo.wXResolution     >= pBootArgs->cxDisplayScreen) &&
            (vesaModeInfo.wYResolution     >= pBootArgs->cyDisplayScreen) )
        {
            // Only allow 565 16bpp modes through TODO: ???
            if( (vesaModeInfo.ucBitsPerPixel == 16) &&
                ((vesaModeInfo.ucRedMaskSize   != 5) ||
                 (vesaModeInfo.ucGreenMaskSize != 6) ||
                 (vesaModeInfo.ucBlueMaskSize  != 5)) )
            {
                continue;
            }

            // Compute screen resolution error
            ulError =
                (vesaModeInfo.wXResolution - pBootArgs->cxDisplayScreen) +
                (vesaModeInfo.wYResolution - pBootArgs->cyDisplayScreen);

            // If the error is diminishing save the mode data
            if (ulError < ulCurrentError)
            {
                // Save the current error for next comparison
                ulCurrentError = ulError;

                // Update the boot args with current data
                pBootArgs->vesaMode          = pVideoModeList[i];
                pBootArgs->pvFlatFrameBuffer = vesaModeInfo.dwPhysBasePtr;
                pBootArgs->cxPhysicalScreen  = vesaModeInfo.wXResolution;
                pBootArgs->cyPhysicalScreen  = vesaModeInfo.wYResolution;
                pBootArgs->cbScanLineLength  = vesaModeInfo.wBytesPerScanLine;
                pBootArgs->RedMaskSize       = vesaModeInfo.ucRedMaskSize;
                pBootArgs->RedMaskPosition   = vesaModeInfo.ucRedFieldPosition;
                pBootArgs->GreenMaskSize     = vesaModeInfo.ucGreenMaskSize;
                pBootArgs->GreenMaskPosition = vesaModeInfo.ucGreenFieldPosition;
                pBootArgs->BlueMaskSize      = vesaModeInfo.ucBlueMaskSize;
                pBootArgs->BlueMaskPosition  = vesaModeInfo.ucBlueFieldPosition;
            }
        }
    }

    if (ulCurrentError < 0xFFFFFFFF)
    {
        return BLSTATUS_OK;
    }
    else
    {
        return BLSTATUS_NO_VIDEO_MODE_FOUND;
    }
}

/**
 * Finds a VESA mode that exactly matches the physical screen properties
 * given in BOOT_ARGS. Adapted from loadcepc source code.
 */
BLSTATUS FindExactVESAMode(BOOT_ARGS * pBootArgs)
{
    ULONG ulVbeStatus = 0;              // VBE calls return status
    VESA_GENERAL_INFO * vesaGeneralInfo
        = VESA_GENINFO_START;           // VBE controller information
    PWORD pVideoModeList;               // VBE video modes available
    VESA_MODE_INFO vesaModeInfo;        // VBE mode information

    ULONG i;

    // Init the signature
    vesaGeneralInfo->szSignature[0] = 'V';
    vesaGeneralInfo->szSignature[1] = 'B';
    vesaGeneralInfo->szSignature[2] = 'E';
    vesaGeneralInfo->szSignature[3] = '2';

    ulVbeStatus = BIOS_VBEControllerInfo();

    // Do we have VESA?
    if (ulVbeStatus != 0x004F || vesaGeneralInfo->wVersion < 0x0200)
    {
        ERRMSG(MSG_NO_VESA, ("No VESA-compliant video adapter\n"));
        return BLSTATUS_ERROR;
    }

    // vesaGeneralInfo.pModeList is a real-mode far pointer so
    // we need to convert it to protected-mode flat address
    pVideoModeList = (PWORD)FAR_TO_FLAT((DWORD)vesaGeneralInfo->pModeList);

    pBootArgs->vesaMode = 0;

    // Iterate over available video modes
    for (i = 0; pVideoModeList[i] != 0xFFFF; i++)
    {
        if ((ulVbeStatus = BIOS_VBEModeInfo(&vesaModeInfo, pVideoModeList[i]))
            != 0x004F)
        {
            WARNMSG(MSG_CANNOT_GET_MODE_INFO, ("Cannot get mode %d info (status=0x%x)\n", pVideoModeList[i], ulVbeStatus));
            break;
        }

        if ((vesaModeInfo.ucNumberOfPlanes == 1)                           &&
            (vesaModeInfo.dwPhysBasePtr    != 0)                           &&
            (vesaModeInfo.ucBitsPerPixel   == pBootArgs->bppScreen)        &&
            (vesaModeInfo.wXResolution     == pBootArgs->cxPhysicalScreen) &&
            (vesaModeInfo.wYResolution     == pBootArgs->cyPhysicalScreen) )
        {
            // Conditions are met. Set BootArgs with Mode data and return.
            pBootArgs->vesaMode          = pVideoModeList[i];
            pBootArgs->pvFlatFrameBuffer = vesaModeInfo.dwPhysBasePtr;
            pBootArgs->cbScanLineLength  = vesaModeInfo.wBytesPerScanLine;
            pBootArgs->RedMaskSize       = vesaModeInfo.ucRedMaskSize;
            pBootArgs->RedMaskPosition   = vesaModeInfo.ucRedFieldPosition;
            pBootArgs->GreenMaskSize     = vesaModeInfo.ucGreenMaskSize;
            pBootArgs->GreenMaskPosition = vesaModeInfo.ucGreenFieldPosition;
            pBootArgs->BlueMaskSize      = vesaModeInfo.ucBlueMaskSize;
            pBootArgs->BlueMaskPosition  = vesaModeInfo.ucBlueFieldPosition;
            return BLSTATUS_OK;
        }
    }

    return BLSTATUS_NO_VIDEO_MODE_FOUND;
}

/**
 * Finds appropriate VESA mode
 */
BLSTATUS FindVESAMode(BOOT_ARGS * pBootArgs)
{
    // Check whether to search for closest or exact match
    if (pBootArgs->cxPhysicalScreen > 0 &&
        pBootArgs->cyPhysicalScreen > 0)
    {
        // Exact match needed
        return FindExactVESAMode(pBootArgs);
    }
    else
    {
        // Closest mode needed
        return FindClosestVESAMode(pBootArgs);
    }
}

/**
 * Finds a video mode that can accommodate default
 * display screen.
 */
BLSTATUS FindDefaultVESAMode(BOOT_ARGS * pBootArgs)
{
    pBootArgs->cxDisplayScreen     = DEFAULT_DISPLAY_WIDTH;
    pBootArgs->cyDisplayScreen     = DEFAULT_DISPLAY_HEIGHT;
    pBootArgs->cxPhysicalScreen    = 0;
    pBootArgs->cyPhysicalScreen    = 0;
    pBootArgs->bppScreen           = DEFAULT_DISPLAY_DEPTH;

    if (FindClosestVESAMode(pBootArgs) != BLSTATUS_OK)
    {
        ERRMSG(MSG_CANNOT_FIND_DEFAULT_VIDEO, ("Failed to find default video mode\n"));
        return BLSTATUS_ERROR;
    }

    return BLSTATUS_OK;
}

/**
 * Sets VESA mode.
 * Flat frame buffer flag will be OR'ed.
 */
BLSTATUS SetVESAMode(WORD vesaMode)
{
    vesaMode |= 0x4000;
    if (BIOS_SetVBEMode(vesaMode) == 0x004F)
    {
        return BLSTATUS_OK;
    }
    else
    {
        return BLSTATUS_ERROR;
    }
}

/**
 * Initializes video mode.
 * If user-requested mode cannot be found/set, an attempt to set
 * default screen mode will be made.
 *
 * Returns BLSTATUS_OK if either the user-requested or default
 * mode has been set.
 */
BLSTATUS InitVideo(BOOT_ARGS * pBootArgs)
{
    // Find appropriate VESA mode
    switch (FindVESAMode(pBootArgs))
    {
        case BLSTATUS_ERROR:
            return BLSTATUS_ERROR;

        case BLSTATUS_NO_VIDEO_MODE_FOUND:
            // Try to find a mode for the default settings
            WARNMSG(MSG_TRYING_DEFAULT_VIDEO, ("Requested video mode not found\n"));
            if (FindDefaultVESAMode(pBootArgs) != BLSTATUS_OK)
            {
                WARNMSG(MSG_TRYING_DEFAULT_VIDEO, ("Requested video mode not found\n"));
                return BLSTATUS_ERROR;
            }
            break;

        case BLSTATUS_OK:
            break;
    }

    // Set VESA mode
    if (SetVESAMode(pBootArgs->vesaMode) != BLSTATUS_OK)
    {
        WARNMSG(MSG_CANNOT_SET_VIDEO_MODE, ("Cannot set video mode %x\n", pBootArgs->vesaMode));
        if (FindDefaultVESAMode(pBootArgs) != BLSTATUS_OK)
        {
            WARNMSG(MSG_TRYING_DEFAULT_VIDEO, ("Requested video mode not found\n"));
            return BLSTATUS_ERROR;
        }

        if (SetVESAMode(pBootArgs->vesaMode) != BLSTATUS_OK)
        {
            WARNMSG(MSG_CANNOT_SET_VIDEO_MODE, ("Cannot set video mode %x\n", pBootArgs->vesaMode));
            return BLSTATUS_ERROR;
        }
    }

    return BLSTATUS_OK;
}

/**
 * Sets fallback video mode (320x200x256)
 */
void SetFallbackVideoMode(BOOT_ARGS * pBootArgs)
{
    BIOS_SetVideoMode(0x13);

    pBootArgs->cxDisplayScreen     = 320;
    pBootArgs->cyDisplayScreen     = 200;
    pBootArgs->cxPhysicalScreen    = 320;
    pBootArgs->cyPhysicalScreen    = 200;
    pBootArgs->bppScreen           = 8;
    pBootArgs->cbScanLineLength    = 320;
    pBootArgs->pvFlatFrameBuffer   = 0x800A0000;
}

/**
 * Puts a string on the screen in text-mode.
 *
 * Inputs
 *          text - zero-terminated string
 */
void WriteText(PCHAR text)
{
    while (*text != '\0')
    {
        // We'll use the default settings (0th page, 7th color)
        BIOS_WriteCharacterTeletype(0, 7, *text);
        text++;
    }
}

#if 0
void SetTextMode(BOOT_ARGS * pBootArgs)
{
    BIOS_SetVideoMode(0x03);

    pBootArgs->ucVideoMode         = 0;
    pBootArgs->cxDisplayScreen     = 0;
    pBootArgs->cyDisplayScreen     = 0;
    pBootArgs->cxPhysicalScreen    = 0;
    pBootArgs->cyPhysicalScreen    = 0;
    pBootArgs->bppScreen           = 0;
    pBootArgs->cbScanLineLength    = 0;
    pBootArgs->pvFlatFrameBuffer   = 0;
}
#endif
