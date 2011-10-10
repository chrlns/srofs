#include <WProgram.h>
#include "SROFS.h"

void SROFS_File::close() {
	this->fs->close(this);
}

int SROFS_File::read(uint8_t* buf, uint16_t len) {
	return this->fs->read(this, buf, len);
}

bool SROFS::begin() {
	pinMode(10, OUTPUT);        // set the SS pin as an output (necessary!)
	digitalWrite(10, HIGH);     // but turn off the W5100 chip!
	sd.init(SPI_HALF_SPEED, 4); // Initialize SD card with default values
	if(!sd.readData(0, 0, sizeof(struct srofs_superblock), (uint8_t*)(&superblock))) {
		return false;
	} else {
		return superblock.magic[0] == 'S' &&
			superblock.magic[1] == 'R' &&
			superblock.magic[2] == 'O' &&
			superblock.magic[3] == 'F' &&
			superblock.magic[4] == 'S';
	}
}

void SROFS::close(SROFS_File* file) {

}

bool SROFS::open(const char* file_path, SROFS_File* file) {
	file->fs  = this;
	file->ptr = 0;

	// Binary lookup for the file
	struct srofs_fileentry entry; // 128 bytes large, use it carefully
	int min = 0;
	int max = this->superblock.num_files - 1;
	while(min <= max) {
		file->idx = (min + max) / 2;
		read_entry(file->idx, &entry);
		int cmp = strcmp((uint8_t*)file_path, entry.file_path);
		if(cmp < 0) {
			max = file->idx - 1;
		} else if(cmp > 0) {
			max = file->idx + 1;
		} else {
			// We found the file
			return true;
		}	
	}
	return false;
}

int SROFS::read(SROFS_File* file, uint8_t* buf, uint16_t len) {
	struct srofs_fileentry entry;
	int blksz = (1 << superblock.blocksize);
	read_entry(file->idx, &entry);

	int filesize = blksz * (entry.num_blocks - 1) + entry.last_block_size;
	if(file->ptr >= filesize) {
		return -1;
	} 
	if(file->ptr + len >= filesize) {
		len = filesize - file->ptr;
	}

	int block  = entry.first_block + file->ptr / blksz;
	int offset = file->ptr % blksz;

	sd.readData(block, offset, len, buf);

	file->ptr += len;
	return len;
}

void SROFS::read_entry(int idx, struct srofs_fileentry* entry) {
	// We have an index, but we need the block number
	int entriespblk = (1 << superblock.blocksize) / sizeof(struct srofs_fileentry);
	int block  = superblock.index_start + idx / entriespblk;
	int offset = (idx % entriespblk) * sizeof(struct srofs_fileentry);

	sd.readData(block, offset, sizeof(struct srofs_fileentry), (uint8_t*)entry);
}

// Compares two strings and returns 0 if both strings are equal, -1 if
// string a is smaller than b and +1 if a is greater than b.
// The implementation uses recursion which is hopefully optimized by the
// compiler.
static int8_t strcmp(uint8_t* a, uint8_t* b) {
	if(*a == 0 && *b == 0) {
		return 0;
	}
	
	if(*a < *b) {
		return -1;
	}

	if(*a > *b) {
		return 1;
	}

	return strcmp(a + 1, b + 1);
}
