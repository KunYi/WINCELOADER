@REM
@REM Copyright (c) Microsoft Corporation.  All rights reserved.
@REM
@REM
@REM Use of this sample source code is subject to the terms of the Microsoft
@REM license agreement under which you licensed this sample source code. If
@REM you did not accept the terms of the license agreement, you are not
@REM authorized to use this sample source code. For the terms of the license,
@REM please see the license agreement between you and Microsoft or, if applicable,
@REM see the LICENSE.RTF on your install media or the root of your tools installation.
@REM THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
@REM
@REM
@REM This batch file builds the boot sector code for the Windows CE BIOS bootloader.
@REM
@REM It relies on a 16-bit assembler and linker.  As well, it executes a debug.com
@REM script at the end to extract the boot sector text area from the resultant
@REM COM file.  Since the boot sector code runs in a loader-less environment, the
@REM better part of the COM file is of no use to us.
@REM
@REM ** NOTE: the debug.com script extracts just the text area of the boot sector
@REM    and doesn't include the BIOS parameter block (BPB) area.  This is because 
@REM    the BPB created by the format operation is used instead and an offset
@REM    (62d/0x3E) is applied to the boot sector code before it's written into
@REM    the boot sector.
@REM
@ECHO OFF

ml /Zm /Flbsect.lst /c bsect.asm
link bsect.obj,bsect.com,,,,
debug bsect.com < getbsect.scr
