# BFHT
BFHT it's my attempt to implement a Blazingly Fast Hash Table in C with a straightforward API.

The project it's actually in a current usable state, but i will probably change some things in future based on necessities.
The real aim of BFHT is more of a "playground" to experiment and tune an open addressing hash table, according to a specific set of keys then a "fixed" implementation.

For this purpose the hash table supports the current parameters for tuning at compile time:
- Hash table initial size (Must be a power of two)
- Hash function: [FNV1a, MURMUR, DJB2]
- Size types: [PRIMES, TWOPOWERS] (SPECIFYING PRIMES AUTOMATICALLY IGNORES THE INITIAL SIZE PARAM)
- Load factor alpha: [125, 375, 500, 625, 75, 875] (they are intended as 0.125, 0.375 and so on)
- Probing strategies: [LINEAR, QUADRATIC]
- Max regrowings of the hash table (must be between 0 and 23)

## API
VERY IMPORTANT DISCLAIMER:

At the moment the Hash Table assumes that keys are "classic" NULL terminated C strings.

run make and use the "bfht.a" file inside the build folder to use the hash table (include bfht.h to use the API)

The interface is specified with "void *" anyway in order to use different hash functions that don't assume to work on necessary on strings

The parameters for tuning can be modified in the header "bfht.h" (that's also the header to include to use the bfht API) 
or directly at compile time specifying their value in the make command (es. **make ALPHA=500 PROBING=LINEAR**)

The API consists of a function to create the hash table (bfht_create) that just needs:
- Compare function to compare the keys (used inside the bfht_find)
- Deletion function to destroy keys when removing elements from the hash table, or destroying the whole hash table
- Destroy function to destroy the data associated to the keys (**VERY IMPORTANT CAVEAT REGARDING THIS, SEE "bfht_insert" specification**)

A function to destroy an hash table (bfht_destroy) that just needs the hash table to destroy.

A function to insert a key/value pair in the hash table (bfht_insert) if the key already existed, the hash table just overrides the data deleting the old one, **VERY IMPORTANT** in order to avoid **MEMORY LEAKS** be sure that if the "data" you're inserting in the hash table needs to be deallocated when "deleted", specify a correct del_func for the data when creating the hash table.
The hash table keeps working if you specify the del_func as **NULL**, so be prepared to **MEMORY LEAKS** if you just insert dynamically allocated keys and data and then don't provide del_func for both.

A function to remove a key/value pair from the hash table (bfht_remove)

A function to find the data associated to a key in the hash table (bfht_find)


```c
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

```

## PROFILING
To run the hash table profiling on a particular key dataset, you just need to place a file named "profiling" in the words folder.
(at the moment the profiling should only work on Linux cause i used CLOCK_MONOTONIC for the measures)

Then run "make profiling PROF_OFILE=[SOME_OUTPUT_FILE_NAME]" to just test the default values of the hash table or use the "profiling.py" (specifying -n as the number of measures repetitions) to test all the possible combinations of parameters in a GRID SEARCH.
You'll get the measures and some pyplot graphs in the "./prof" folder (note that the script resets these folders every time it's run, so it's advised to save them somewhere to keep them in various profiling runs).

The real profiling code is "prof.c" in prof folder. The code just calculate the mean time to do a fixed number of lookups (1000) of random keys, given an hash table that contains an increasing number of keys.

![Alpha](./prof/images/alpha.png)
![Hash](./prof/images/hash.png)
![Probing](./prof/images/probing.png)
![Max Growings](./prof/images/size_max_grow.png)
![Size Types](./prof/images/size_type.png)

## TODO
- Add AVX support for key lookup to increase performance

