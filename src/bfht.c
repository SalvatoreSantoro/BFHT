#include "../bfht.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if ALPHA == 125
#define COMPUTE_ALPHA_LIMIT(x) ((x) >> 3)
#elif ALPHA == 250
#define COMPUTE_ALPHA_LIMIT(x) ((x) >> 2)
#elif ALPHA == 375
#define COMPUTE_ALPHA_LIMIT(x) (((x) >> 2) + ((x) >> 3))
#elif ALPHA == 500
#define COMPUTE_ALPHA_LIMIT(x) ((x) >> 1)
#elif ALPHA == 625
#define COMPUTE_ALPHA_LIMIT(x) (((x) >> 1) + ((x) >> 3))
#elif ALPHA == 750
#define COMPUTE_ALPHA_LIMIT(x) (((x) >> 1) + ((x) >> 2))
#elif ALPHA == 875
#define COMPUTE_ALPHA_LIMIT(x) (((x) >> 1) + ((x) >> 2) + ((x) >> 3))
#else
#error "Unsupported ALPHA value"
#endif

#if HT_SIZE_TYPE == PRIME_SIZE
#define MODULO(h, t) (h % t->size)
#elif HT_SIZE_TYPE == POW_2_SIZE
#define MODULO(h, t) (h & t->size_mask)
#endif // HT_SIZE_TYPE == PRIME_SIZE

#define HT_MAX_SIZE HT_INITIAL_SIZE* HT_SIZE_MAX_RESIZE

#if PROBING == LINEAR
#define PROBE(h_k, i, ht) MODULO((h_k + i), ht)
#elif
#include <math.h>
#define PROBE(h_k, i, ht) MODULO((h_k + i + pow(i, 2)), ht)
#endif

uint32_t def_hash(const void* key)
{
    uint32_t hash = 5381;
    const char* p = (char*)key;
    int c;

    while ((c = *p++)) {
        if (isupper(c)) {
            c = c + 32;
        }
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

typedef struct {
    void* key;
    void* data;
    uint32_t hash;
    bool free : 1;
    bool deleted : 1;
} Elem;

struct Bfht {
    del_func key_destroy;
    del_func data_destroy;
    cmp_func key_compare;
    // if size is always a power of 2 it's faster to do modulo using a mask
    size_t size_mask;
    size_t size;
    size_t occupied_limit;
    size_t valid;
    size_t occupied;
    Elem* elems;
};

void* bfht_find(Bfht* bfht, void* key)
{
    uint32_t hash = def_hash(key);
    size_t idx = 0;
    int position;
    Elem* start_elem;

    for (size_t i = 0; i < bfht->size; i++) {
        position = PROBE(hash, idx++, bfht);
        start_elem = &bfht->elems[position];
        if (start_elem->deleted)
            continue;
        if (start_elem->free)
            break;
        if ((hash == start_elem->hash) && (bfht->key_compare(key, start_elem->key)))
            return start_elem;
    }

    return NULL;
}

void bfht_insert(Bfht* bfht, void* key, void* data)
{
    size_t old_size;
    size_t next_size;
    size_t idx;
    Elem* old_elems;
    uint32_t hash;
    // resize
    // cap the size
    if ((bfht->occupied == bfht->occupied_limit) && ((bfht->size << 1) < HT_MAX_SIZE)) {
        old_size = bfht->size;
        bfht->size = bfht->size << 1;
        for (size_t i = 0; i < old_size; i++) {
            idx = MODULO(bfht->elems->hash, bfht);
            // INSERT IN TABLE
        }

        free(old_elems);
        bfht->elems = malloc(bfht->size * sizeof(Elem));
    }

    hash = def_hash(key);
    idx = MODULO(hash, bfht);

    if (bfht->elems[idx].free) {
        bfht->elems[idx].free = false;
        bfht->elems[idx].hash = hash;
        bfht->elems[idx].key = key;
        bfht->elems[idx].data = data;
        return;
    }
}

Bfht* bfht_create(cmp_func key_compare, del_func key_destroy, del_func data_destroy)
{
    Bfht* bfht = malloc(sizeof(Bfht) + (HT_INITIAL_SIZE * sizeof(Elem)));
    if (bfht == NULL)
        return NULL;

    bfht->size = HT_INITIAL_SIZE;
    bfht->size_mask = HT_INITIAL_SIZE - 1;
    bfht->occupied_limit = COMPUTE_ALPHA_LIMIT(HT_INITIAL_SIZE);
    bfht->occupied = 0;
    bfht->valid = 0;
    bfht->key_compare = key_compare;
    bfht->data_destroy = data_destroy;
    bfht->key_destroy = key_destroy;

    for (int i = 0; i < HT_INITIAL_SIZE; i++) {
        bfht->elems[i].key = NULL;
        bfht->elems[i].data = NULL;
        bfht->elems[i].free = true;
        bfht->elems[i].deleted = false;
    }

    return bfht;
}

void bfht_destroy(Bfht* bfht)
{
    free(bfht->elems);
    free(bfht);
}
