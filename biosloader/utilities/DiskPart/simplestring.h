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
// --------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// --------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <tchar.h>

class CSimpleString
{
public:
    CSimpleString();
    virtual ~CSimpleString();

    bool AllocateString( DWORD dwNumChars );
    bool ReAllocateString( DWORD dwNumChars );
    void FreeString();

    TCHAR* GetString() const;

    DWORD GetBufSizeInBytes() const;
    DWORD GetBufSizeInChars() const;

    bool Prepend( const TCHAR* strString );
    bool Append( const TCHAR* strString );

private:
    TCHAR* m_String;
    DWORD m_dwLength;
};


