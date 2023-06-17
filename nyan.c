#include <malloc.h>
#include <stdio.h>
#include <wchar.h>
#include <fcntl.h>
#include <stdint.h>
#include "nyan.h"
#include "mouse.h"
#include "util.h"

struct jump_pair {
	int jz;
	int jnz;
};

void set_jump_pair(struct nyan_s *nyan, struct jump_pair *jump_pairs, int *jplen) {
	int stack[nyan->len];
	int stack_idx = 0;

	for (int i = 0; i < nyan->len; i++) {
		switch (nyan->commands[i]) {
			case JUMP_ZERO:
				stack[stack_idx] = i;
				stack_idx++;
			break;
			case JUMP_NON_ZERO:
				stack_idx--;
				jump_pairs[*jplen].jz = stack[stack_idx];
				jump_pairs[*jplen].jnz = i;
				(*jplen)++;
			break;
			default:
				// do nothing
			break;
		}
	}

	if (stack_idx > 0) {
		printf("Error: unmatched [ found\n");
		}
	else if (stack_idx < 0) {
		printf("Error: unmatched ] found\n");
	}
}

int get_matching_jz(struct jump_pair* jp, int jnz, int jplen) {
	for (int i = 0; i < jplen; i++) {
		if (jp[i].jnz == jnz) {
			return jp[i].jz;
		}
	}
	return -1;
}

int get_matching_jnz(struct jump_pair* jp, int jz, int jplen) {
	for (int i = 0; i < jplen; i++) {
		if (jp[i].jz == jz) {
			return jp[i].jnz;
		}
	}
	return -1;
}

int get_matching_y(struct mouse* mice, int len, int x) {
	for (int i = 0; i < len; i++) {
		if (mice[i].x == x) {
			return mice[i].y;
		}
	}
	return -1;
}

int compile_raw_nyan(struct nyan_s nyan, Assembly* assem);

int compile_nyan(struct nyan_s nyan, Assembly* assem) {
	asm_init(assem);

	asm_add(assem, "section .data");
	asm_add(assem, "memory: times 0x100000 db 0");
	asm_add(assem, "section .text");

	// make modules as functions
	for (int i = 0; i < nyan.mice_len; i++) {
		struct mouse m = nyan.mice[i];
		asm_addf(assem, "func_%d:", m.x);
		Assembly* assm_mouse = malloc(sizeof(Assembly));
		asm_init(assm_mouse);
		struct nyan_s nyan_mouse = parse_nyan(m.f);
		compile_raw_nyan(nyan_mouse, assm_mouse);
		asm_add(assem, assm_mouse->data);
		free(assm_mouse);
	}

	asm_add(assem, "global _start");
	asm_add(assem, "_start:");
	asm_add(assem, "mov r8, memory");
	asm_add(assem, "mov rdx, _start");

	compile_raw_nyan(nyan, assem);
	
	asm_add(assem, "mov rax, 60");
	asm_add(assem, "mov rdi, 0");
	asm_add(assem, "syscall");

	return 0;
}

