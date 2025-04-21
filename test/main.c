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
static char *profiling_path = "words/profiling";
static char *test_path = "words/test";
static char *output_file = "words/out_prof";

// Function to check if file exists
int file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1; // File exists
    }
    return 0; // File doesn't exist
}

// Function to append measurement
void append_measurement(int word_count, double time_taken) {
    // Check if file already exists
    int exists = file_exists(output_file);

    // Open file in append mode
    FILE *file = fopen(output_file, "a");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Write header if the file didn't exist before
    if (!exists) {
        fprintf(file, "Number of Words,Time\n");
    }

    // Append the measurement
    fprintf(file, "%d,%.8f\n", word_count, time_taken);

    fclose(file);
}

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

void profiling(void) {
    Bfht *bfht;
    struct timespec start, end;
    double elapsed;
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
        free(rng_words);
        append_measurement(n, elapsed);
        printf("Find took %f seconds to execute %ld lookups\n", elapsed, n);
    }

    bfht_destroy(bfht);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("No args specified\n");
        return -1;
    }

    if (strcmp(argv[1], "-t") == 0) {
        load_words(test_path);
        printf("Loading %ld words to test correctness.\n", words_vec_len);
        test_correctness();
    }

    if (strcmp(argv[1], "-p") == 0) {
        load_words(profiling_path);
        printf("Loading %ld words for profiling.\n", words_vec_len);
        profiling();
    }

    destroy_words();
}
