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
#include <bootLog.h>

//------------------------------------------------------------------------------

static
void
GetFormatValue(
    wcstring_t *pFormat,
    long *pWidth,
    va_list *ppArgList
    );

//------------------------------------------------------------------------------

static
void
Reverse(
    wchar_t *pFirst,
    wchar_t *pLast
    );

//------------------------------------------------------------------------------

size_t
BootLogSPrintf(
    wstring_t buffer,
    size_t maxChars,
    wcstring_t format,
    ...
    )
{
    size_t size;
    va_list pArgList;

    va_start(pArgList, format);
    size = BootLogVSPrintf(buffer, maxChars, format, pArgList, TRUE);
    va_end(pArgList);

    return size;
}

//------------------------------------------------------------------------------
//
//  Function:  BootLogVSPrintf
//  
//  If emulateWPrintf is TRUE then %s and %S will be WCHAR* and CHAR*
//  If emulateWPrintf is FALSE then %s and %S will be CHAR* and WCHAR*
//  The above also holds for %c and %C
size_t
BootLogVSPrintf(
    wstring_t buffer,
    size_t maxChars,
    wcstring_t format,
    va_list pArgList,
    bool_t emulateWPrintf
    )
{
    static const wchar_t upch[]  = L"0123456789ABCDEF";
    static const wchar_t lowch[] = L"0123456789abcdef";
    enum { typeNone = 0, typeNumber, typeCh, typeString } type;
    enum { modeNone = 0, modeH, modeL, modeX } mode;
    bool_t padLeft, prefix, sign, upper;
    int32_t width, radix = 0, precision;
    size_t chars;
    wchar_t ch, fillCh;
    wstring_t pos, pW;
    string_t pC;
    int64_t value;


    // First check input params
    if ((buffer == NULL) || (format == NULL) || (maxChars < 1)) return 0;

    // Set actual possition
    pos = buffer;

    // While there is format strings
    while ((*format != L'\0') && (maxChars > 0))
        {

        // If it is other than format prefix, print it and move to next one
        if (*format != L'%')
            {
            if (--maxChars <= 0) goto cleanUp;
            *pos++ = *format++;
            continue;
            }

        // Set flags to default values
        padLeft = false;
        prefix = false;
        sign = false;
        upper = false;
        fillCh = L' ';
        width = 0;
        precision = -1;
        type = typeNone;
        mode = modeNone;

        // read pad left and prefix flags
        while (*++format != L'\0')
            {
            if (*format == L'-')
                {
                padLeft = true;
                }
            else if (*format == L'#')
                {
                prefix = true;
                }
            else
                {
                break;
                }
            }

        // Find fill character
        if (*format == L'0')
            {
            fillCh = L'0';
            format++;
            }

        // Read the width specification
        GetFormatValue(&format, (long *)&width, &pArgList);

        // Read the precision
        if (*format == L'.')
            {
            format++;
            GetFormatValue(&format, (long *)&precision, &pArgList);
            }

        // Get the operand size
        if (*format == L'l')
            {
            mode = modeL;
            format++;
            }
        else if (*format == L'w')
            {
            mode = modeL;
            format++;
            }
        else if ((format[0] == L'I') && (format[1] == L'3') &&
                    (format[2] == L'2'))
            {
            mode = modeL;
            format += 3;
            }
        else if (*format == L'h')
            {
            mode = modeH;
            format++;
            }
        else if ((format[0] == L'I') && (format[1] == L'6') &&
                    (format[2] == L'4'))
            {
            mode = modeX;
            format += 3;
            }

        // Break if there is unexpected format string end
        if (*format == L'\0') break;

        switch (*format)
            {
            case L'i':
            case L'd':
                sign = true;
                radix = 10;
                type = typeNumber;
                if (mode == modeNone) mode = modeL;
                break;

            case L'u':
                sign = false;
                radix = 10;
                type = typeNumber;
                if (mode == modeNone) mode = modeL;
                break;

            case L'X':
                upper = true;
                radix = 16;
                type = typeNumber;
                if (mode == modeNone) mode = modeL;
                break;

            case L'p':
            case L'x':
                radix = 16;
                type = typeNumber;
                if (mode == modeNone) mode = modeL;
                break;

            case L'c':
                if (mode == modeNone) 
                    {
                        if(emulateWPrintf) mode = modeL;
                        else mode = modeH;
                    }
                type = typeCh;
                break;

            case L'C':
                if (mode == modeNone) 
                    {
                        if(emulateWPrintf) mode = modeH;
                        else mode = modeL;
                    }
                type = typeCh;
                break;

            case L'a':
                mode = modeH;
                type = typeString;
                break;

            case L'S':
                if (mode == modeNone) 
                    {
                        if(emulateWPrintf) mode = modeH;
                        else mode = modeL;
                    }
                type = typeString;
                break;

            case 's':
                if (mode == modeNone) 
                    {
                        if(emulateWPrintf) mode = modeL;
                        else mode = modeH;
                    }
                type = typeString;
                break;

            default:
                if (--maxChars <= 0) goto cleanUp;
                *pos++ = *format;
            }

        // Move to next format character
        format++;

        switch (type)
            {
            case typeNumber:
                // Special cases to act like MSC v5.10
                if (padLeft || precision >= 0) fillCh = L' ';
                // Fix possible prefix
                if (radix != 16) prefix = false;
                // Depending on mode obtain value
                if (mode == modeH)
                    {
                    if (sign)
                        {
                        value = (int64_t)va_arg(pArgList, int16_t);
                        }
                    else
                        {
                        value = (int64_t)va_arg(pArgList, uint16_t);
                        }
                    }
                else if (mode == modeL)
                    {
                    if (sign)
                        {
                        value = (int64_t)va_arg(pArgList, int32_t);
                        }
                    else
                        {
                        value = (int64_t)va_arg(pArgList, uint32_t);
                        }
                    }
                else if (mode == modeX)
                    {
                    value = va_arg(pArgList, int64_t);
                    }
                else
                    {
                    goto cleanUp;
                    }
                // Should sign be printed?
                if (sign && ((int64_t)value < 0))
                    {
                    (int64_t)value = -(int64_t)value;
                    }
                else
                    {
                    sign = false;
                    }
                // Start with reverse string
                pW = pos;
                chars = 0;
                do
                    {
                    if (--maxChars <= 0) goto cleanUp;
                    *pW++ = upper ? upch[value % radix] : lowch[value % radix];
                    chars++;
                    }
                while (((value /= radix) != 0) && (maxChars > 0));
                // Fix sizes
                width -= chars;
                precision -= chars;
                if (precision > 0) width -= precision;
                // Fill to the field precision
                while (precision-- > 0)
                    {
                    if (--maxChars <= 0) goto cleanUp;
                    *pW++ = L'0';
                    }
                if ((width > 0) && !padLeft)
                    {
                    // If we're filling with spaces, put sign first
                    if (fillCh != L'0')
                        {
                        if (sign)
                            {
                            if (--maxChars <= 0) goto cleanUp;
                            *pW++ = L'-';
                            width--;
                            sign = false;
                            }
                        if (prefix && (radix == 16))
                            {
                            if (--maxChars <= 0) goto cleanUp;
                            *pW++ = upper ? L'X' : L'x';
                            if (--maxChars <= 0) goto cleanUp;
                            *pW++ = L'0';
                            prefix = false;
                            }
                        }
                    // Leave place for sign
                    if (sign) width--;
                    // Fill to the field width
                    while (width-- > 0)
                        {
                        if (--maxChars <= 0) goto cleanUp;
                        *pW++ = fillCh;
                        }
                    // Still have sign?
                    if (sign)
                        {
                        if (--maxChars <= 0) goto cleanUp;
                        *pW++ = L'-';
                        sign = false;
                        }
                    // Or prefix?
                    if (prefix)
                        {
                        if (--maxChars <= 0) goto cleanUp;
                        *pW++ = upper ? L'X' : L'x';
                        if (--maxChars <= 0) goto cleanUp;
                        *pW++ = L'0';
                        prefix = false;
                        }
                    // Now reverse the string in place
                    Reverse(pos, pW - 1);
                    pos = pW;
                    }
                else
                    {
                    // Add the sign character
                    if (sign)
                        {
                        if (--maxChars <= 0) goto cleanUp;
                        *pW++ = L'-';
                        sign = false;
                    }
                    if (prefix)
                        {
                        if (--maxChars <= 0) goto cleanUp;
                        *pW++ = upper ? L'X' : L'x';
                        if (--maxChars <= 0) goto cleanUp;
                        *pW++ = L'0';
                        prefix = false;
                        }
                    // Reverse the string in place
                    Reverse(pos, pW - 1);
                    pos = pW;
                    // Pad to the right of the string in case left aligned
                    while (width-- > 0)
                        {
                        if (--maxChars <= 0) goto cleanUp;
                        *pos++ = fillCh;
                        }
                    }
                break;

            case typeCh:
                // Depending on size obtain value
                if (mode == modeH)
                    {
                    ch = (wchar_t)va_arg(pArgList, char);
                    }
                else if (mode == modeL)
                    {
                    ch = va_arg(pArgList, wchar_t);
                    }
                else
                    {
                    goto cleanUp;
                    }
                if (--maxChars <= 0) goto cleanUp;
                *pos++ = ch;
                break;

            case typeString:
                if (mode == modeH)
                    {
                    // It is ascii string
                    pC = va_arg(pArgList, string_t);
                    if (pC == NULL) pC = "(NULL)";
                    // Get string size
                    chars = 0;
                    while (chars < maxChars && pC[chars] != '\0') chars++;
                    // Fix string size
                    if ((precision >= 0) && (chars > (size_t)precision))
                        {
                        chars = precision;
                        }
                    width -= chars;
                    if (padLeft)
                        {
                        while (chars-- > 0)
                            {
                            if (--maxChars <= 0) goto cleanUp;
                            *pos++ = (wchar_t)*pC++;
                            }
                        while (width-- > 0)
                            {
                            if (--maxChars <= 0) goto cleanUp;
                            *pos++ = fillCh;
                            }
                        }
                    else
                        {
                        while (width-- > 0)
                            {
                            if (--maxChars <= 0) goto cleanUp;
                            *pos++ = fillCh;
                            }
                        while (chars-- > 0)
                            {
                            if (--maxChars <= 0) goto cleanUp;
                            *pos++ = (wchar_t)*pC++;
                            }
                        }
                    }
                else if (mode == modeL)
                    {
                    // It is unicode string
                    pW = va_arg(pArgList, wstring_t);
                    if (pW == NULL) pW = L"(NULL)";
                    // Get string size
                    chars = 0;
                    while ((chars < maxChars) && (pW[chars] != L'\0')) chars++;
                    // Fix string size
                    if ((precision >= 0) && (chars > (size_t)precision))
                        {
                        chars = precision;
                        }
                    width -= chars;
                    if (padLeft)
                        {
                        while (chars-- > 0)
                            {
                            if (--maxChars <= 0) goto cleanUp;
                            *pos++ = *pW++;
                            }
                        while (width-- > 0)
                            {
                            if (--maxChars <= 0) goto cleanUp;
                            *pos++ = fillCh;
                            }
                        }
                    else
                        {
                        while (width-- > 0)
                            {
                            if (--maxChars <= 0) goto cleanUp;
                            *pos++ = fillCh;
                            }
                        while (chars-- > 0)
                            {
                            if (--maxChars <= 0) goto cleanUp;
                            *pos++ = *pW++;
                            }
                        }
                    }
                else
                    {
                    goto cleanUp;
                    }
                break;
            }

        }

cleanUp:
    *pos = L'\0';
    return (pos - buffer);
}

//------------------------------------------------------------------------------

static
void
GetFormatValue(
    wcstring_t *pFormat,
    long *pWidth,
    va_list *ppArgList
    )
{
    long width = 0;


    if (**pFormat == L'*')
        {
        *pWidth = va_arg(*ppArgList, int);
        (*pFormat)++;
        }
    else
        {
        while ((**pFormat >= L'0') && (**pFormat <= L'9'))
            {
            width = width * 10 + (**pFormat - L'0');
            (*pFormat)++;
            }
        *pWidth = width;
        }
}

//------------------------------------------------------------------------------

static
void
Reverse(
    wchar_t *pFirst,
    wchar_t *pLast
    )
{
    int swaps;
    wchar_t ch;

    swaps = (pLast - pFirst + 1) >> 1;
    while (swaps--)
        {
        ch = *pFirst;
        *pFirst++ = *pLast;
        *pLast-- = ch;
        }
}

//------------------------------------------------------------------------------

