#ifndef _BFHT_MACROS_H
#define _BFHT_MACROS_H

#include "../bfht.h"

// Range check (must be between 0 and 23)
#if (HT_SIZE_MAX_GROWINGS < 0) || (HT_SIZE_MAX_GROWINGS > 23)
    #error "HT_SIZE_MAX_GROWINGS must be between 0 and 23"
#endif

// ALPHA
#if ALPHA == 125
    #define COMPUTE_ALPHA_UP_LIM(x)  ((x) >> 3)
    #define COMPUTE_ALPHA_LOW_LIM(x) ((x) >> 5)
#elif ALPHA == 250
    #define COMPUTE_ALPHA_UP_LIM(x)  ((x) >> 2)
    #define COMPUTE_ALPHA_LOW_LIM(x) ((x) >> 4)
#elif ALPHA == 375
    #define COMPUTE_ALPHA_UP_LIM(x)  (((x) >> 2) + ((x) >> 3))
    #define COMPUTE_ALPHA_LOW_LIM(x) ((x) >> 4) + ((x) >> 5)
#elif ALPHA == 500
    #define COMPUTE_ALPHA_UP_LIM(x)  ((x) >> 1)
    #define COMPUTE_ALPHA_LOW_LIM(x) ((x) >> 3)
#elif ALPHA == 625
    #define COMPUTE_ALPHA_UP_LIM(x)  (((x) >> 1) + ((x) >> 3))
    #define COMPUTE_ALPHA_LOW_LIM(x) (((x) >> 3) + ((x) >> 5))
#elif ALPHA == 750
    #define COMPUTE_ALPHA_UP_LIM(x)  (((x) >> 1) + ((x) >> 2))
    #define COMPUTE_ALPHA_LOW_LIM(x) (((x) >> 3) + ((x) >> 4))
#elif ALPHA == 875
    #define COMPUTE_ALPHA_UP_LIM(x)  (((x) >> 1) + ((x) >> 2) + ((x) >> 3))
    #define COMPUTE_ALPHA_LOW_LIM(x) (((x) >> 3) + ((x) >> 5) + ((x) >> 5))
#else
    #error "Unsupported ALPHA value"
#endif

// PROBING
#define LINEAR    1
#define QUADRATIC 0

// SIZE_TYPE
#define PRIMES     1
#define TWO_POWERS 0

#if PROBING == LINEAR
    #define PROBE(h_k, i, ht) MODULO((h_k + i), ht)
#elif PROBING == QUADRATIC
    #include <math.h>
    #define PROBE(h_k, i, ht) MODULO((h_k + i + (size_t) pow(i, 2)), ht)
#else
    #error "Unsupported PROBING value"
#endif

// SIZE
#if HT_SIZE_TYPE == PRIMES
// VERY IMPORTANT
// Note that primes[0] and primes[25] are just "barriers" value, should never be effectively used as
// proper hash table sizes, this could break things when using EXPANDED_SIZE and SHRINKED_SIZE
// macros (going out of bounds of array)
static int __primes[26] = {0,         193,       389,       769,        1543,      3079,     6151,
                           12289,     24593,     49157,     98317,      196613,    393241,   786433,
                           1572869,   3145739,   6291469,   12582917,   25165843,  50331653, 100663319,
                           201326611, 402653189, 805306457, 1610612741, 1610612742};
    // starting from location 1 and resizing n times, you end up in n+1
    #define HT_MAX_SIZE      (__primes[HT_SIZE_MAX_GROWINGS + 1])
    #define HT_MIN_SIZE      (__primes[1])
    #define MODULO(h, t)     (h % t->size)
    #define EXPANDED_SIZE(h) (__primes[h->primes_pos + 1])
    #define SHRINKED_SIZE(h) (__primes[h->primes_pos - 1])
#elif HT_SIZE_TYPE == TWO_POWERS // power of 2 size
    #define HT_MAX_SIZE      ((size_t)HT_INITIAL_SIZE << HT_SIZE_MAX_GROWINGS)
    #define HT_MIN_SIZE      (HT_INITIAL_SIZE)
    #define MODULO(h, t)     (h & t->size_mask)
    #define EXPANDED_SIZE(h) (h->size << 1)
    #define SHRINKED_SIZE(h) (h->size >> 1)
    #define IS_POWER_OF_2(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))
    #if !IS_POWER_OF_2(HT_INITIAL_SIZE)
        #error "HT_INITIAL_SIZE is not a power of 2!"
    #endif
#else
    #error "Unsupported size type"
#endif

// UTILS

#define STORE_ELEM(e, h, k, d)                                                                                         \
    do {                                                                                                               \
        e.free = false;                                                                                                \
        e.deleted = false;                                                                                             \
        e.hash = h;                                                                                                    \
        e.key = k;                                                                                                     \
        e.data = d;                                                                                                    \
    } while (0)

#endif // !_BFHT_MACROS_H
