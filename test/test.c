#include "../bfht.h"
#include "../common/words.h"
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

static char *test_path = "words/test";

void test_correctness(void) {
    int *p;
    int *values;
    int *mock_counts;
    Bfht *bfht;

    values = malloc(sizeof(int) * words_vec_len);
    mock_counts = malloc(sizeof(int) * words_vec_len);

    if (values == NULL) {
        printf("OOM\n");
        exit(-1);
    }

    if (mock_counts == NULL) {
        printf("OOM\n");
        exit(-1);
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

    bfht_destroy(bfht);
}


int main(void) {
    load_words(test_path);
    printf("Loading %ld words to test correctness.\n", words_vec_len);
    test_correctness();
    destroy_words();
}
