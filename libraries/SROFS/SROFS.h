#include "Sd2Card.h"
#include "srofs_structs.h"

class SROFS_File;


static int8_t strcmp(uint8_t* a, uint8_t* b);

class SROFS {
	private:
		Sd2Card sd;
		struct srofs_superblock superblock;

		void   read_entry(int idx, struct srofs_fileentry* entry);

	public:
		// Open SD card and read superblock of the filesystem
		bool begin();

		// Close given file
		void close(SROFS_File* file);

		// Open file with given filename
		bool open(const char* fileName, SROFS_File* file);

		// Read from the given file into the given buffer
		int read(SROFS_File* file, uint8_t* buf, uint16_t len);
};

class SROFS_File {
	friend class SROFS;

	private:
		SROFS*   fs;  // Corresponding filesystem
		uint16_t idx; // Filelist index
		uint32_t ptr; // Current file pointer

	public:
		void close();
		int read(uint8_t* buf, uint16_t len);
};
