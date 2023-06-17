#include "mouse.h"
#include "util.h"

struct mouse* parse_mouse(char* data, int* len) {
    int num_lines;
    char** lines = split_lines(data, &num_lines);
    struct mouse* mice = malloc(sizeof(struct mouse*) * num_lines);

    for (int i = 0; i < num_lines; i++) {
        char* line = lines[i];
        char* token = strtok(line, ":");
        char** tokens = malloc(sizeof(char*) * 2);
        tokens[0] = malloc(sizeof(char) * 128);
        tokens[1] = malloc(sizeof(char) * 512);
        struct mouse mouse;

        int j = 0;
        while (token != NULL) {
            strcpy(tokens[j], token);
            token = strtok(NULL, " ");
            j++;
        }

        size_t tlen = strlen(trim(tokens[1])) +1;
        mouse.f = calloc(tlen, sizeof(char));
        memcpy(mouse.f, trim(tokens[1]), (tlen) * sizeof(char));

        char* x = strtok(tokens[0], "->");
        char* y = strtok(NULL, "->");
        mouse.x = atoi(x);
        mouse.y = atoi(y);

        mice[i] = mouse;
    }

    *len = num_lines;

    return mice;
}