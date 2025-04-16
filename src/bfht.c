#include "../bfht.h"
#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if ALPHA == 125
#define COMPUTE_ALPHA_UP_LIM(x) ((x) >> 3)
#define COMPUTE_ALPHA_LOW_LIM(x) ((x) >> 5)
#elif ALPHA == 250
#define COMPUTE_ALPHA_UP_LIM(x) ((x) >> 2)
#define COMPUTE_ALPHA_LOW_LIM(x) ((x) >> 4)
#elif ALPHA == 375
#define COMPUTE_ALPHA_UP_LIM(x) (((x) >> 2) + ((x) >> 3))
#define COMPUTE_ALPHA_LOW_LIM(x) ((x) >> 4) + ((x) >> 5)
#elif ALPHA == 500
#define COMPUTE_ALPHA_UP_LIM(x) ((x) >> 1)
#define COMPUTE_ALPHA_LOW_LIM(x) ((x) >> 3)
#elif ALPHA == 625
#define COMPUTE_ALPHA_UP_LIM(x) (((x) >> 1) + ((x) >> 3))
#define COMPUTE_ALPHA_LOW_LIM(x) (((x) >> 3) + ((x) >> 5))
#elif ALPHA == 750
#define COMPUTE_ALPHA_UP_LIM(x) (((x) >> 1) + ((x) >> 2))
#define COMPUTE_ALPHA_LOW_LIM(x) (((x) >> 3) + ((x) >> 4))
#elif ALPHA == 875
#define COMPUTE_ALPHA_UP_LIM(x) (((x) >> 1) + ((x) >> 2) + ((x) >> 3))
#define COMPUTE_ALPHA_LOW_LIM(x) (((x) >> 3) + ((x) >> 5) + ((x) >> 5))
#else
#retor "Unsupported ALPHA value"
#endif

#if HT_SIZE_TYPE == PRIME_SIZE
#define MODULO(h, t) (h % t->size)
#define EXPANDED_SIZE(h)
#define SHRINKED_SIZE(h)
#elif HT_SIZE_TYPE == POW_2_SIZE
#define MODULO(h, t) (h & t->size_mask)
#define EXPANDED_SIZE(h) (h->size << 1)
#define SHRINKED_SIZE(h) (h->size >> 1)
#endif // HT_SIZE_TYPE == PRIME_SIZE

#define HT_MAX_SIZE (HT_INITIAL_SIZE * HT_SIZE_MAX_RESIZE)

#if PROBING == LINEAR
#define PROBE(h_k, i, ht) MODULO((h_k + i), ht)
#elif
#include <math.h>
#define PROBE(h_k, i, ht) MODULO((h_k + i + pow(i, 2)), ht)
#endif

#define STORE_ELEM(e, h, k, d) \
    do {                       \
        e.free = false;        \
        e.deleted = false;     \
        e.hash = h;            \
        e.key = k;             \
        e.data = d;            \
    } while (0)

