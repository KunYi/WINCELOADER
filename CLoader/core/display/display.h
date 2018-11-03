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

#pragma pack(push, 1)

typedef struct VbeInfoBlock_t {
    uint32_t signature;
    uint16_t version;
    uint16_t oemStringPtr[2];
    uint8_t  capabilities[4];
    uint16_t videoModesPtr[2];
    uint16_t totalMemory;
    uint16_t oemSoftwareRev;
    uint16_t oemVendorNamePtr[2];
    uint16_t oemProductNamePtr[2];
    uint16_t oemProductRevPtr[2];
    uint8_t reserved[222];
    uint8_t oemData[256];
} VbeInfoBlock_t;

typedef struct VbeModeInfoBlock_t {
    uint16_t modeAttributes;            // 0000
    uint8_t  winAAttributes;            // 0002
    uint8_t  winBAttributes;            // 0003
    uint16_t winGranularity;            // 0004
    uint16_t winSize;                   // 0006
    uint16_t winASegment;               // 0008
    uint16_t winBSegment;               // 000A
    uint16_t winFuncPrt[2];             // 000C
    uint16_t bytesPerScanLine;          // 0010
    
    uint16_t xResolution;               // 0012
    uint16_t yResolution;               // 0014
    uint8_t  xCharSize;                 // 0016
    uint8_t  yCharSize;                 // 0017
    uint8_t  numberOfPlanes;            // 0018
    uint8_t  bitsPerPixel;              // 0019
    uint8_t  numberOfBanks;             // 001A
    uint8_t  memoryModel;               // 001B
    uint8_t  bankSize;                  // 001C
    uint8_t  numberOfImagePages;        // 001D
    uint8_t  reserved1;                 // 001E

    uint8_t  redMaskSize;               // 001F
    uint8_t  redFieldPosition;          // 0020
    uint8_t  greenMaskSize;             // 0021
    uint8_t  greenFieldPosition;        // 0022
    uint8_t  blueMaskSize;              // 0023
    uint8_t  blueFieldPosition;         // 0024
    uint8_t  rsvdMaskSize;              // 0025
    uint8_t  rsvdFieldPosition;         // 0026
    uint8_t  directColorModeInfo;       // 0027

    uint32_t physBasePtr;               // 0028
    uint16_t reserved2[3];              // 002C

    uint16_t linBytesPerScanLine;       // 0032
    uint8_t  bnkNumberOfImagePages;     // 0033
    uint8_t  linNumberOfImagePages;     // 0034
    uint8_t  linRedMaskSize;            // 0035
    uint8_t  linRedFieldPosition;       // 0036
    uint8_t  linGreenMaskSize;          // 0037
    uint8_t  linGreenFieldPosition;     // 0038
    uint8_t  linBlueMaskSize;           // 0039
    uint8_t  linBlueFieldPosition;      // 003A
    uint8_t  linRsvdMaskSize;           // 003B
    uint8_t  linRsvdFieldPosition;      // 003C
    uint8_t  maxPixelClock;             // 003D
    
    uint8_t  reserved3[189];            // 002C
} VbeModeInfoBlock_t;

typedef struct VbeCrtcInfoBlock_t {
    uint16_t horizontalTotal;
    uint16_t horizontalSyncStart;
    uint16_t horizontalSyncEnd;
    uint16_t verticalTotal;
    uint16_t verticalSyncStart;
    uint16_t verticalSyncEnd;
    uint8_t  flags;
    uint32_t pixelClock;
    uint16_t refreshRate;
    uint8_t  reserved[40];
} VbeCrtcInfoBlock_t;

#pragma pack(pop)

//------------------------------------------------------------------------------

typedef struct DisplayMode_t {
    uint16_t vbeMode; 
    size_t width;
    size_t height;
    size_t bpp;
    uint32_t phFrame;
    size_t stride;
    enum_t redSize;
    enum_t redPos;
    enum_t greenSize;
    enum_t greenPos;
    enum_t blueSize;
    enum_t bluePos;
} DisplayMode_t;

typedef struct Display_t {
//    BootDriverVTable_t *pVTable;
    enum_t mode;
    enum_t modes;
    DisplayMode_t *aMode;
    void *pFrame;
} Display_t;

//------------------------------------------------------------------------------

bool_t
BootDisplayBiosDeinit(
    void *pContext
    );


static
bool_t
DisplayModeQuery(
    Display_t *pDisplay,
    enum_t *pMode,
    uint32_t *pWidth,
    uint32_t *pHeight,
    uint32_t *pBpp,
    uint32_t *pVesaMode,
    uint32_t *pPhFrame,
    uint32_t *pStride,
    uint32_t *pRedSize,
    uint32_t *pRedPos,
    uint32_t *pGreenSize,
    uint32_t *pGreenPos,
    uint32_t *pBlueSize,
    uint32_t *pBluePos
    );

static
bool_t
DisplayModeSet(
    Display_t *pDisplay,
    enum_t mode
    );

static
bool_t
DisplayFillRect(
    Display_t *pDisplay,
    RECT *pRect,
    uint32_t color
    );

static
bool_t
DisplayBltRect(
    Display_t *pDisplay,
    RECT *pRect,
    void *pBuffer
    );


//------------------------------------------------------------------------------

