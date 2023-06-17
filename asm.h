#pragma once

typedef struct Assembly {
	char* data;
	unsigned len;
} Assembly;

void asm_init(Assembly* assem);
void asm_add(Assembly* assem, char* data);
void asm_addf(Assembly* assem, char* fmt, ...);
