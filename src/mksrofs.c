#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <srofs.h>
#include <string.h>

void prepare_target(FILE* file, int size) {
	printf("Preparing target file with size %u...", size);
	
	while(size > 0) {
		size--;
		fputc(0, file);
	}
	fflush(file);
	
	printf(" done.\n");
}

void print_arguments() {
	printf("mksrofs [options] rootdir targetfile\n");
	printf("\t-s Target file size (default: 1 GiB)\n");
}

void read_dir(DIR* dir) {
	struct dirent* entry;
	while((entry = readdir(dir))) {

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

	fclose(targetfile);
	return 0;
}
