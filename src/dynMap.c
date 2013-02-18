// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#include "dyn.h"

#include <stdlib.h>
#include <string.h>

// A quick thanks to the author of this web page:
//
// http://www.concentric.net/~ttwang/tech/sorthash.htm
//
// It seemed to offer the most straightforward explanation of how to implement this that I could
// find on the Internet, with its only flaw being that their example explaining a split uses
// bucket 2 at a time when their modulus is also 2, making it unclear that a rewind needs to
// rebucket split+mod instead of split*2 (or anything else). I eventually figured it out.

// ------------------------------------------------------------------------------------------------
// Hash function declarations

#ifdef DYN_USE_MURMUR3
static dynMapHash murmur3string(const unsigned char *str);
static dynMapHash murmur3int(dynInt i);
#define HASHSTRING murmur3string
#define HASHINT murmur3int
#endif

#ifdef DYN_USE_DJB2
static dynMapHash djb2string(const unsigned char *str);
static unsigned int djb2int(dynInt i);
#define HASHSTRING djb2string
#define HASHINT djb2int
#endif

#ifndef HASHSTRING
#error Please choose a hash function!
#endif

// ------------------------------------------------------------------------------------------------
// Constants and Macros

#define INITIAL_MODULUS 2  // "N" on Wikipedia's explanation of linear hashes
#define SHRINK_FACTOR   4  // How many times bigger does the table capacity have to be to its
                           // width to cause the table to shrink?

// ------------------------------------------------------------------------------------------------
// Internal helper functions

static dynSize linearHashCompute(dynMap *dm, dynMapHash hash)
{
    // Use a simple modulus on the hash, and if the resulting bucket is behind
    // "the split" (putting it in the front partition instead of the expansion),
    // rehash with the next-size-up modulus.

    dynSize addr = hash % dm->mod;
    if(addr < dm->split)
    {
        addr = hash % (dm->mod << 1);
    }
    return addr;
}

// This is used by dmNewEntry to chain a new entry into a bucket, and it is used
// by the split and rewind functions to rebucket everything in a single bucket.
static void dmBucketEntryChain(dynMap *dm, dynMapEntry *chain)
{
    while(chain)
    {
        dynMapEntry *entry = chain;
        dynInt tableIndex = linearHashCompute(dm, entry->hash);
        chain = chain->next;

        entry->next = dm->table[tableIndex];
        dm->table[tableIndex] = entry;
    }
}

static dynMapEntry *dmNewEntry(dynMap *dm, dynMapHash hash, void *key)
{
    dynMapEntry *entry;
    dynMapEntry *chain;
    int found = 0;

    // Create the new entry and bucket it
    entry = (dynMapEntry *)calloc(1, sizeof(*entry) + dm->elementSize);
    if(dm->flags & DKF_INTEGER)
    {
        // Integer keys
        entry->keyInt = *((int *)key);
    }
    else
    {
        // String keys
        if(dm->flags & DKF_UNOWNED_KEYS)
        {
            entry->keyStr = (char *)key;
        }
        else
        {
            entry->keyStr = strdup((char *)key);
        }
    }
    entry->hash = hash;
    dmBucketEntryChain(dm, entry);

    // Steal the chain at the split boundary...
    chain = dm->table[dm->split];
    dm->table[dm->split] = NULL;

    // ...advance the split...
    ++dm->split;
    if(dm->split == dm->mod)
    {
        // It is time to grow our linear hash!
        dm->mod *= 2;
        dm->split = 0;
        daSetSize(&dm->table, dm->mod << 1, NULL);
    }

    // ... and reattach the stolen chain.
    dmBucketEntryChain(dm, chain);

    ++dm->count;
    return entry;
}

static void dmRewindSplit(dynMap *dm)
{
    dynMapEntry *chain;
    dynSize indexToRebucket;

    --dm->split;
    if(dm->split < 0)
    {
        dm->mod >>= 1;
        dm->split = dm->mod - 1;
        daSetSize(&dm->table, dm->mod << 1, NULL);

        // Time to shrink!
        if((daSize(&dm->table) * SHRINK_FACTOR) < daCapacity(&dm->table))
        {
            daSetCapacity(&dm->table, daSize(&dm->table) * SHRINK_FACTOR, NULL); // Should be no need to destroy anything
        }
    }

    indexToRebucket = dm->split + dm->mod;
    chain = dm->table[indexToRebucket];
    dm->table[indexToRebucket] = NULL;

    dmBucketEntryChain(dm, chain);
}

