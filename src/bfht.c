#include "../bfht.h"
#include <ctype.h>
#include <stddef.h>
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


#define STORE_ELEM()

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
    Elem* table;
};

static void _bfht_resize(Bfht* bfht, size_t new_size)
{

    size_t old_size;
    Elem* old_elems;
    int position;
    size_t count = 0;
    size_t idx = 0;

    old_elems = bfht->table;
    old_size = bfht->size;

    bfht->size = new_size;
    // only if size is power of 2
    bfht->size_mask = bfht->size - 1;
    bfht->occupied_limit = COMPUTE_ALPHA_LIMIT(bfht->size);
    // reset "occupied" to valid, because we're effectively discarding "deleted" locations
    bfht->occupied = bfht->valid;

    // allocate
    bfht->table = malloc((bfht->size) * (sizeof(Elem)));

    for (size_t i = 0; i < bfht->size; i++)
        bfht->table[i].free = true;

    for (size_t i = 0; i < old_size; i++) {
        if (old_elems[i].free || old_elems[i].deleted)
            continue;
        // stop cicle if moved all the "valid"(effectively occupied) elems
        if (count == bfht->valid)
            break;

        while (1) {
            position = PROBE(old_elems[i].hash, idx++, bfht);
            // move the old elem in the new array
            if (bfht->table[position].free) {
                bfht->table[position].free = false;
                bfht->table[position].hash = old_elems[i].hash;
                bfht->table[position].key = old_elems[i].key;
                bfht->table[position].data = old_elems[i].data;
                count += 1;
                break;
            }
        }
    }

    // the first when allocating old_elems will be NULL
    if (old_elems != NULL)
        free(old_elems);
}

void* bfht_find(Bfht* bfht, void* key)
{
    Elem* start_elem;
    int position;
    size_t idx = 0;
    uint32_t hash = def_hash(key);

    for (size_t i = 0; i < bfht->size; i++) {
        position = PROBE(hash, idx++, bfht);
        start_elem = &bfht->table[position];
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
    size_t new_size;
    size_t idx = 0;
    uint32_t hash;
    uint32_t position;

    // resize
    // cap the size
    if ((bfht->occupied == bfht->occupied_limit) && ((bfht->size << 1) < HT_MAX_SIZE)) {
        new_size = (bfht->size << 1) > HT_INITIAL_SIZE ? (bfht->size << 1) : HT_INITIAL_SIZE;
        _bfht_resize(bfht, new_size);
    }

    for (size_t i = 0; i < bfht->size; i++) {
        hash = def_hash(key);
        position = PROBE(hash, idx, bfht);
        if (bfht->table[position].free) {
            bfht->table[position].free = false;
            bfht->table[position].deleted = false;
            bfht->table[position].hash = hash;
            bfht->table[position].key = key;
            bfht->table[position].data = data;
            return;
        }
    }
}

Bfht* bfht_create(cmp_func key_compare, del_func key_destroy, del_func data_destroy)
{
    Bfht* bfht = malloc(sizeof(Bfht));
    if (bfht == NULL)
        return NULL;

    // the table starts empty and on the first insert there is the real allocation
    bfht->size = 0;
    bfht->size_mask = 0;
    bfht->occupied_limit = 0;
    bfht->occupied = 0;
    bfht->valid = 0;
    bfht->key_compare = key_compare;
    bfht->data_destroy = data_destroy;
    bfht->key_destroy = key_destroy;

    return bfht;
}

void bfht_destroy(Bfht* bfht)
{
    free(bfht->table);
    free(bfht);
}
