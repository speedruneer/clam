/*
How does brfs work?

You start with the header sector, 512 bytes
It contains:
FS signature ("BRFS")
FS Version (4 bytes, a long/dword)
Partition Name as a LPS (length prefixed string)

Then follows the BTBL (root directory node):
the block ID as a qword, root is 0x1
3 bytes indicating "DIR"
NTS filename (root is just "/")
then directory names like as NTS
"dev", "mnt", "usr"

Then block IDs for those directories (all qword, can be read as dword ignoring the higher part for 32 bit systems)
0x8837762, 0x83, 0x93

then file names like as NTS
"swap", "example.txt"

Then block IDs as qword

If there is more files or stuff than can be put on one block
put the next root directory block ID
it will not start with "DIR" but with "EXD"

files use the same principle with "EXF"
dirs are "EXD" and "DIR" yes

A first file block is
BLOCK ID
"FIL"
filename as a NTS (max of 64 chars for all NTS btw)
<metadata>: Metadata is handled differently but the default driver provided uses: creation timestamp: QWORD, file permission flag (OS independent, used mainly for linux)
<data>
next block ID (EOF == this is last block)

Any other file block is
BLOCK ID
"EXF"
<data>
next block ID (EOF == this is last block)

Symlinks are represented by:
BLOCK ID
"SYM"
Filename NTS
BLOCK ID of symlink, if it's EOF then it's an unitinialized symlink, which is bad
rest is just 0s
*/