// ------------------------------------------------------------------------------------------------
// creation / destruction / cleanup

dynMap *dmCreate(dmKeyFlags flags, dynSize elementSize)
{
    dynMap *dm = (dynMap *)calloc(1, sizeof(*dm));
    dm->flags = flags;
    dm->split = 0;
    dm->mod   = INITIAL_MODULUS;
    dm->count = 0;
    dm->elementSize = (elementSize > 0) ? elementSize : sizeof(dynMapDefaultData);
    daSetSize(&dm->table, dm->mod << 1, NULL);
    return dm;
}

void dmDestroyIndirect(dynMap *dm, void * /*dynDestroyFunc*/ destroyFunc)
{
    if(dm)
    {
        dmClearIndirect(dm, destroyFunc);
        daDestroyIndirect(&dm->table, NULL);
        free(dm);
    }
}

void dmDestroy(dynMap *dm, void * /*dynDestroyFunc*/ destroyFunc)
{
    if(dm)
    {
        dmClear(dm, destroyFunc);
        daDestroyIndirect(&dm->table, NULL);
        free(dm);
    }
}

static void dmDestroyEntry(dynMap *dm, dynMapEntry *p)
{
    if((dm->flags & (DKF_STRING|DKF_UNOWNED_KEYS)) == DKF_STRING) // string map with owned keys?
    {
        free(p->keyStr);
    }
    free(p);
}

static void dmClearInternal(dynMap *dm, void * /*dynDestroyFunc*/ destroyFunc, int ptrs)
{
    dynDestroyFunc func = destroyFunc;
    if(dm)
    {
        dynInt tableIndex;
        for(tableIndex = 0; tableIndex < daSize(&dm->table); ++tableIndex)
        {
            dynMapEntry *entry = dm->table[tableIndex];
            while(entry)
            {
                dynMapEntry *freeme = entry;
                void *data = dmEntryData(freeme);
                if(func)
                {
                    if(ptrs)
                    {
                        char **p = (char **)data;
                        func(*p);
                    }
                    else
                    {
                        func(data);
                    }
                }
                entry = entry->next;
                dmDestroyEntry(dm, freeme);
            }
        }
        memset(dm->table, 0, daSize(&dm->table) * sizeof(dynMapEntry*));
    }
}

void dmClearIndirect(dynMap *dm, void * /*dynDestroyFunc*/ destroyFunc)
{
    dmClearInternal(dm, destroyFunc, 0);
}

void dmClear(dynMap *dm, void * /*dynDestroyFunc*/ destroyFunc)
{
    dmClearInternal(dm, destroyFunc, 1);
}

static dynMapEntry *dmFindString(dynMap *dm, const char *key, int autoCreate)
{
    dynMapHash hash = (dynMapHash)HASHSTRING(key);
    dynInt index = linearHashCompute(dm, hash);
    dynMapEntry *entry = dm->table[index];
    for( ; entry; entry = entry->next)
    {
        if(!strcmp(entry->keyStr, key))
            return entry;
    }

    if(autoCreate)
    {
        // A new entry!
        return dmNewEntry(dm, hash, (void*)key);
    }
    return NULL;
}

static dynMapEntry *dmFindInteger(dynMap *dm, dynInt key, int autoCreate)
{
    dynMapHash hash = (dynMapHash)HASHINT(key);
    dynInt index = linearHashCompute(dm, hash);
    dynMapEntry *entry = dm->table[index];
    for( ; entry; entry = entry->next)
    {
        if(entry->keyInt == key)
            return entry;
    }

    if(autoCreate)
    {
        // A new entry!
        return dmNewEntry(dm, hash, (void*)&key);
    }
    return NULL;
}

dynMapEntry *dmGetString(dynMap *dm, const char *key)
{
    return dmFindString(dm, key, 1);
}

int dmHasString(dynMap *dm, const char *key)
{
    return (dmFindString(dm, key, 0) != NULL);
}

