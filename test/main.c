#include "../bfht.h"
#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static char **words_vec;
static char *file_content;
static size_t words_vec_len = 0;

void load_words(void) {
    char **tmp;
    char *token;
    char *file;
    struct stat fs;
    int fd;
    size_t words_vec_size = 256;
    size_t file_size;

    char *delim = "  \n,.!\t:\"()\'-$";

    fd = open("words/dataset", O_RDONLY);
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
                exit(-2);
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

bool key_cmp(const void *str1, const void *str2) { return !strcmp(str1, str2); }

int main(void) {
    int *p;
    int *values;
    int *mock_counts;
    Bfht *bfht;

    load_words();
    printf("Loading %ld words.\n", words_vec_len);

    values = malloc(sizeof(int) * words_vec_len);
    mock_counts = malloc(sizeof(int) * words_vec_len);

    if (values == NULL) {
        printf("OOM\n");
        return 0;
    }

    if (mock_counts == NULL) {
        printf("OOM\n");
        return 0;
    }

    for (size_t i = 0; i < words_vec_len; i++) {
        mock_counts[i] = 0;
        for (size_t j = 0; j < words_vec_len; j++) {
            if (!strcmp(words_vec[i], words_vec[j]))
                mock_counts[i] += 1;
        }
    }

    for (size_t i = 0; i < words_vec_len; i++)
        values[i] = 0;

    bfht = bfht_create(key_cmp, NULL, NULL);

    // insert all single occurrences (the data location for repeated occurrences
    // will be overwritten)
    for (size_t i = 0; i < words_vec_len; i++)
        bfht_insert(bfht, words_vec[i], &values[i]);

    // count the occurrences of the words
    for (size_t i = 0; i < words_vec_len; i++) {
        p = bfht_find(bfht, words_vec[i]);
        if (p != NULL)
            *p += 1;
    }

    // compare with brute-force approach to verify correctness
    for (size_t i = 0; i < words_vec_len; i++) {
        p = bfht_find(bfht, words_vec[i]);
        assert((*p) == mock_counts[i]);
    }

    for (size_t i = 0; i < words_vec_len; i++)
        bfht_remove(bfht, words_vec[i]);

    printf("Hash Table test passed!\n");

    free(mock_counts);
    free(values);
    destroy_words();
    bfht_destroy(bfht);
}
