#ifndef _BFHT_H
#define _BFHT_H

#include <stdbool.h>

enum {
    BFHT_OK = 0,
    BFHT_EOOM,
    BFHT_INS_WRN,
    BFHT_UPDATE,
    BFHT_DEL_WRN,
};

/**************** TUNABLES *****************/

/****************** SIZE *******************/

// if undefined as default power of 2 sizes are used
#define PRIME_SIZE 1

// NEED TO BE A POWER OF 2 (IGNORED IF COMPILING WITH "PRIME_SIZE")
#define HT_INITIAL_SIZE 256

// RANGE 0-23
#define HT_SIZE_MAX_GROWINGS 23

/****************** PROBING *****************/

// POSSIBLE VALUES: [LINEAR, QUADRATIC]

#define PROBING LINEAR
/***************** LOAD FACTOR **************/

// POSSIBLE VALUES: [125, 375, 500, 625, 75, 875] (they are intended as 0.125, 0.375 and so on)

#define ALPHA 500


// CURRENTLY UNSUPPORTED
// IT HAS NO EFFECT
#define BETA 2

typedef struct Bfht Bfht;
typedef bool (*cmp_func)(const void *, const void *);
typedef void (*del_func)(void *);

int bfht_insert(Bfht *bfht, void *key, void *data);

int bfht_remove(Bfht *bfht, void *key);

void *bfht_find(Bfht *bfht, void *key);

Bfht *bfht_create(cmp_func key_compare, del_func key_destroy, del_func data_destroy);

void bfht_destroy(Bfht *bfht);

#endif
