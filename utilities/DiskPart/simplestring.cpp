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

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include "SimpleString.h"

CSimpleString::CSimpleString()
: m_String( NULL ),
  m_dwLength( 0 )
{
}

CSimpleString::~CSimpleString()
{
    FreeString();
}

bool CSimpleString::AllocateString( DWORD dwNumChars )
{
    m_dwLength = 0;
    FreeString();

    m_String = new TCHAR[dwNumChars];
    if( m_String )
    {
        ZeroMemory( m_String, dwNumChars * sizeof(TCHAR) );
        m_dwLength = dwNumChars;
        return true;
    }

    return false;
}

bool CSimpleString::ReAllocateString( DWORD dwNumChars )
{
    TCHAR* NewString = new TCHAR[dwNumChars];
    if( !NewString )
    {
        return false;
    }

    ZeroMemory( NewString, dwNumChars * sizeof(TCHAR) );
    CopyMemory( NewString, m_String, min(dwNumChars, m_dwLength) * sizeof(TCHAR) );

    FreeString();

    m_String = NewString;
    m_dwLength = dwNumChars;

    return true;
}

void CSimpleString::FreeString()
{
    if( m_String )
    {
        delete [] m_String;
        m_String = NULL;
    }

    m_dwLength = 0;
}

TCHAR* CSimpleString::GetString() const
{
    return m_String;
}

DWORD CSimpleString::GetBufSizeInBytes() const
{
    return m_dwLength * sizeof(TCHAR);
}

DWORD CSimpleString::GetBufSizeInChars() const
{
    return m_dwLength;
}

bool CSimpleString::Prepend( const TCHAR* strString )
{
    DWORD dwExistingLength = 0;
    DWORD dwNewLength = 0;

    dwExistingLength = _tcsclen( m_String ) + 1; // +1 for NULL
    dwNewLength = _tcsclen( strString );

    if( (dwExistingLength + dwNewLength) > GetBufSizeInChars() - 1 )
    {
        if( !ReAllocateString( dwExistingLength + dwNewLength + 20 ) )
        {
            return false;
        }
    }

    MoveMemory( m_String + dwNewLength, m_String, dwExistingLength * sizeof(TCHAR) );
    CopyMemory( m_String, strString, dwNewLength * sizeof(TCHAR) );

    return true;
}

bool CSimpleString::Append( const TCHAR* strString )
{
    DWORD dwExistingLength = 0;
    DWORD dwNewLength = 0;

    dwExistingLength = _tcsclen( m_String );
    dwNewLength = _tcsclen( strString );

    if( (dwExistingLength + dwNewLength) > GetBufSizeInChars() - 1 )
    {
        if( !ReAllocateString( dwExistingLength + dwNewLength + 20 ) )
        {
            return false;
        }
    }

    if( StringCchCat( m_String, GetBufSizeInChars(), strString ) != S_OK )
    {
        return false;
    }

    return true;
}
