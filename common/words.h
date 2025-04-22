#ifndef _WORDS_LOADER_H
#define _WORDS_LOADER_H

#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

static char **words_vec;
static char *file_content;
static size_t words_vec_len = 0;

void load_words(const char *file_path) {
    char **tmp;
    char *token;
    char *file;
    struct stat fs;
    int fd;
    size_t words_vec_size = 256;
    size_t file_size;

    char *delim = "  \n,.!\t:\"()\'-$";

    fd = open(file_path, O_RDONLY);
    if(fd == -1){
        perror("Can't find the file: ");
        exit(-1);
    }
    fstat(fd, &fs);
    file_size = fs.st_size;

    file = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    file_content = malloc(file_size + 1);
    memcpy(file_content, file, file_size);
    file_content[file_size] = '\0';

    words_vec = malloc(words_vec_size * (sizeof(char *)));
    token = strtok(file_content, delim);

    while (token != NULL) {
        if (words_vec_len == words_vec_size) {
            words_vec_size *= 2;
            tmp = realloc(words_vec, words_vec_size * sizeof(char *));
            if (tmp == NULL) {
                fprintf(stderr, "WORDS BUFFER OUT OF MEMORY");
                exit(-1);
            }
            words_vec = tmp;
        }
        words_vec[words_vec_len] = token;
        words_vec_len += 1;
        token = strtok(NULL, delim);
    }
    munmap(file, file_size);
}

void destroy_words(void) {
    free(file_content);
    free(words_vec);
}

bool key_cmp(const void *str1, const void *str2) {
    return !strcmp(str1, str2);
}

#endif
