#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <wctype.h>

#define EXIT_FAILURE -1
#define EXIT_SUCCESS 0
#define MAX_STR_LEN 1024

char **split_lines(char *str, int *count) {
    const char* delim = "\n";
    char *token = strtok(str, delim);
    char **lines = malloc(sizeof(char*) * 1024);
    int i = 0;
    while (token != NULL) {
        lines[i] = token;
        token = strtok(NULL, delim);
        i++;
    }
    *count = i;
    return lines;
}

char* rtrim(char* s) {
    char t[MAX_STR_LEN];
    char *end;

    strcpy(t, s); // 이것은 Visual C 2005용
    end = t + strlen(t) - 1;
    while (end != t && isspace(*end))
        end--;
    *(end + 1) = '\0';
    s = t;

    return s;
}

// 문자열 좌측 공백문자 삭제 함수
char* ltrim(char *s) {
    char* begin;
    begin = s;

    while (*begin != '\0') {
        if (isspace(*begin))
            begin++;
        else {
            s = begin;
            break;
        }
    }

    return s;
}

// 문자열 앞뒤 공백 모두 삭제 함수
char* trim(char *s) {
    return rtrim(ltrim(s));
}

void wread_file(wchar_t** file, size_t* set_size, char* fname) {
    FILE *fp;
    wchar_t *buffer = malloc(sizeof(wchar_t) * MAX_STR_LEN);
    size_t size = 0;
    int n;

    fp = fopen(fname, "r,ccs=UTF-8");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }


    while ((n = fgetwc(fp)) != WEOF) {
        if (buffer == NULL) {
            perror("Error allocating memory");
            fclose(fp);
            exit(EXIT_FAILURE);
        }
        buffer[size++] = n;
    }
    buffer[size] = L'\0';

    *file = buffer;
    *set_size = size;

    fclose(fp);
}

void read_file(char** file, size_t* set_size, char* fname) {
    FILE *fp;
    char *buffer = calloc(MAX_STR_LEN, sizeof(char));
    size_t size = 0;
    int n;

    fp = fopen(fname, "r");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    while ((n = fgetc(fp)) != EOF) {
        buffer = realloc(buffer, sizeof(char) * (size + 1));
        if (buffer == NULL) {
            perror("Error allocating memory");
            fclose(fp);
            exit(EXIT_FAILURE);
        }
        buffer[size++] = n;
    }

    if (buffer == NULL) {
        // Empty file
        buffer = (char*)malloc(sizeof(char));
        buffer[0] = '\0';
        fclose(fp);
        *file = buffer;
        *set_size = size;
        return;
    }

    buffer[size] = '\0';

    *file = buffer;
    *set_size = size;

    fclose(fp);
}

int endsWith( char *string, char *check )
{
    string = strrchr(string, '.');

    if( string != NULL )
        return( strcmp(string, check) );

    return( -1 );
}

char *remove_nyan_ext (char* myStr) {
    char* newStr = malloc((sizeof(char) + 1) * strlen(myStr));
    int endsWithNyan = endsWith(myStr, ".nyan");
    if (endsWithNyan == 0) {
        strncpy(newStr, myStr, strlen(myStr) - 5);
        newStr[strlen(myStr) - 5] = '\0';
        return newStr;
    } else {
        return myStr;
    }
}

char* substr(char* str, int start, int end) {
    char* sub = malloc(sizeof(char) * (end - start + 1));
    int j = 0;
    for (int i = start; i < end; i++) {
        sub[j] = str[i];
        j++;
    }
    sub[j] = '\0';
    return sub;
}