uint32_t def_hash(const void* key)
{
    assert(key != NULL);

    int c;
    uint32_t hash = 5381;
    const char* p = (char*)key;

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

// making the total size of the struct smaller could be useful (at the moment is bigger than CACHE block size)
struct Bfht {
    del_func key_destroy;
    del_func data_destroy;
    cmp_func key_compare;
    // if size is always a power of 2 it's faster to do modulo using a mask
    size_t size_mask;
    size_t size;
    size_t occupied_upper_limit;
    size_t occupied_lower_limit;
    size_t valid;
    size_t occupied;
    Elem* table;
};

static int _bfht_resize(Bfht* bfht, size_t new_size)
{
    size_t old_size;
    Elem* temp_elems;
    int position;
    size_t count;
    size_t idx;

    // allocate
    temp_elems = malloc((new_size) * (sizeof(Elem)));
    if (temp_elems == NULL)
        return BFHT_EOOM;

    for (size_t i = 0; i < new_size; i++) {
        temp_elems[i].free = true;
        temp_elems[i].deleted = false;
    }

    old_size = bfht->size;
    bfht->size = new_size;
    // only if size is power of 2
    bfht->size_mask = new_size - 1;
    bfht->occupied_upper_limit = COMPUTE_ALPHA_UP_LIM(new_size);
    bfht->occupied_lower_limit = COMPUTE_ALPHA_LOW_LIM(new_size);
    // reset "occupied" to valid, because we're effectively discarding "deleted" locations
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
            position = PROBE(bfht->table[i].hash, idx++, bfht);
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

static void* _bfht_find_elem(Bfht* bfht, void* key)
{
    Elem* start_elem;
    int position;
    size_t idx = 0;
    uint32_t hash = def_hash(key);

    for (size_t i = 0; i < bfht->size; i++) {
        position = PROBE(hash, idx++, bfht);
        start_elem = &bfht->table[position];
        if (start_elem->free)
            break;
        if (start_elem->deleted)
            continue;
        if ((hash == start_elem->hash) && (bfht->key_compare(key, start_elem->key)))
            return start_elem;
    }

    return NULL;
}

void* bfht_find(Bfht* bfht, void* key)
{
    Elem* elem = _bfht_find_elem(bfht, key);
    if (elem != NULL)
        return elem->data;
    return NULL;
}

int bfht_delete(Bfht* bfht, void* key)
{
    size_t new_size;

    Elem* elem = _bfht_find_elem(bfht, key);
    if (elem == NULL) {
        return BFHT_OK;
    }

    if (bfht->data_destroy)
        bfht->data_destroy(elem->data);

    if (bfht->key_destroy)
        bfht->key_destroy(elem->key);

    elem->free = true,
    elem->deleted = true,

    bfht->valid -= 1;

    // resize
    // cap the size
    if ((bfht->valid == bfht->occupied_lower_limit) && (SHRINKED_SIZE(bfht) >= HT_INITIAL_SIZE)) {
        new_size = SHRINKED_SIZE(bfht);
        int ret = _bfht_resize(bfht, new_size);
        if (ret != BFHT_OK)
            return BFHT_DEL_WRN;
    }

    return BFHT_OK;
}

// Returns:
// BFHT_OK if everything was ok
// BFHT_UPDATE the inserted element was already inside, so the table updated the data (the old pointed data is destroyed)
// BFHT_INS_WRN if successfully inserted element but couldn't expand the hash table due to an Out of memory retor
// BFHT_EOOM if couldn't insert the element (hash table is full and can't expand further)

int bfht_insert(Bfht* bfht, void* key, void* data)
{
    size_t new_size;
    uint32_t hash;
    uint32_t position;
    int ret = BFHT_OK;
    size_t idx = 0;

    // resize
    // cap the size
    if ((bfht->occupied == bfht->occupied_upper_limit) && (EXPANDED_SIZE(bfht) < HT_MAX_SIZE)) {
        new_size = EXPANDED_SIZE(bfht) > HT_INITIAL_SIZE ? EXPANDED_SIZE(bfht) : HT_INITIAL_SIZE;
        int ret = _bfht_resize(bfht, new_size);
        if (ret != BFHT_OK)
            ret = BFHT_INS_WRN;
    }

    hash = def_hash(key);

    for (size_t i = 0; i < bfht->size; i++) {
        position = PROBE(hash, idx++, bfht);
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

Bfht* bfht_create(cmp_func key_compare, del_func key_destroy, del_func data_destroy)
{
    Bfht* bfht = malloc(sizeof(Bfht));
    if (bfht == NULL)
        return NULL;

    // the table starts empty and on the first insert there is the real allocation
    bfht->size = 0;
    bfht->size_mask = 0;
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

void bfht_destroy(Bfht* bfht)
{
    if (bfht->table != NULL) {
        free(bfht->table);
    }
    free(bfht);
}
