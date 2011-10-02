#include "SROFS.h"

void SROFS_File::close() {
	this->fs->close(this);
}

int SROFS_File::read(uint8_t* buf, uint16_t len) {
	return this->fs->read(this, buf, len);
}

bool SROFS::begin() {
	sd.init(); // Initialize SD card with default values
	if(!sd.readData(0, 0, sizeof(struct srofs_superblock), reinterpret_cast<uint8_t*>(&superblock))) {
		return false;
	} else {
		return true;
	}
}

void SROFS::close(SROFS_File* file) {

}

int SROFS::open(const char* fileName, SROFS_File* file) {

}

int SROFS::read(SROFS_File* file, uint8_t* buf, uint16_t len) {

}
