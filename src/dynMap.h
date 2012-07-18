// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#ifndef DYNMAP_H
#define DYNMAP_H

#include "dynBase.h"

#ifndef dynMapHash
#define dynMapHash unsigned int // output from djb2hash
#endif

typedef enum dmKeyType
{
    KEYTYPE_STRING = 1,
    KEYTYPE_INTEGER
} dmKeyType;

typedef struct dynMapEntry
{
    union
    {
        char *keyStr;
        dynInt keyInt;
    };
    union
    {
        void *valuePtr;
        dynInt valueInt;
        long long value64;
    };
    struct dynMapEntry *next;
    dynMapHash hash;
} dynMapEntry;

typedef struct dynMap
{
    dynMapEntry **table; // Hash table daArray
    dynSize split;       // Linear Hashing 'split'
    dynSize level;       // Linear Hashing 'level'
    int keyType;
} dynMap;

dynMap *dmCreate(dmKeyType keyType, dynInt estimatedSize);
void dmDestroy(dynMap *dm, dynDestroyFunc destroyFunc);
void dmClear(dynMap *dm, dynDestroyFunc destroyFunc);

dynMapEntry *dmGetString(dynMap *dm, const char *key);
int dmHasString(dynMap *dm, const char *key);
void dmEraseString(dynMap *dm, const char *key, dynDestroyFunc destroyFunc);

dynMapEntry *dmGetInteger(dynMap *dm, dynInt key);
int dmHasInteger(dynMap *dm, dynInt key);
void dmEraseInteger(dynMap *dm, dynInt key, dynDestroyFunc destroyFunc);

void dmDebug(dynMap *dm);

// Convenience macros

#define dmGetS2P(MAP, KEY) (dmGetString(MAP, KEY)->valuePtr)
#define dmGetS2I(MAP, KEY) (dmGetString(MAP, KEY)->valueInt)
#define dmGetI2P(MAP, KEY) (dmGetInteger(MAP, KEY)->valuePtr)
#define dmGetI2I(MAP, KEY) (dmGetInteger(MAP, KEY)->valueInt)
#define dmHasS dmHasString
#define dmHasI dmHasInteger

#endif
