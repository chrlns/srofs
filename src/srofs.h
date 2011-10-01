#include <stdint.h>

#define MAX_PATH_LEN 118

struct srofs_superblock {
	uint8_t  magic[5];    // "SROFS" filesystem identifier
	uint8_t  version;     // Filesystem version number, currently 0
	uint8_t  blocksize;   // Real blocksize is 2^blocksize, default 2^9
	uint8_t  index_start; // Logical blocknumber of first file index block
	uint16_t num_files;   // Number of files in this filesystem
};

struct srofs_fileentry {
	uint8_t  file_path[MAX_PATH_LEN]; // NUL terminated string containing the 
                                          // complete file path (ASCII chars), e.g.
                                          //  "wwwroot/foo/bar.txt"
	uint32_t num_blocks;      // Number of used logical blocks
	uint32_t first_block;     // Logical block index of first data block
	uint16_t last_block_size; // Number of octets used in last data block
};
