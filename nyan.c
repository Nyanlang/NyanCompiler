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

int compile_raw_nyan(struct nyan_s nyan, Assembly* assem, char* name);

int compile_nyan(struct nyan_s nyan, Assembly* assem) {
	asm_init(assem);

	asm_add(assem, "section .data");
	asm_add(assem, "memory: times 0x100000 db 0");
	asm_add(assem, "modules: times 0x100000 db 0");
	asm_add(assem, "section .text");

	// make modules as functions
	for (int i = 0; i < nyan.mice_len; i++) {
		struct mouse m = nyan.mice[i];
		asm_addf(assem, "func_%d:", m.x);
		asm_addf(assem, "add r8, %d", 0x1000 * (m.x + 1));
		Assembly* assm_mouse = malloc(sizeof(Assembly));
		asm_init(assm_mouse);
		struct nyan_s nyan_mouse = parse_nyan(m.f);
		char* funcname = malloc(sizeof(char) * 10);
		sprintf(funcname, "func_%d", m.x);
		compile_raw_nyan(nyan_mouse, assm_mouse, funcname);
		asm_add(assem, assm_mouse->data);
		asm_addf(assem, "sub r8, %d", 0x1000 * m.x);
		free(assm_mouse);
	}

	asm_add(assem, "global _start");
	asm_add(assem, "_start:");
	asm_add(assem, "mov r8, memory");
	asm_add(assem, "mov rdx, _start");
	for (int i = 0; i < nyan.mice_len; i++) {
		struct mouse m = nyan.mice[i];
		asm_addf(assem, "mov qword [modules+%d], func_%d", m.x, m.x);
	}

	compile_raw_nyan(nyan, assem, "main");
	
	asm_add(assem, "mov rax, 60");
	asm_add(assem, "mov rdi, 0");
	asm_add(assem, "syscall");

	return 0;
}

int compile_raw_nyan(struct nyan_s nyan, Assembly* assem, char* name) {
	asm_add(assem, "push r8");
	asm_add(assem, "push r9");
	asm_add(assem, "mov r9, 0");

	struct jump_pair *jump_pairs = malloc(sizeof(struct jump_pair) * nyan.len);
	int jplen = 0;

	set_jump_pair(&nyan, jump_pairs, &jplen);

	for (int i = 0; i < nyan.len; i++) {
		switch (nyan.commands[i]) {
			case POINTER_ADD:
				asm_add(assem, "inc r8");
			break;
			case POINTER_SUB:
				asm_add(assem, "dec r8");
			break;
			case VALUE_ADD:
				asm_add(assem, "inc qword [r8]");
			break;
			case VALUE_SUB:
				asm_add(assem, "dec qword [r8]");
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
				asm_addf(assem, ".jz%s%d:", name, i);
				asm_add(assem, "cmp qword [r8], 0");
				// get pair jump point
				int jnz = get_matching_jnz(jump_pairs, i, jplen);
				asm_addf(assem, "je .jnz%s%d", name, jnz);
				break;
			}
			case JUMP_NON_ZERO:
				asm_addf(assem, ".jnz%s%d:", name, i);
				asm_add(assem, "cmp byte qword [r8], 0");
				int jz = get_matching_jz(jump_pairs, i, jplen);
				asm_addf(assem, "jne .jz%s%d", name, jz);
				break;
			case MODULE_POINTER_ADD:
				asm_add(assem, "inc r9");
			break;
			case MODULE_POINTER_SUB:
				asm_add(assem, "dec r9");
				break;
			case MODULE_RETREIVE:
				// find module in modules and call it
				asm_add(assem, "mov rbx, modules");
				asm_add(assem, "mov rax, qword [rbx+r9]");
				asm_add(assem, "call rax");
				// mov data in rax to data in r8
				asm_add(assem, "mov qword [r8], rax");
				// mov rax to r8
				break;
			case MODULE_RETURN: {
				// mov data in ptr to rax
				asm_add(assem, "mov rax, qword [r8]");
				// return
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
