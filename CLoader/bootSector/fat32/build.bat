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
@ECHO OFF

if NOT %WINCEDEBUG%. == debug. goto skip_debug

c:\masm611\bin\ml /DDEBUG=1 /Zm /Flbsect.lst /c bsect.asm 
goto endbld

:skip_debug
c:\masm611\bin\ml /Zm /Flbsect.lst /c bsect.asm 

:endbld
rem \masm611\bin\masm /Zm /Flbsect.lst /c bsect.asm
c:\masm5\link bsect.obj,bsect.com,,,,
..\clipfile bsect.com bsect.img 0xfc00 0x200