int compile_raw_nyan(struct nyan_s nyan, Assembly* assem) {
	asm_add(assem, "push r8");
	asm_add(assem, "push r9");

	int ptr = 0; // pointer
	int mptr = 0; // module pointer

	struct jump_pair *jump_pairs = malloc(sizeof(struct jump_pair) * nyan.len);
	int jplen = 0;

	set_jump_pair(&nyan, jump_pairs, &jplen);

	for (int i = 0; i < nyan.len; i++) {
		switch (nyan.commands[i]) {
			case POINTER_ADD:
				ptr++;
			break;
			case POINTER_SUB:
				ptr--;
			break;
			case VALUE_ADD:
				asm_addf(assem, "mov r9, r8");
				asm_addf(assem, "add r9, %d", ptr);
				asm_add(assem, "inc byte [r9]");
			break;
			case VALUE_SUB:
				// set r9 to r8 + ptr
				asm_addf(assem, "mov r9, r8");
				asm_addf(assem, "add r9, %d", ptr);
				asm_add(assem, "dec byte [r9]");
			break;
			case DEBUG_PRINT:
			// TODO: print number
			case PRINT:
				asm_add(assem, "mov rax, 1");
				asm_add(assem, "mov rdi, 1");
				asm_add(assem, "mov rsi, r8");
				asm_add(assem, "mov rdx, 1");
				asm_add(assem, "syscall");
			break;
			case JUMP_ZERO: {
				// jump point
				asm_addf(assem, ".jz%d:", i);
				asm_add(assem, "cmp byte [r8], 0");
				// get pair jump point
				int jnz = get_matching_jnz(jump_pairs, i, jplen);
				asm_addf(assem, "je .jnz%d", jnz);
				break;
			}
			case JUMP_NON_ZERO:
				asm_addf(assem, ".jnz%d:", i);
				asm_add(assem, "cmp byte [r8], 0");
				int jz = get_matching_jz(jump_pairs, i, jplen);
				asm_addf(assem, "jne .jz%d", jz);
				break;
			case MODULE_POINTER_ADD:
				mptr++;
			break;
			case MODULE_POINTER_SUB:
				mptr--;
				break;
			case MODULE_RETREIVE:
				// check mptr exists
				if (mptr < 0 || mptr >= nyan.mice_len) {
					printf("Error: module pointer out of bounds\n");
					return -1;
				}
				// call function
				asm_addf(assem, "call func_%d", get_matching_y(nyan.mice, nyan.mice_len, mptr));
				// retrive return value
				asm_add(assem, "mov r9, r8");
				asm_addf(assem, "add r9, %d", ptr);
				asm_add(assem, "mov byte [r9], al");
				break;
			case MODULE_RETURN: {
				asm_add(assem, "mov r9, r8");
				asm_addf(assem, "add r9, %d", ptr);
				asm_add(assem, "mov al, byte [r9]");
				asm_add(assem, "pop r9");
				asm_add(assem, "pop r8");
				asm_add(assem, "ret");
				break;
			}
		}
	}

	return 0;
}

// create a function that returns nyan struct
struct nyan_s parse_nyan(char* filename) {
	wchar_t* code = NULL;
	size_t len = 0;
	wread_file(&code, &len, filename);

	char* mbuffer = NULL;
	size_t msize;
	char* mousename = remove_nyan_ext(filename);
	strcat(mousename, ".mouse");
	read_file(&mbuffer, &msize, mousename);


	int mlen;
	struct mouse* mice = parse_mouse(mbuffer, &mlen);

	struct nyan_s nyan;
	nyan.commands = calloc(len, sizeof(unsigned int*));
	for (int i = 0; i < len; i++) {
		nyan.commands[i] = IGNORE;
	}
	nyan.len = len;

	nyan.mice = mice;

	nyan.mice_len = mlen;



	//    for (int i = 0; i < nyan.mice_len; i++) {
	//        int x;
	//        int y;
	//        memcpy(&x, &nyan.mice[i]->x, sizeof(int));
	//        memcpy(&y, &nyan.mice[i]->y, sizeof(int));
	//        printf("NYMouse %d: %d, %d, %s\n", i, x, y, nyan.mice[i]->f);
	//    }

	for (int i = 0; i < len; i++) {
		switch (code[i]) {
			case L'?':
				nyan.commands[i] = POINTER_ADD;
			break;
			case L'!':
				nyan.commands[i] = POINTER_SUB;
			break;
			case L'냥':
				nyan.commands[i] = VALUE_ADD;
			break;
			case L'냐':
				nyan.commands[i] = VALUE_SUB;
			break;
			case L'뀨':
				nyan.commands[i] = DEBUG_PRINT;
			break;
			case L'.':
				nyan.commands[i] = PRINT;
			break;
			case L'~':
				nyan.commands[i] = JUMP_ZERO;
			break;
			case L'-':
				nyan.commands[i] = JUMP_NON_ZERO;
			break;
			case L'먕':
				nyan.commands[i] = MODULE_POINTER_ADD;
			break;
			case L'먀':
				nyan.commands[i] = MODULE_POINTER_SUB;
			break;
			case L':':
				nyan.commands[i] = MODULE_RETREIVE;
			break;
			case L';':
				nyan.commands[i] = MODULE_RETURN;
			break;
			case L' ':
				nyan.commands[i] = IGNORE;
				// ignore whitespace
			break;
			case L'\n':
				nyan.commands[i] = IGNORE;
				// ignore newlines
			break;
			default:
				wprintf(L"Unknown command: %lc\n", code[i]);
		}
	}

	free(mbuffer);
	free(mousename);
	free(code);

	return nyan;
}
