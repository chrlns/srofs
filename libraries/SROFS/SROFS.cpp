#include "SROFS.h"

void SROFS_File::close() {
	this->fs->close(this);
}

int SROFS_File::read(uint8_t* buf, uint16_t len) {
	return this->fs->read(this, buf, len);
}

bool SROFS::begin() {
	sd.init(); // Initialize SD card with default values
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
	/*int min = 0;
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
	}*/
	for(int n = 0; n < superblock.num_files; n++) {
		read_entry(n, &entry);
		if(strcmp((uint8_t*)file_path, entry.file_path) == 0) {
			file->idx = n;
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

int8_t SROFS::strcmp(uint8_t* a, uint8_t* b) {
	for(int n = 0; a[n] != 0 && b[n] != 0; n++) {
		if(a[n] < b[n]) {
			return -1;
		}
		if(a[n] > b[n]) {
			return 1;
		}
	}
	return 0;
}
