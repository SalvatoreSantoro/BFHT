#include "../bfht.h"
#include "../common/words.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static char *profiling_path = "words/profiling";
static char *output_path;

// Function to check if file exists
int file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

// Function to append measurement
void append_measurement(int word_count, double time_taken) {
    int exists = file_exists(output_path);

    FILE *file = fopen(output_path, "a");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Write header if the file didn't exist before
    if (!exists) {
        fprintf(file, "Number of Words,Time\n");
    }

    fprintf(file, "%d,%.20f\n", word_count, time_taken);

    fclose(file);
}

void profiling(void) {
    Bfht *bfht;
    struct timespec start, end;
    double elapsed;
    double mean_elapsed;
    int mock;
    int *rng_words;

    srand(time(NULL));

    bfht = bfht_create(key_cmp, NULL, NULL);

    for (size_t i = 0; i < words_vec_len; i++) {
        bfht_insert(bfht, words_vec[i], &mock);
    }

    // test with increasing number of keys
    for (size_t n = 1000; n < words_vec_len; n += 1000) {
        // generate n random index for keys
        rng_words = malloc(sizeof(int) * n);
        for (size_t k = 0; k < n; k++) {
            rng_words[k] = rand() % words_vec_len;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);
        for (size_t i = 0; i < n; i++) {
            bfht_find(bfht, words_vec[rng_words[i]]);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);

        elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        mean_elapsed = elapsed / n;
        free(rng_words);
        append_measurement(n, mean_elapsed);
        printf("Find took %.20f seconds to execute %ld lookups\n", mean_elapsed, n);
    }

    bfht_destroy(bfht);
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
