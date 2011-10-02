#include <stdint.h>

#define MAX_PATH_LEN 118
#define NOALIGN __attribute__ ((aligned (1)))

struct srofs_superblock {
	NOALIGN uint8_t  magic[5];    // "SROFS" filesystem identifier
	NOALIGN uint8_t  version;     // Filesystem version number, currently 0
	NOALIGN uint8_t  blocksize;   // Real blocksize is 2^blocksize, default 2^9
	NOALIGN uint8_t  index_start; // Logical blocknumber of first file index block
	NOALIGN uint16_t num_files;   // Number of files in this filesystem
};

struct srofs_fileentry {
	NOALIGN uint8_t  file_path[MAX_PATH_LEN]; // NUL terminated string containing the 
                                                  // complete file path (ASCII chars), e.g.
                                                  //  "wwwroot/foo/bar.txt"
	NOALIGN uint32_t num_blocks;      // Number of used logical blocks
	NOALIGN uint32_t first_block;     // Logical block index of first data block
	NOALIGN uint16_t last_block_size; // Number of octets used in last data block
};
