#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "nyan.h"
#include "util.h"

int main(int argc, char** argv) {
    setlocale(LC_ALL, ""); // set the locale to the user's default locale

	if (argc < 3) {
		printf("Usage: nyan <filename> <output>\n");
		return EXIT_FAILURE;
	}

    char* filename = argv[1];
	char* output = argv[2];

	Assembly* assem = malloc(sizeof(Assembly));

    struct nyan_s nyan = parse_nyan(filename);
	compile_nyan(nyan, assem);

	FILE* f = fopen("nyan.s", "w");
	printf("%s", assem->data);
	fprintf(f, "%s", assem->data);
	fclose(f);

	char* cmd = "nasm -f elf64 nyan.s -o nyan.o && ld nyan.o -o %s";
	char* cmd2 = malloc(sizeof(char) * 1024);
	sprintf(cmd2, cmd, output);
	system(cmd2);

	// clean up
	free(cmd2);
	free(assem);

	// delete temp files
	remove("nyan.s");
	remove("nyan.o");

    return EXIT_SUCCESS;
}

