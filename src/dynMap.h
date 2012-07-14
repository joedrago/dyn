// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#ifndef DYNAMIC_MAP_H
#define DYNAMIC_MAP_H

#ifndef DYNAMIC_MAP_SIZE_TYPE
#define DYNAMIC_MAP_SIZE_TYPE int
#endif

#ifndef DYNAMIC_MAP_HASH_TYPE
#define DYNAMIC_MAP_HASH_TYPE unsigned int
#endif

typedef enum dmKeyType
{
    KEYTYPE_STRING = 1,
    KEYTYPE_INTEGER,
} dmKeyType;

#define DYNAMIC_MAP_DEFAULT_KEYTYPE KEYTYPE_STRING
#define DYNAMIC_MAP_DEFAULT_WIDTH 7
#define DYNAMIC_MAP_DEFAULT_VALUE_SIZE sizeof(char*)

#define dmGetS(MAP, KEY) MAP[dmStringIndex(&MAP, KEY)]
#define dmGetI(MAP, KEY) MAP[dmIntegerIndex(&MAP, KEY)]

typedef (*dmDestroyFunc)(void *p);
typedef (*dmDestroyFuncP1)(void *p1, void *p);
typedef (*dmDestroyFuncP2)(void *p1, void *p2, void *p);

void dmCreate(void *dmptr, dmKeyType keyType, DYNAMIC_MAP_SIZE_TYPE valueSize, DYNAMIC_MAP_SIZE_TYPE tableWidth);
void dmDestroy(void *dmptr, dmDestroyFunc destroyFunc);
void dmClear(void *dmptr, dmDestroyFunc destroyFunc);
DYNAMIC_MAP_SIZE_TYPE dmStringIndex(void *dmptr, const char *key);
DYNAMIC_MAP_SIZE_TYPE dmIntegerIndex(void *dmptr, int key);
int dmHasStringIndex(void *dmptr, const char *str);

#endif