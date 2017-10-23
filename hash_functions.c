#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Complete these two functions according to the assignment specifications
#define BLOCK_SIZE 8

char *hash(FILE *file) {
	char *hash_val = malloc(sizeof(char) * 9);
	int i;
	for (i = 0; i < 9; i++) {
		hash_val[i] = '\0';
	}
	char *buffer;
	long filelen;

	fseek(file, 0, SEEK_END);
	filelen = ftell(file);
	rewind(file);
	buffer = (char *) malloc((filelen + 1) * sizeof(char));
	buffer[filelen] = '\0';
	int index = 0;
	for (i = 0; i < filelen; i++) {
		fread(&buffer[index], 1, 1, file);
		index++;
		}

	int x = 0;
	i = 0;
	while (i < filelen) {
		hash_val[x] = hash_val[x] ^ buffer[i];
		x++;
		i++;
		if (x == 8){
			x = 0;
		}
	}
	fclose(file);
	return hash_val;
}
