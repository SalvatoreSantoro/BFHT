#ifndef _BFHT_H
#define _BFHT_H

#include <stdbool.h>

/**************** TUNABLES *****************/

/****************** SIZE *******************/

#define POW_2_SIZE 1
#define PRIME_SIZE 0

#define HT_SIZE_TYPE POW_2_SIZE

#define HT_INITIAL_SIZE 256
// HT_MAX_SIZE = HT_INITIAL_SIZE * HT_SIZE_MAX_RESIZE
#define HT_SIZE_MAX_RESIZE 10

/****************** PROBING *****************/

#define LINEAR 1
#define QUADRATIC 0

#define PROBING LINEAR

/***************** LOAD FACTOR **************/

// use subpowers of 2 for faster computations when multiplying or dividing (shifts)

// #define ALPHA 125   // 0.125 * 1000
// #define ALPHA 250   // 0.25
// #define ALPHA 375   // 0.375
#define ALPHA 500 // 0.5
// #define ALPHA 625   // 0.625
// #define ALPHA 750   // 0.75
// #define ALPHA 875   // 0.875

/**************** RESIZE FACTOR *************/

#define BETA 2

typedef struct Bfht Bfht;
typedef bool (*cmp_func)(const void*, const void*);
typedef void (*del_func)(void*);

#endif
