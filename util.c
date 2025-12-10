#include <stddef.h>
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct*) userp;

    // Ensure capacity
    if (mem->size + realsize + 1 > mem->capacity) {
        size_t newcap = mem->capacity * 2;
        while (newcap < mem->size + realsize + 1)
            newcap *= 2;

        char *newmem = realloc(mem->memory, newcap);
        if (newmem == NULL) {
            fprintf(stderr, "Not enough memory for write callback\n");
            return 0; // CURL will abort transfer
        }

        mem->memory = newmem;
        mem->capacity = newcap;
    }

    // Append new data
    memcpy(mem->memory + mem->size, contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = '\0';

    return realsize;
}



char *read_entire_file(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *buf = malloc(size + 1);
    if (!buf) return NULL;

    size_t n = fread(buf, 1, size, fp);
    buf[n] = '\0';
    return buf;
}

int is_file_empty(FILE *fp) {
    fseek(fp, 0, SEEK_END);      // go to end
    long size = ftell(fp);       // get position (file size)
    rewind(fp);                  // return to start
    return size == 0;
}
