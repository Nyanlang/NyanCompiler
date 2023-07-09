#include "asm.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#define MAX_ASM 10000000

void asm_init(Assembly* assm) {
	assm->data = malloc(sizeof(char) * MAX_ASM);
	strcpy(assm->data, "");
	assm->len = 0;
}

void asm_add(Assembly* assm, char* str) {
	strncat(assm->data, str, MAX_ASM);
	strncat(assm->data, "\n", MAX_ASM);
	// null terminate
	assm->len++;
}

void asm_addf(Assembly* assm, char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char* str = malloc(sizeof(char) * MAX_ASM);
	vsprintf(str, fmt, args);
	asm_add(assm, str);
	free(str);
	va_end(args);
}