void dmEraseString(dynMap *dm, const char *key, void * /*dynDestroyFunc*/ destroyFunc)
{
    dynDestroyFunc func = destroyFunc;
    dynMapHash hash = (dynMapHash)HASHSTRING(key);
    dynInt index = linearHashCompute(dm, hash);
    dynMapEntry *prev = NULL;
    dynMapEntry *entry = dm->table[index];
    for( ; entry; prev = entry, entry = entry->next)
    {
        if(!strcmp(entry->keyStr, key))
        {
            void *data = dmEntryData(entry);
            if(prev)
            {
                prev->next = entry->next;
            }
            else
            {
                dm->table[index] = entry->next;
            }
            if(func)
                func(data);
            dmDestroyEntry(dm, entry);
            --dm->count;
            dmRewindSplit(dm);
            return;
        }
    }
}

dynMapEntry *dmGetInteger(dynMap *dm, dynInt key)
{
    return dmFindInteger(dm, key, 1);
}

int dmHasInteger(dynMap *dm, dynInt key)
{
    return (dmFindInteger(dm, key, 0) != NULL);
}

void dmEraseInteger(dynMap *dm, dynInt key, void * /*dynDestroyFunc*/ destroyFunc)
{
    dynDestroyFunc func = destroyFunc;
    dynMapHash hash = (dynMapHash)HASHINT(key);
    dynInt index = linearHashCompute(dm, hash);
    dynMapEntry *prev = NULL;
    dynMapEntry *entry = dm->table[index];
    for( ; entry; prev = entry, entry = entry->next)
    {
        if(entry->keyInt == key)
        {
            void *data = dmEntryData(entry);
            if(prev)
            {
                prev->next = entry->next;
            }
            else
            {
                dm->table[index] = entry->next;
            }
            if(func)
                func(data);
            dmDestroyEntry(dm, entry);
            --dm->count;
            dmRewindSplit(dm);
            return;
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Iteration

//typedef void (*dynMapIterateFunc)(dynMap *dm, dynMapEntry *e, void *userData);

void dmIterate(dynMap *dm, /* dynMapIterateFunc */ void *func, void *userData)
{
    dynMapIterateFunc itFunc = (dynMapIterateFunc)func;
    int bucketCount = dm->split + dm->mod;
    int i;
    for(i = 0; i < bucketCount; ++i)
    {
        dynMapEntry *entry = dm->table[i];
        for( ; entry; entry = entry->next)
        {
            itFunc(dm, entry, userData);
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Entry funcs

void *dmEntryData(dynMapEntry *entry)
{
    return (void *)(((char *)entry) + sizeof(dynMapEntry));
}

#ifdef DYN_USE_MURMUR3

//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// Note - The x86 and x64 versions do _not_ produce the same results, as the
// algorithms are optimized for their respective platforms. You can still
// compile and run any of them on any platform, but your performance with the
// non-native version will be less than optimal.

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER)

typedef unsigned char uint8_t;
typedef unsigned long uint32_t;
typedef unsigned __int64 uint64_t;

// Other compilers

#else	// defined(_MSC_VER)

#include <stdint.h>

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------

void MurmurHash3_x86_32  ( const void * key, int len, uint32_t seed, void * out );
void MurmurHash3_x86_128 ( const void * key, int len, uint32_t seed, void * out );
void MurmurHash3_x64_128 ( const void * key, int len, uint32_t seed, void * out );

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER)

#define FORCE_INLINE	__forceinline

#include <stdlib.h>

#define ROTL32(x,y)	_rotl(x,y)
#define ROTL64(x,y)	_rotl64(x,y)

#define BIG_CONSTANT(x) (x)

// Other compilers

#else	// defined(_MSC_VER)

#define	FORCE_INLINE __attribute__((always_inline))

inline uint32_t rotl32 ( uint32_t x, int8_t r )
{
    return (x << r) | (x >> (32 - r));
}

inline uint64_t rotl64 ( uint64_t x, int8_t r )
{
    return (x << r) | (x >> (64 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

FORCE_INLINE uint32_t getblock32 ( const uint32_t * p, int i )
{
    return p[i];
}

FORCE_INLINE uint64_t getblock64 ( const uint64_t * p, int i )
{
    return p[i];
}

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

FORCE_INLINE uint32_t fmix32 ( uint32_t h )
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

//----------

FORCE_INLINE uint64_t fmix64 ( uint64_t k )
{
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xff51afd7ed558ccd);
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
    k ^= k >> 33;

    return k;
}

//-----------------------------------------------------------------------------

void MurmurHash3_x86_32 ( const void * key, int len,
                         uint32_t seed, void * out )
{
    const uint8_t * tail;
    uint32_t k1;

    const uint8_t * data = (const uint8_t*)key;
    const int nblocks = len / 4;

    uint32_t h1 = seed;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    int i;

    //----------
    // body

    const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);

    for(i = -nblocks; i; i++)
    {
        uint32_t k1 = getblock32(blocks,i);

        k1 *= c1;
        k1 = ROTL32(k1,15);
        k1 *= c2;

        h1 ^= k1;
        h1 = ROTL32(h1,13); 
        h1 = h1*5+0xe6546b64;
    }

    //----------
    // tail

    tail = (const uint8_t*)(data + nblocks*4);

    k1 = 0;

    switch(len & 3)
    {
    case 3: k1 ^= tail[2] << 16;
    case 2: k1 ^= tail[1] << 8;
    case 1: k1 ^= tail[0];
        k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= len;

    h1 = fmix32(h1);

    *(uint32_t*)out = h1;
} 

//-----------------------------------------------------------------------------

void MurmurHash3_x86_128 ( const void * key, int len,
                          uint32_t seed, void * out )
{
    int i;
    const uint8_t * tail;
    const uint8_t * data = (const uint8_t*)key;
    const int nblocks = len / 16;

    uint32_t h1 = seed;
    uint32_t h2 = seed;
    uint32_t h3 = seed;
    uint32_t h4 = seed;

    uint32_t k1 = 0;
    uint32_t k2 = 0;
    uint32_t k3 = 0;
    uint32_t k4 = 0;

    const uint32_t c1 = 0x239b961b; 
    const uint32_t c2 = 0xab0e9789;
    const uint32_t c3 = 0x38b34ae5; 
    const uint32_t c4 = 0xa1e38b93;

    //----------
    // body

    const uint32_t * blocks = (const uint32_t *)(data + nblocks*16);

    for(i = -nblocks; i; i++)
    {
        uint32_t k1 = getblock32(blocks,i*4+0);
        uint32_t k2 = getblock32(blocks,i*4+1);
        uint32_t k3 = getblock32(blocks,i*4+2);
        uint32_t k4 = getblock32(blocks,i*4+3);

        k1 *= c1; k1  = ROTL32(k1,15); k1 *= c2; h1 ^= k1;

        h1 = ROTL32(h1,19); h1 += h2; h1 = h1*5+0x561ccd1b;

        k2 *= c2; k2  = ROTL32(k2,16); k2 *= c3; h2 ^= k2;

        h2 = ROTL32(h2,17); h2 += h3; h2 = h2*5+0x0bcaa747;

        k3 *= c3; k3  = ROTL32(k3,17); k3 *= c4; h3 ^= k3;

        h3 = ROTL32(h3,15); h3 += h4; h3 = h3*5+0x96cd1c35;

        k4 *= c4; k4  = ROTL32(k4,18); k4 *= c1; h4 ^= k4;

        h4 = ROTL32(h4,13); h4 += h1; h4 = h4*5+0x32ac3b17;
    }

    //----------
    // tail

    tail = (const uint8_t*)(data + nblocks*16);

    switch(len & 15)
    {
    case 15: k4 ^= tail[14] << 16;
    case 14: k4 ^= tail[13] << 8;
    case 13: k4 ^= tail[12] << 0;
        k4 *= c4; k4  = ROTL32(k4,18); k4 *= c1; h4 ^= k4;

    case 12: k3 ^= tail[11] << 24;
    case 11: k3 ^= tail[10] << 16;
    case 10: k3 ^= tail[ 9] << 8;
    case  9: k3 ^= tail[ 8] << 0;
        k3 *= c3; k3  = ROTL32(k3,17); k3 *= c4; h3 ^= k3;

    case  8: k2 ^= tail[ 7] << 24;
    case  7: k2 ^= tail[ 6] << 16;
    case  6: k2 ^= tail[ 5] << 8;
    case  5: k2 ^= tail[ 4] << 0;
        k2 *= c2; k2  = ROTL32(k2,16); k2 *= c3; h2 ^= k2;

    case  4: k1 ^= tail[ 3] << 24;
    case  3: k1 ^= tail[ 2] << 16;
    case  2: k1 ^= tail[ 1] << 8;
    case  1: k1 ^= tail[ 0] << 0;
        k1 *= c1; k1  = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= len; h2 ^= len; h3 ^= len; h4 ^= len;

    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;

    h1 = fmix32(h1);
    h2 = fmix32(h2);
    h3 = fmix32(h3);
    h4 = fmix32(h4);

    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;

    ((uint32_t*)out)[0] = h1;
    ((uint32_t*)out)[1] = h2;
    ((uint32_t*)out)[2] = h3;
    ((uint32_t*)out)[3] = h4;
}

//-----------------------------------------------------------------------------

void MurmurHash3_x64_128 ( const void * key, int len,
                          uint32_t seed, void * out )
{
    int i;
    const uint8_t * data = (const uint8_t*)key;
    const int nblocks = len / 16;
    const uint8_t * tail;

    uint64_t h1 = seed;
    uint64_t h2 = seed;

    uint64_t k1 = 0;
    uint64_t k2 = 0;

    const uint64_t c1 = BIG_CONSTANT(0x87c37b91114253d5);
    const uint64_t c2 = BIG_CONSTANT(0x4cf5ad432745937f);

    //----------
    // body

    const uint64_t * blocks = (const uint64_t *)(data);

    for(i = 0; i < nblocks; i++)
    {
        uint64_t k1 = getblock64(blocks,i*2+0);
        uint64_t k2 = getblock64(blocks,i*2+1);

        k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;

        h1 = ROTL64(h1,27); h1 += h2; h1 = h1*5+0x52dce729;

        k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;

        h2 = ROTL64(h2,31); h2 += h1; h2 = h2*5+0x38495ab5;
    }

    //----------
    // tail

    tail = (const uint8_t*)(data + nblocks*16);

    switch(len & 15)
    {
    case 15: k2 ^= ((uint64_t)tail[14]) << 48;
    case 14: k2 ^= ((uint64_t)tail[13]) << 40;
    case 13: k2 ^= ((uint64_t)tail[12]) << 32;
    case 12: k2 ^= ((uint64_t)tail[11]) << 24;
    case 11: k2 ^= ((uint64_t)tail[10]) << 16;
    case 10: k2 ^= ((uint64_t)tail[ 9]) << 8;
    case  9: k2 ^= ((uint64_t)tail[ 8]) << 0;
        k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;

    case  8: k1 ^= ((uint64_t)tail[ 7]) << 56;
    case  7: k1 ^= ((uint64_t)tail[ 6]) << 48;
    case  6: k1 ^= ((uint64_t)tail[ 5]) << 40;
    case  5: k1 ^= ((uint64_t)tail[ 4]) << 32;
    case  4: k1 ^= ((uint64_t)tail[ 3]) << 24;
    case  3: k1 ^= ((uint64_t)tail[ 2]) << 16;
    case  2: k1 ^= ((uint64_t)tail[ 1]) << 8;
    case  1: k1 ^= ((uint64_t)tail[ 0]) << 0;
        k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= len; h2 ^= len;

    h1 += h2;
    h2 += h1;

    h1 = fmix64(h1);
    h2 = fmix64(h2);

    h1 += h2;
    h2 += h1;

    ((uint64_t*)out)[0] = h1;
    ((uint64_t*)out)[1] = h2;
}

static dynMapHash murmur3string(const unsigned char *str)
{
    dynMapHash hash;
    MurmurHash3_x86_32(str, strlen(str), 0, &hash);
    return hash;
}

static dynMapHash murmur3int(dynInt i)
{
    dynMapHash hash;
    MurmurHash3_x86_32(&i, sizeof(dynInt), 0, &hash);
    return hash;
}

#endif

//-----------------------------------------------------------------------------

#ifdef DYN_USE_DJB2

static dynMapHash djb2string(const unsigned char *str)
{
    dynMapHash hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

static unsigned int djb2int(dynInt i)
{
    const char *p = (const char *)&i;
    unsigned int hash = 5381;

    hash = ((hash << 5) + hash) + p[0];
    hash = ((hash << 5) + hash) + p[1];
    hash = ((hash << 5) + hash) + p[2];
    hash = ((hash << 5) + hash) + p[3];
    return hash;
}

#endif
