Simple Read Only Filesystem (SROFS)
===================================

SROFS is a simple design of a readonly filesystem for embedded systems such as
Atmel ATmega based microcontroller boards (Arduino in mind).

Features (as of version 0 of the filesystem):
 - max. number of files: 65536
 - max. block size: 64K
 - max. file path (not name) length: 117 chars + '\0' terminator 

Structures
==========

A SROFS filesystem consists of
 - a superblock (starting at device/partition block 0)
 - a file index
 - one or more files

There are no directories although the filesystem API may provide virtual access
to directory structures.

Superblock:
-----------
<pre>
[0-4]      MAGIC            "SROFS" filesystem identifier
[5  ]      VERSION          SROFS version number, currently 0
[6  ]      BLOCKSIZE        Size of a logical filesystem block, 2^BLOCKSIZE
                            default 9 = 2^9 = 512 byte blocks
[7  ]  	   INDEX_START      Logical blocknumber of first file index block
[8-9]      NUM_FILES        Number of files in this filesystem.
</pre>

File Index:
-----------
<pre>
[File Entry]*               NUM_FILES times the file entry structure; the file 
                            entries are orded alphabetically (see later).
</pre>

File Entry:
-----------
<pre>
[0-117]    FILE_PATH        NUL terminated string containing the complete file 
                            path (ASCII), e.g. "wwwroot/foo/bar.txt"
[118-121]  NUM_BLOCKS  	    Number of used logical blocks 
[122-125]  FIRST_BLOCK      Logical block index of first data block
[126-127]  LAST_BLOCK_SIZE  Number of octets used in last data block
</pre>

File lookup
===========

File lookup can be done using the following (pseudo) code for a binary search:

<pre>
min = 0
max = NUM_FILES - 1

WHILE min <= max
    m = (min + max) / 2
    IF file < entry[m]
        max = m - 1
    ELSE IF file > entry[m]
        min = m + 1
    ELSE
        RETURN entry[m]
END WHILE

RETURN File not found
</pre>
