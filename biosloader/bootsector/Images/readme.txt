The boot sector goes in the first sector of the partition, or in the sector with the BPB for FAT volumes.
The size of the image is dependent on the size of the BPB.  FAT32 and exFAT have larger BPBs, and so the
amount of code that can be written in the sector is smaller.