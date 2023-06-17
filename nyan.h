//
// Created by Minco on 2023-04-01.
//

#ifndef NYANLANG_NYAN_H
#define NYANLANG_NYAN_H

#include <wchar.h>
#include "asm.h"

struct nyan_s {
    unsigned short* commands;
    size_t len;
    struct mouse* mice;
    size_t mice_len;
};

enum nyan_command {
    POINTER_ADD = 0,
    POINTER_SUB = 1,
    VALUE_ADD = 2,
    VALUE_SUB = 3,
    DEBUG_PRINT = 4,
    PRINT = 5,
    JUMP_ZERO = 6,
    JUMP_NON_ZERO = 7,
    MODULE_POINTER_ADD = 8,
    MODULE_POINTER_SUB = 9,
    MODULE_RETURN = 10,
    MODULE_RETREIVE = 11,

    IGNORE = 12
};

int compile_nyan(struct nyan_s nyan, Assembly* assem);
struct nyan_s parse_nyan(char* filename);

#endif //NYANLANG_NYAN_H
