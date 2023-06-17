#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mouse {
    int x;
    int y;
    char *f;
};

struct mouse* parse_mouse(char* data, int* len);
