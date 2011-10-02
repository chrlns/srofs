#include <dirent.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <srofs_structs.h>
#include <string.h>
#include <sys/stat.h>

char* stradd(char*, char*);

struct fileentry {
	struct fileentry* next;
	struct srofs_fileentry* data;
}; 

struct fileentry* filelist = NULL;
struct srofs_superblock superblock;

void paddto(FILE* file, int size) {
	while(ftell(file) < size) {
		fputc(0, file);
	}
	fflush(file);
}

void write_superblock(FILE* file) {
	fseek(file, 0, SEEK_SET); // Reset to beginning of target
	fwrite(&superblock, 1, sizeof(struct srofs_superblock), file);
	fflush(file);
}

void write_fileindex(FILE* file) {
	fseek(file, superblock.index_start * (1 << superblock.blocksize), SEEK_SET);
	struct fileentry* ptr = filelist;
	while(ptr != NULL) {
		fwrite(ptr->data, 1, sizeof(struct srofs_fileentry), file);
		ptr = ptr->next;
	}
	fflush(file);
}

void write_filedata(FILE* file, char* rootdir) {
	const int blksz = 1 << superblock.blocksize;
	char* path = stradd(rootdir, "/");
	struct fileentry* ptr = filelist;
	while(ptr != NULL) {
		char* fullpath = stradd(path, (char*)ptr->data->file_path);
		FILE* src = fopen(fullpath, "rb");
		if(!src) {
			printf("Warning: could not read source file %s\n", fullpath);
			continue;
		}
		while(!feof(src)) {
			int c = fgetc(src);
			fputc(c, file);
		}
		fclose(src);

		// Padd to next logical block boundary
		while(ftell(file) % blksz != 0) {
			fputc(0, file);
		}

		free(fullpath);
		ptr = ptr->next;
	}
}

void print_arguments() {
	printf("mksrofs [options] rootdir targetfile\n");
	printf("\t-s Target file size (default: 1 GiB)\n");
}

void insert_fileentry(struct srofs_fileentry* newentry) {
	struct fileentry* container = 
		(struct fileentry*)malloc(sizeof(struct fileentry));
	container->data = newentry;
	if(filelist == NULL) {
		container->next = NULL;
		filelist = container;
		superblock.num_files = 1;
	} else {
		superblock.num_files++;
		struct fileentry* last = NULL;
		struct fileentry* cur  = filelist;
		while(cur != NULL) {
			if(cur->data->file_path > newentry->file_path) {
				if(last == NULL) {
					filelist->next = container;
				} else {
					last->next = container;
				}
				container->next = cur;
				break;
			}
			last = cur;
			cur  = cur->next;
		}

		// Add new entry to the end of the list
		if(cur == NULL) {
			last->next = container;
			container->next = NULL;
		}
	}
}

char* stradd(char* a, char* b) {
	if(a == NULL) {
		return b;
	} else if(b == NULL) {
		return a;
	}

	int size = strlen(a) + strlen(b) + 1;
	char* newstr = (char*)malloc(sizeof(char) * size);
	strcpy(newstr, a);
	strcpy(newstr + strlen(a), b);
	newstr[size - 1] = 0;
	return newstr;
}

void read_dir(DIR* dir, char* base, char* parent) {
	struct dirent* entry;
	while((entry = readdir(dir))) {
		if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		char* path0 = stradd(base, "/");
		char* path1 = (parent == NULL) ? "" : stradd(parent, "/");
		char* path2 = stradd(path1, entry->d_name);
		char* path3 = stradd(path0, path2);
		DIR* cdir = opendir(path3);
		if(cdir) {
			read_dir(cdir, base, path2);
			closedir(cdir);
		} else if(strlen(path2) > MAX_PATH_LEN) {
			printf("Path too long, skipping: %s\n", path2);
		} else {
			// Entry is a file and path is not too long, very good!
			printf("Adding file %s\n", path2);
			struct srofs_fileentry* newentry = (struct srofs_fileentry*)
				malloc(sizeof(struct srofs_fileentry));
			strcpy((char*)newentry->file_path, path2);
			insert_fileentry(newentry);
		}
	}
}

// Returns the size of the given file (works up to 2G)
int fsize(const char* filename) {
	struct stat st;

	if(stat(filename, &st) == 0) {
		return st.st_size;
	}

	return -1; 
}

void fill_index(char* rootdir) {
	const int blksz = 1 << superblock.blocksize;
	char* path = stradd(rootdir, "/");

	// How many blocks are used by superblock and file index?
	int first_block = 1 + superblock.num_files * sizeof(struct srofs_fileentry) / blksz;
	if(superblock.num_files * sizeof(struct srofs_fileentry) % blksz != 0) {
		first_block++;
	}

	struct fileentry* ptr = filelist;
	while(ptr != NULL) {
		char* fullpath = stradd(path, (char*)ptr->data->file_path);
		int   size = fsize(fullpath);
		if(size < 0) {
			printf("Warning: cannot stat file %s\n", fullpath);
		}
		ptr->data->last_block_size = size % blksz;
		ptr->data->num_blocks = size % blksz;
		if(ptr->data->last_block_size > 0) {
			ptr->data->num_blocks++;
		}
		ptr->data->first_block = first_block;
		first_block += ptr->data->num_blocks;
		free(fullpath);
		ptr = ptr->next;
	}
}

int main(int argc, char* argv[]) {
	if(argc < 3) {
		print_arguments();
		return 1;
	}

	int targetsize = 0x40000000; // 1 GiB
	for(int n = 1; n < argc - 2; n++) {
		if(strcmp(argv[n], "-s") == 0) {
			n++;
			targetsize = atoi(argv[n]);
		}
	}

	memcpy(superblock.magic, "SROFS", 5);
	superblock.version     = 0;
	superblock.blocksize   = 9; // 2^9
	superblock.index_start = 1; // 2nd block

	char* rootdir    = argv[argc - 2];
	char* targetfile = argv[argc - 1];

	DIR* root = opendir(rootdir);
	if(!root) {
		perror("opendir()");
		return EXIT_FAILURE;
	}

	FILE* target = fopen(targetfile, "wb");
	if(!target) {
		perror("fopen()");
		return EXIT_FAILURE;
	}

	// Read files and construct the basic SROFS structures
	read_dir(root, rootdir, NULL);
	fill_index(rootdir);

	// Write the constructed filesystem to a real file
	write_superblock(target);
	paddto(target, 1 << superblock.blocksize);
	write_fileindex(target);
	paddto(target, ftell(target) + 
		(1 << superblock.blocksize) - ftell(target) % (1 << superblock.blocksize));
	write_filedata(target, rootdir);
	paddto(target, targetsize);	

	if(ftell(target) > targetsize) {
		printf("Warning: target size too small for specified files!\n");
	}

	closedir(root);
	fclose(target);
	return 0;
}
