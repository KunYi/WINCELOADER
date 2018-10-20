@REM
@REM Copyright (c) Microsoft Corporation.  All rights reserved.
@REM
@REM
@REM Use of this source code is subject to the terms of the Microsoft end-user
@REM license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
@REM If you did not accept the terms of the EULA, you are not authorized to use
@REM this source code. For a copy of the EULA, please see the LICENSE.RTF on your
@REM install media.
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

@REM Notes WWI:

@REM edited to work with MSVC\BIN\link
@REM  /omf             : Intel Object Module Format
@REM  C:\MSVC\BIN\link : DOS 16bit linker (reading OMF obj files)
@REM  debug script seems to offset x200 bytes : FA5A + 1A6 -> FC5A + 1A6
@REM  0x1A6 (422d) + 0x5A (90d = BPB fat32) = 512

@REM alternatively MASM32 could be used (www.masm32.com)

@REM 'debug' is a fishy and obscure (oldschool DOS) tool that can be used to extract 
@REM bytes from a binary file (among other low level bare metal things it can do...)
@REM 'debug' however only runs on 32bit Windows OS, not anymore on 64bit Windows.

@ECHO OFF

ml /Zm /Flbsect.lst /c /omf bsect.asm
C:\MSVC\BIN\link bsect.obj,bsect.com,,,,
debug bsect.com < getbsect.scr
