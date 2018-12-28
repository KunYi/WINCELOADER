@REM
@REM use nasm to make boot sector, 512Byte
@REM
@ECHO OFF

..\nasm_win32\nasm.exe -o bsect.img -l bsect.lst bsect.s
