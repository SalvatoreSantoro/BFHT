#ifndef _HASH_FUNCTIONS_H
#define _HASH_FUNCTIONS_H

#include <stdint.h>
#include <string.h>

#if HASH_FUN == FNV1a
    #define FNV_OFFSET_BASIS 2166136261U
    #define FNV_PRIME        16777619U

static uint32_t def_hash(const void *data) {
    uint32_t hash = FNV_OFFSET_BASIS;
    const unsigned char *bytes = (const unsigned char *) data;

    while (*bytes) {
        hash ^= *bytes++;
        hash *= FNV_PRIME;
    }

    return hash;
}

#elif HASH_FUN == MURMUR

static uint32_t def_hash(const void *key) {
    uint32_t seed = 0x9747b28c;
    // a bit slower due to null terminated strings
    int len = strlen(key);
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    uint32_t h = seed ^ len;
    const unsigned char *data = (const unsigned char *) key;

    while (len >= 4) {
        uint32_t k = *(uint32_t *) data;
        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    switch (len) {
    case 3:
        h ^= data[2] << 16;
        __attribute__((fallthrough));
    case 2:
        h ^= data[1] << 8;
        __attribute__((fallthrough));
    case 1:
        h ^= data[0];
        h *= m;
    }

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

#elif HASH_FUN == DJB2
static uint32_t def_hash(const void *key) {
    int c;
    uint32_t hash = 5381;
    const char *p = (char *) key;
    while ((c = *p++)) {
        if (isupper(c)) {
            c = c + 32;
        }
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}
#else
    #error "Unsupported Hash Function"
#endif
#endif
