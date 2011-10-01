#include <dirent.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <srofs.h>
#include <string.h>

#define MAX_PATH_LEN 118

struct fileentry {
	struct fileentry* next;
	struct srofs_fileentry* data;
}; 

struct fileentry* filelist = NULL;
struct srofs_superblock superblock;

void prepare_target(FILE* file, int size) {
	printf("Preparing target file with size %u...", size);
	
	while(size > 0) {
		size--;
		fputc(0, file);
	}
	fflush(file);
	
	printf(" done.\n");
}

void write_superblock(FILE* file) {
	fseek(file, 0, SEEK_SET); // Reset to beginning of target
	fwrite(&superblock, 1, sizeof(struct srofs_fileentry), file);
	fflush(file);
}

void write_fileindex(FILE* file) {

}

void write_filedata(FILE* file) {
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
			insert_fileentry(newentry);
		}
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
	prepare_target(target, targetsize);
	read_dir(root, rootdir, NULL);
	write_superblock(target);
	write_fileindex(target);
	write_filedata(target);
	
	closedir(root);
	fclose(target);
	return 0;
}
