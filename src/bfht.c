#include "../bfht.h"
#include "hash.h"
#include "macros.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    void *key;
    void *data;
    uint32_t hash;
    bool free : 1;
    bool deleted : 1;
} Elem;

// making the total size of the struct smaller could be useful (at the moment is
// bigger than CACHE block size)
struct Bfht {
    del_func key_destroy;
    del_func data_destroy;
    cmp_func key_compare;
    size_t size;
    size_t occupied_upper_limit;
    size_t occupied_lower_limit;
    size_t valid;
    size_t occupied;
    Elem *table;
    // according to type size settings we define
    // variables useful for resizing of ht
#if HT_SIZE_TYPE == PRIMES
    int primes_pos;
#else
    size_t size_mask;
#endif
};

static int _bfht_resize(Bfht *bfht, size_t new_size) {
    size_t old_size;
    Elem *temp_elems;
    int position;
    size_t count;
    size_t idx;

    // allocate
    temp_elems = malloc((new_size) * (sizeof(Elem)));
    if (temp_elems == NULL)
        return BFHT_EOOM;

    // Could be fun to unroll this loop

    for (size_t i = 0; i < new_size; i++) {
        temp_elems[i].free = true;
        temp_elems[i].deleted = false;
    }

    old_size = bfht->size;
    bfht->size = new_size;

#if HT_SIZE_TYPE == PRIMES
    if (new_size > old_size)
        bfht->primes_pos += 1;
    else
        bfht->primes_pos -= 1;
#else
    bfht->size_mask = new_size - 1;
#endif

    bfht->occupied_upper_limit = COMPUTE_ALPHA_UP_LIM(new_size);
    bfht->occupied_lower_limit = COMPUTE_ALPHA_LOW_LIM(new_size);
    // reset "occupied" to valid, because we're effectively discarding "deleted"
    // locations
    bfht->occupied = bfht->valid;

    count = 0;

    for (size_t i = 0; i < old_size; i++) {
        // stop cicle if moved all the "valid"(effectively occupied) elems
        if (count == bfht->valid)
            break;
        if (bfht->table[i].free || bfht->table[i].deleted)
            continue;

        idx = 0;

        // this cycle assumes that the new size is already in bfht but
        // the element must be transfered to temp_elems,
        // the cycle always end because we're assuming that we need to move
        // elements in a bigger array, so we will eventually find
        // some free spaces (break condition)
        while (1) {
            position = PROBE(bfht->table[i].hash, idx, bfht);
            idx += 1;
            // move the old elem in the new array
            if (temp_elems[position].free) {
                STORE_ELEM(temp_elems[position], bfht->table[i].hash, bfht->table[i].key, bfht->table[i].data);
                count += 1;
                break;
            }
        }
    }

    // the first time when allocating bfht->table will be NULL
    if (bfht->table != NULL)
        free(bfht->table);
    bfht->table = temp_elems;

    return BFHT_OK;
}

static void *_bfht_find_elem(Bfht *bfht, void *key) {
    Elem *elem;
    int position;
    size_t idx = 0;
    uint32_t hash = def_hash(key);

    for (size_t i = 0; i < bfht->size; i++) {
        position = PROBE(hash, idx, bfht);
        idx += 1;
        elem = &bfht->table[position];
        if (elem->free)
            break;
        if (elem->deleted)
            continue;
        if ((hash == elem->hash) && (bfht->key_compare(key, elem->key)))
            return elem;
    }

    return NULL;
}

void *bfht_find(Bfht *bfht, void *key) {
    Elem *elem = _bfht_find_elem(bfht, key);
    if (elem != NULL)
        return elem->data;
    return NULL;
}

int bfht_remove(Bfht *bfht, void *key) {
    size_t new_size;

    /* static int count = 0; */
    /* count += 1; */
    /* printf("DEL COUNT: %d\n", count); */

    Elem *elem = _bfht_find_elem(bfht, key);
    if (elem == NULL) {
        return BFHT_OK;
    }

    if (bfht->data_destroy)
        bfht->data_destroy(elem->data);

    if (bfht->key_destroy)
        bfht->key_destroy(elem->key);

    elem->deleted = true,

    bfht->valid -= 1;

    // resize
    // cap the size
    if ((bfht->valid == bfht->occupied_lower_limit) && (SHRINKED_SIZE(bfht) >= HT_MIN_SIZE)) {
        new_size = SHRINKED_SIZE(bfht);
        int ret = _bfht_resize(bfht, new_size);
        // printf("DEL RESIZE: %ld\n", new_size);
        if (ret != BFHT_OK)
            return BFHT_DEL_WRN;
    }

    return BFHT_OK;
}

// Returns:
// BFHT_OK if everything was ok
// BFHT_UPDATE the inserted element was already inside, so the table updated the
// data (the old pointed data is destroyed) BFHT_INS_WRN if successfully
// inserted element but couldn't expand the hash table due to an Out of memory
// retor BFHT_EOOM if couldn't insert the element (hash table is full and can't
// expand further)

int bfht_insert(Bfht *bfht, void *key, void *data) {
    size_t new_size;
    uint32_t hash;
    uint32_t position;
    int ret = BFHT_OK;
    size_t idx = 0;

    /* static int count = 0; */
    /* count += 1; */
    /* printf("INS COUNT: %d\n", count); */

    assert(data != NULL);
    assert(key != NULL);

    // resize
    // cap the size
    if ((bfht->occupied == bfht->occupied_upper_limit) && (EXPANDED_SIZE(bfht) <= HT_MAX_SIZE)) {
        new_size = EXPANDED_SIZE(bfht) > HT_MIN_SIZE ? EXPANDED_SIZE(bfht) : HT_MIN_SIZE;
        // printf("INS RESIZE: NEW %ld, OLD %ld, MAX %ld\n", new_size, bfht->size, HT_MAX_SIZE);
        int ret = _bfht_resize(bfht, new_size);
        if (ret != BFHT_OK)
            ret = BFHT_INS_WRN;
    }

    hash = def_hash(key);

    for (size_t i = 0; i < bfht->size; i++) {
        position = PROBE(hash, idx, bfht);
        idx += 1;
        if (bfht->table[position].free || bfht->table[position].deleted) {
            STORE_ELEM(bfht->table[position], hash, key, data);
            bfht->valid += 1;
            bfht->occupied += 1;
            return ret;
        }

        // if already inserted just overwrite with new data value
        if ((hash == bfht->table[position].hash) && (bfht->key_compare(key, bfht->table[position].key))) {
            if (bfht->data_destroy)
                bfht->data_destroy(bfht->table[position].data);
            bfht->table[position].data = data;
            return BFHT_UPDATE;
        }
    }
    return BFHT_EOOM;
}

Bfht *bfht_create(cmp_func key_compare, del_func key_destroy, del_func data_destroy) {
    Bfht *bfht = malloc(sizeof(Bfht));
    if (bfht == NULL)
        return NULL;

    // the table starts empty and on the first insert there is the real
    // allocation
    bfht->size = 0;
#if HT_SIZE_TYPE == PRIMES
    bfht->primes_pos = 0;
#else
    bfht->size_mask = 0;
#endif
    bfht->occupied_upper_limit = 0;
    bfht->occupied_lower_limit = 0;
    bfht->occupied = 0;
    bfht->valid = 0;
    bfht->key_compare = key_compare;
    bfht->data_destroy = data_destroy;
    bfht->key_destroy = key_destroy;
    bfht->table = NULL;

    return bfht;
}

void bfht_destroy(Bfht *bfht) {
    if (bfht->table != NULL) {
        free(bfht->table);
    }
    free(bfht);
}
