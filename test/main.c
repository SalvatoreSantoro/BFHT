#include "../bfht.h"
#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

static char** words;
static char* file;
static size_t file_size;
static size_t words_size = 1024;
static size_t words_read = 0;

void load_words(void)
{
    char** tmp;
    char* token;
    struct stat fs;
    int fd;
    char* file_content = NULL;
    char* delim = "  \n,.!\t:\"()\'-$";

    fd = open("words/1", O_RDONLY);
    fstat(fd, &fs);

    file_size = fs.st_size;

    file = mmap(NULL, fs.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);

    file_content = malloc(fs.st_size + 1);
    memcpy(file_content, file, fs.st_size);
    file_content[fs.st_size] = '\0';

    words = malloc(words_size * (sizeof(char*)));
    token = strtok(file_content, delim);

    while (token != NULL) {
        if (words_read == words_size) {
            words_size *= 2;
            tmp = realloc(words, words_size * sizeof(char*));
            if (tmp == NULL) {
                fprintf(stderr, "WORDS BUFFER OUT OF MEMORY");
                exit(-2);
            }
            words = tmp;
        }
        words[words_read] = token;
        words_read += 1;
        token = strtok(NULL, delim);
    }
}

void destroy_words(void)
{
    munmap(file, file_size);
    free(words);
}

bool key_cmp(const void* str1, const void* str2)
{
    return !strcmp(str1, str2);
}

int main(void)
{
    Bfht* bfht = bfht_create(key_cmp, NULL, NULL);
    load_words();
    words_read = 1000;
    printf("%ld words loaded\n", words_read);

    int* word_count = malloc(words_read);
    int* word_count_mock = malloc(words_read);
    memset(word_count, 1, words_read);
    int* p = NULL;

    for (size_t i = 0; i < words_read; i++)
        if (bfht_insert(bfht, words[i], &word_count[i]) == BFHT_UPDATE){}

    for (size_t i = 0; i < words_read; i++){
        p = bfht_find(bfht, words[i]);
        if (p != NULL)
            words[i]

    }

        for (size_t i = 0; i < words_read; i++)
            bfht_delete(bfht, words[i]);

    free(word_count);
    destroy_words();
    bfht_destroy(bfht);
    return 0;
}
