/*
How does rofs work?

You start with the header sector, 512 bytes
It contains:
FS signature ("ROFS")
FS Version (2 bytes, a short/word)
Partition Name as a LPS (length prefixed string)

Then follows files:

How do files work?
Filename as a LPS (word for length)
File path as a LPS (like /path/to/file.txt) (word for length)
file length as a qword
then raw file data

then other files
The last file on a rofs partition ends with a normal 0x0 EOF followed by a 0xFF End Of Disk (EOD) sector (512 times 0xFF)
*/