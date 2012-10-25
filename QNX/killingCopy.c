nclude <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Proper usage: ./killingCopy file1 file2\n");
		return EXIT_SUCCESS;
	}


	char buffer[512];
	FILE *file1;
	FILE *file2;
	int readBytes = 512;

	file1 = fopen(argv[1], "r");
	if (file1 == NULL) {
		printf("Cannot open first file\n");
		return 1;
	}

	file2 = fopen(argv[2], "w");
	if(file2 == NULL) {
		printf("Cannot open second file\n");
		fclose(file1);
		return 1;
	}

	while(readBytes == 512) {
		readBytes = fread(&buffer, sizeof(char), 512, file1);

		fprintf(file2, "%d", readBytes);
		fprintf(file2, "\n");
	}

	fclose(file1);
	fclose(file2);

	return EXIT_SUCCESS;
}
