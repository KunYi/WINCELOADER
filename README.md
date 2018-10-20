# BIOSLOADER
Special version for WINCE/WEC, clone from "http://geekswithblogs.net/WernerWillemsens/archive/2015/08/03/x86-bootloader-for-wce8.aspx"

---
A bootloader consists of many sub-bootloaders
1. A fixed HDD has a Master Boot Record in its first (0th) sector. A MBR is always 512bytes in size, despite how the disk is organized.
   
   The MBR lists up to maximum 4 partitions, also referred to as volumes.

2. Each Volume has its own bootloader part, again 512 bytes in size.

    Disk with 4 partitions can have only 1 active partition. This is the partition which holds the (only) partition bootloader.

    The other 3 partitions are considered as non-bootable partitions.

3. A removable HDD (USB stick) is typically formatted by desktop Windows as a single partition/volume. 
   
   Unless the disk was formatted before as a fixed disk with 4 partitions. 
   
   Formatting in desktop Windows always tries to look at what how the disk was formatted before.
   If you clean the disk (writing all zeros) by special software, the disk is blank and only than desktop Windows will format it as a single partition/volume.

4. Booting on a (fixed) HDD is thus a multi boot step
   * MBR (only for fixed HDD)
   * Partition bootloader (on active partition)
   * OS boot loader
   
In Windows CE we always build the

1. Partition bootlader (BSECT.IMG), always 512 bytes. Written in assembler
2. OS boot loader (BLDR), 20-24kB in size. Written in C with some parts in assembler. 

The OS boot loader in Windows CE uses the BIOS to call

	  - INT13h for disk access
	  - INT10H for video access
	  - INT16H for keyboard access

BIOS calls need to be executed in Real mode (16bit),while most code of the OS bootloader runs in Protected mode (32bit)

Therefore BLDR jumps from Protected to Real mode and vice versa to call BIOS functionality.

Hence the OS bootloader is sometimes referred to as the BIOSLOADER.

Windows CE has code for FAT12/16/32 and exFAT. We typically build for FAT32, which is most supported in the field and by all Windows versions.

I. Building BSECT
-----------------
1. Go to C:\WINCE600\PLATFORM\ETX_2740\SRC\BOOTLOADER\BIOSLOADER\BOOTSECTOR\FAT32
2. Run from Release build window 'build2.bat'
3. This builds a FAT32 compatible partition bootloader.
4. The result is BSECT.IMG, a binary image of 512 bytes.

II. Building BLDR
-----------------
1. Go to C:\WINCE600\PLATFORM\ETX_2740\SRC\BOOTLOADER\BIOSLOADER\INIPARSER
2. Run from Release build window 'build -c'. It is important to build the Release version to fit in 64k memory.
3. Go to C:\WINCE600\PLATFORM\ETX_2740\SRC\BOOTLOADER\BIOSLOADER\LOADER\FIXED
4. Run from Release build window 'build -c'. It is important to build the Release version to fit in 64k memory.
5. Go to C:\WINCE600\PLATFORM\ETX_2740\SRC\BOOTLOADER\BIOSLOADER\LOADER\FIXED\FAT32
6. Run from Release build window 'makebldr2.bat'
7. The result is BLDR, a binary image of 24k bytes that runs in 64k memory space.
   'makebldr2.bat' uses the 'build' tool that can only work in 32bit Windows and cannot be run from a full path 
   with more than 64 characters. If so, copy the contents of FIXED\FAT32 folder to a folder with less characters
   before executing 'makebldr2.bat'
   
 III. Creating bootable disk
 ---------------------------
    Use desktop Windows CeSys.exe tool (see adapted version in $PGX\Main\Tools\CeSys) to write
    
    BSECT and BLDR to an active partition/volume on a clean FAT32 formatted disk.