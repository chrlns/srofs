class SROFS_File {
	private:
		
};

class SROFS {
	private:

	public:
		void close(SROFS_File* file);
		SROFS_File open(const char* fileName);
		uint16_t read(SROFS_File* file, uint8_t* buf, uint32_t off, uint16_t len);
}:
