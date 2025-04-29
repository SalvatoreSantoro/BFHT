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

/******************* HASH ******************/

// POSSIBLE VALUES: [FNV1a,MURMUR, DJB2]
#ifndef HASH_FUN
#define HASH_FUN DJB2 
#endif

/****************** SIZE *******************/

// if undefined as default power of 2 sizes are used

// POSSIBLE VALUES: [PRIMES, TWOPOWERS]
#ifndef HT_SIZE_TYPE
    #define HT_SIZE_TYPE TWOPOWERS
#endif

// NEED TO BE A POWER OF 2 (IGNORED IF COMPILING WITH "PRIME_SIZE")
#ifndef HT_INITIAL_SIZE
    #define HT_INITIAL_SIZE 256
#endif

// RANGE 0-23
#ifndef HT_SIZE_MAX_GROWINGS
    #define HT_SIZE_MAX_GROWINGS 20
#endif

/****************** PROBING *****************/

// POSSIBLE VALUES: [LINEAR, QUADRATIC]

#ifndef PROBING
    #define PROBING LINEAR
#endif

/***************** LOAD FACTOR **************/

// POSSIBLE VALUES: [125, 375, 500, 625, 75, 875] (they are intended as 0.125, 0.375 and so on)

#ifndef ALPHA
    #define ALPHA 500
#endif

// CURRENTLY UNSUPPORTED
// IT HAS NO EFFECT
#define BETA 2

typedef struct Bfht Bfht;
typedef bool (*cmp_func)(const void *, const void *);
typedef void (*del_func)(void *);

// Returns:
// BFHT_OK if everything was ok
// BFHT_UPDATE the inserted element was already inside, so the table updated the
// data (the old pointed data is destroyed)
// BFHT_INS_WRN if successfully inserted the element but couldn't expand the hash table (memory is saturating)
// BFHT_EOOM if couldn't insert the element (hash table is full and can't be expanded further)
int bfht_insert(Bfht *bfht, void *key, void *data);

int bfht_remove(Bfht *bfht, void *key);

void *bfht_find(Bfht *bfht, void *key);

Bfht *bfht_create(cmp_func key_compare, del_func key_destroy, del_func data_destroy);

void bfht_destroy(Bfht *bfht);

#endif
