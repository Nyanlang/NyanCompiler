#include <stdio.h>
#include <malloc.h>
#include <string.h>

char** parse_arg(char* arg, int* len) {
    char** args = malloc(sizeof(char*) * 128);
    char* token = strtok(arg, " ");
    int i = 0;
    while (token != NULL) {
        args[i] = malloc(sizeof(char) * 128);
        strcpy(args[i], token);
        token = strtok(NULL, " ");
        i++;
    }
    *len = i;
    return args;
}