// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#ifndef DYN_H
#define DYN_H

// ---------------------------------------------------------------------------
// Required headers

#include <stdarg.h>

// ---------------------------------------------------------------------------
// Definitions

#ifndef dynInt
#define dynInt int
#endif

#ifndef dynSize
#define dynSize int
#endif

#ifndef dynSizeFormat
#define dynSizeFormat "%d"
#endif

#ifndef dynMapHash
#define dynMapHash unsigned int // output from djb2hash
#endif

#if !DYN_USE_MURMUR3 && !DYN_USE_DJB2
#define DYN_USE_MURMUR3 1
//#define DYN_USE_DJB2 1
#endif

// ---------------------------------------------------------------------------
// Generic Callback Signatures

// P1 and P2 are "prefixed arguments", useful for passing userdata into a destroy function
typedef void (*dynDestroyFunc)(void *p);
typedef void (*dynDestroyFuncP1)(void *p1, void *p);
typedef void (*dynDestroyFuncP2)(void *p1, void *p2, void *p);

// ---------------------------------------------------------------------------
// Array

// creation / destruction / cleanup
void daCreate(void *daptr, dynSize elementSize); // use elementSize=0 for "pointer sized"
void daDestroy(void *daptr, void * /*dynDestroyFunc*/ destroyFunc);
void daDestroyP1(void *daptr, void * /*dynDestroyFuncP1*/ destroyFunc, void *p1);
void daDestroyP2(void *daptr, void * /*dynDestroyFuncP2*/ destroyFunc, void *p1, void *p2);
void daDestroyStrings(void *daptr);
void daClear(void *daptr, void * /*dynDestroyFunc*/ destroyFunc);
void daClearP1(void *daptr, void * /*dynDestroyFuncP1*/ destroyFunc, void *p1);
void daClearP2(void *daptr, void * /*dynDestroyFuncP2*/ destroyFunc, void *p1, void *p2);
void daClearStrings(void *daptr);

// front/back manipulation
int daShift(void *daptr, void *elementPtr); // returns false on an empty array
void daUnshiftContents(void *daptr, void *p);
#define daUnshift(DAPTR, P) daUnshiftContents(DAPTR, &(P))
dynSize daPushContents(void *daptr, void *entry);
#define daPush(DAPTR, P) daPushContents(DAPTR, &(P))
int daPop(void *daptr, void *elementPtr); // returns false on an empty array

// random access manipulation
void daInsertContents(void *daptr, dynSize index, void *p);
#define daInsert(DAPTR, INDEX, P) daInsertContents(DAPTR, INDEX, &(P))
void daErase(void *daptr, dynSize index);

// Size manipulation
void daSetSize(void *daptr, dynSize newSize, void * /*dynDestroyFunc*/ destroyFunc);
void daSetSizeP1(void *daptr, dynSize newSize, void * /*dynDestroyFuncP1*/ destroyFunc, void *p1);
void daSetSizeP2(void *daptr, dynSize newSize, void * /*dynDestroyFuncP2*/ destroyFunc, void *p1, void *p2);
dynSize daSize(void *daptr);
void daSetCapacity(void *daptr, dynSize newCapacity, void * /*dynDestroyFunc*/ destroyFunc);
void daSetCapacityP1(void *daptr, dynSize newCapacity, void * /*dynDestroyFuncP1*/ destroyFunc, void *p1);
void daSetCapacityP2(void *daptr, dynSize newCapacity, void * /*dynDestroyFuncP2*/ destroyFunc, void *p1, void *p2);
dynSize daCapacity(void *daptr);
void daSquash(void *daptr);

// ---------------------------------------------------------------------------
// Map

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
    dynSize mod;         // pre-split modulus (use mod*2 for overflow)
    int keyType;
    int count;           // count tracking for convenience
} dynMap;

dynMap *dmCreate(dmKeyType keyType, dynInt estimatedSize);
void dmDestroy(dynMap *dm, void * /*dynDestroyFunc*/ destroyFunc);
void dmClear(dynMap *dm, void * /*dynDestroyFunc*/ destroyFunc);

dynMapEntry *dmGetString(dynMap *dm, const char *key);
int dmHasString(dynMap *dm, const char *key);
void dmEraseString(dynMap *dm, const char *key, void * /*dynDestroyFunc*/ destroyFunc);

dynMapEntry *dmGetInteger(dynMap *dm, dynInt key);
int dmHasInteger(dynMap *dm, dynInt key);
void dmEraseInteger(dynMap *dm, dynInt key, void * /*dynDestroyFunc*/ destroyFunc);

// Convenience macros

#define dmGetS2P(MAP, KEY) (dmGetString(MAP, KEY)->valuePtr)
#define dmGetS2I(MAP, KEY) (dmGetString(MAP, KEY)->valueInt)
#define dmGetI2P(MAP, KEY) (dmGetInteger(MAP, KEY)->valuePtr)
#define dmGetI2I(MAP, KEY) (dmGetInteger(MAP, KEY)->valueInt)
#define dmHasS dmHasString
#define dmHasI dmHasInteger

// ---------------------------------------------------------------------------
// String

// creation / destruction / cleanup
void dsCreate(char **dsptr);
void dsDestroy(char **dsptr);
void dsDestroyIndirect(char *ds);
void dsClear(char **dsptr);
char *dsDup(const char *text);
char *dsDupf(const char *format, ...);

// manipulation
void dsCopyLen(char **dsptr, const char *text, dynSize len);
void dsCopy(char **dsptr, const char *text);
void dsConcatLen(char **dsptr, const char *text, dynSize len);
void dsConcat(char **dsptr, const char *text);
void dsPrintf(char **dsptr, const char *format, ...);
void dsConcatv(char **dsptr, const char *format, va_list args);
void dsConcatf(char **dsptr, const char *format, ...);
void dsSetLength(char **dsptr, dynSize newLength);
void dsCalcLength(char **dsptr);
void dsSetCapacity(char **dsptr, dynSize newCapacity);

// information / testing
int dsCmp(char **dsptr, char **other);
dynSize dsLength(char **dsptr);
dynSize dsCapacity(char **dsptr);

#endif
