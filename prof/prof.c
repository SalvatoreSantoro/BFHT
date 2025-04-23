#include "../bfht.h"
#include "../common/words.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static char *profiling_path = "words/profiling";
static char *output_path;

#define NUM_OF_FINDS 1000
#define INIT_ITER    2
#define STRIDE_ITER  2

void profiling(void) {
    Bfht *bfht;
    FILE *file;
    struct timespec start, end;
    double elapsed;
    double mean_elapsed;
    int mock;
    int *rng_words;

    srand(time(NULL));

    file = fopen(output_path, "w");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "Number of Words,Time\n");

    // generate n random index for keys
    rng_words = malloc(sizeof(int) * NUM_OF_FINDS);

    // test with increasing number of keys
    for (size_t n = INIT_ITER; n <= words_vec_len; n *= STRIDE_ITER) {
        bfht = bfht_create(key_cmp, NULL, NULL);

        for (size_t i = 0; i < n; i++) {
            bfht_insert(bfht, words_vec[i], &mock);
        }
        for (size_t k = 0; k < NUM_OF_FINDS; k++) {
            rng_words[k] = rand() % n;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);
        for (size_t i = 0; i < NUM_OF_FINDS; i++) {
            bfht_find(bfht, words_vec[rng_words[i]]);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);

        elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        mean_elapsed = elapsed / NUM_OF_FINDS;
        fprintf(file, "%ld,%.20f\n", n, mean_elapsed);
        printf("Find took %.20f seconds to execute %ld lookups\n", mean_elapsed, n);
        bfht_destroy(bfht);
    }

    free(rng_words);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "No OUTPUT file specified.\n");
        exit(-1);
    }
    output_path = argv[1];
    load_words(profiling_path);
    printf("Loading %ld words to do profiling.\n", words_vec_len);
    profiling();
    destroy_words();
}
