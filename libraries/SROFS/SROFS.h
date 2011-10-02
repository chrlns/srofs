#include "Sd2Card.h"
#include "srofs_structs.h"

class SROFS;

class SROFS_File {
	private:
		SROFS*   fs;  // Corresponding filesystem
		uint32_t ptr; // File pointer

	public:
		void close();
		int read(uint8_t* buf, uint16_t len);
};

class SROFS {
	private:
		Sd2Card sd;
		struct srofs_superblock superblock;

	public:
		// Open SD card and read superblock of the filesystem
		bool begin();

		// Close given file
		void close(SROFS_File* file);

		// Open file with given filename
		int open(const char* fileName, SROFS_File* file);

		// Read from the given file into the given buffer
		int read(SROFS_File* file, uint8_t* buf, uint16_t len);
};
