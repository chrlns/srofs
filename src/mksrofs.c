#include <stdio.h>
#include <srofs.h>

void print_arguments() {
	printf("mksrofs [options] rootdir targetfile\n");
	printf("\t-s Target file size\n");
}

int main(int argc, char* argv[]) {
	if(argc < 3) {
		print_arguments();
		return 1;
	}
	return 0;
}
