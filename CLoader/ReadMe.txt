CLoader

CLoader is a Windows CE / Windows Embedded Compact boot loader that combines the features of the WCELDR boot 
loader from WEC 7 bootloader and the BIOSLOADER from Windows CE 6.

Like WECLDR, CLoader can be compiled within the standard CE tree without the need for special tools. (The one
exception is the boot sector code which requires Microsoft’s 16 bit MASM available on the MS website. Binary 
files containing FAT16 and FAT32 boot sectors are provided.) WCELDR supports both FAT16 and FAT32 formatted disks.   

Also like WCELDR, CLoader can either output information to the screen or to a serial port.

Like the BIOSLOADER from CE 6 and earlier versions, CLoader uses an INI file to configure the boot process. 
INI settings are available for video resolution, debug serial configuration, splash screen filename, .BIN file 
name and other parameters.

Finally, because the code is slimmed down and is designed to focus on one task, loading a .BIN file from the 
disk and jumping to it, the code for CLoader is fairly simple and easy to understand.  The bootloader is divided
into an XLDR and CLDR components that are merged in the last step of the build process. The XLDR component is 
loaded by the boot sector code. The XLDR then locates itself on the disk and loads the remainder of the bootloader
code. It then jumps to the CLDR component. 

The CLDR code starts in BootStart in CLoader\code\x86_bios\start.asm. It sets the stack and jumps to BootMain in
CLoader\code\x86_bios\main.c.  After some initial maintenance, the code calls OEMMain located in CLoader\cldr\main.c.  
The code is designed so that there should be no need to modify any code before OEMMain.
