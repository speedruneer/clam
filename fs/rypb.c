/*
How does RyPB work?

It essentially replaces the MBR by going just before it, it takes variable amount of bytes
each entry is:
(for 64 bit, turn 8->4 for the 32 bit version)
starting LBA 8 bytes
ending LBA 8 bytes
FS Type: 2 bytes

FS Types supported:
Linux SWAP
Solaris SWAP
EXT2
EXT3
EXT4
EXT
BTRFS
NTFS
FAT12
FAT16
FAT32
ExFAT
VFAT32
LFN FAT12
LFN FAT16
LFN FAT32
LFN VFAT32
ISO9660
UDF
RyFS
BRFS
ROFS
ROMFS
HPFS
HFS
HFS+
Joliet
ZFS
XFS
USTAR
ZDSFS
SFS
MFS
FFS (Amiga)
FFS/UFS (BSD)
LEAN
BFS
BeFS
*/