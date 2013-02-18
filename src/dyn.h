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

#ifndef dynU8
#define dynU8 unsigned char
#endif

#ifndef dynU16
#define dynU16 unsigned short
#endif

#ifndef dynU32
#define dynU32 unsigned int
#endif

#ifndef dynF32
#define dynF32 float
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
void daDestroyContents(void *daptr, void * /*dynDestroyFunc*/ destroyFunc);
void daDestroyPtr(void *daptr, void * /*dynDestroyFunc*/ destroyFunc);
void daDestroyP1(void *daptr, void * /*dynDestroyFuncP1*/ destroyFunc, void *p1);
void daDestroyP2(void *daptr, void * /*dynDestroyFuncP2*/ destroyFunc, void *p1, void *p2);
void daDestroyStrings(void *daptr);
void daClearContents(void *daptr, void * /*dynDestroyFunc*/ destroyFunc);
void daClearPtr(void *daptr, void * /*dynDestroyFunc*/ destroyFunc);
void daClearP1(void *daptr, void * /*dynDestroyFuncP1*/ destroyFunc, void *p1);
void daClearP2(void *daptr, void * /*dynDestroyFuncP2*/ destroyFunc, void *p1, void *p2);
void daClearStrings(void *daptr);

// front/back manipulation
int daShift(void *daptr, void *elementPtr); // returns false on an empty array
void daUnshiftContents(void *daptr, void *p);
#define daUnshift(DAPTR, P) daUnshiftContents(DAPTR, &(P))
void daUnshiftU8(void *daptr, dynU8 v);
void daUnshiftU16(void *daptr, dynU16 v);
void daUnshiftU32(void *daptr, dynU32 v);
void daUnshiftF32(void *daptr, dynF32 v);
dynSize daPushContents(void *daptr, void *entry);
#define daPush(DAPTR, P) daPushContents(DAPTR, &(P))
dynSize daPushU8(void *daptr, dynU8 v);
dynSize daPushU16(void *daptr, dynU16 v);
dynSize daPushU32(void *daptr, dynU32 v);
dynSize daPushF32(void *daptr, dynF32 v);
int daPop(void *daptr, void *elementPtr); // returns false on an empty array

// random access manipulation
void daInsertContents(void *daptr, dynSize index, void *p);
#define daInsert(DAPTR, INDEX, P) daInsertContents(DAPTR, INDEX, &(P))
void daInsertU8(void *daptr, dynSize index, dynU8 v);
void daInsertU16(void *daptr, dynSize index, dynU16 v);
void daInsertU32(void *daptr, dynSize index, dynU32 v);
void daInsertF32(void *daptr, dynSize index, dynF32 v);
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

typedef enum dmKeyFlags
{
    // mutually exclusive
    DKF_STRING       = (1 << 0),
    DKF_INTEGER      = (1 << 1),

    DKF_UNOWNED_KEYS = (1 << 2) // does not own/strdup string keys; meaningless on INTEGER maps
} dmKeyFlags;

typedef struct dynMapEntry
{
    union
    {
        char *keyStr;
        dynInt keyInt;
    };
    struct dynMapEntry *next;
    dynMapHash hash;
    // data is immediately following every entry's allocated block
} dynMapEntry;

typedef struct dynMapDefaultData
{
    union
    {
        void *valuePtr;
        dynInt valueInt;
        long long value64;
    };
} dynMapDefaultData;

typedef struct dynMap
{
    dynMapEntry **table; // Hash table daArray
    dynSize split;       // Linear Hashing 'split'
    dynSize mod;         // pre-split modulus (use mod*2 for overflow)
    dynSize elementSize;
    int flags;
    int count;           // count tracking for convenience
} dynMap;

dynMap *dmCreate(dmKeyFlags flags, dynSize elementSize);
void dmDestroyContents(dynMap *dm, void * /*dynDestroyFunc*/ destroyFunc);
void dmDestroyPtr(dynMap *dm, void * /*dynDestroyFunc*/ destroyFunc);
void dmClearContents(dynMap *dm, void * /*dynDestroyFunc*/ destroyFunc);
void dmClearPtr(dynMap *dm, void * /*dynDestroyFunc*/ destroyFunc);

dynMapEntry *dmGetString(dynMap *dm, const char *key);
int dmHasString(dynMap *dm, const char *key);
void dmEraseString(dynMap *dm, const char *key, void * /*dynDestroyFunc*/ destroyFunc);

dynMapEntry *dmGetInteger(dynMap *dm, dynInt key);
int dmHasInteger(dynMap *dm, dynInt key);
void dmEraseInteger(dynMap *dm, dynInt key, void * /*dynDestroyFunc*/ destroyFunc);

void *dmEntryData(dynMapEntry *entry);

// return non-zero to continue iterating, 0 to stop
typedef int (*dynMapIterateFunc)(dynMap *dm, dynMapEntry *e, void *userData);
void dmIterate(dynMap *dm, /* dynMapIterateFunc */ void *func, void *userData);

// Convenience macros

// "to string/integer pointers"
#define dmEntryDefaultData(E) ((dynMapDefaultData *)dmEntryData(E))
#define dmGetS2P(MAP, KEY) (dmEntryDefaultData(dmGetString(MAP, KEY))->valuePtr)
#define dmGetS2I(MAP, KEY) (dmEntryDefaultData(dmGetString(MAP, KEY))->valueInt)
#define dmGetI2P(MAP, KEY) (dmEntryDefaultData(dmGetInteger(MAP, KEY))->valuePtr)
#define dmGetI2I(MAP, KEY) (dmEntryDefaultData(dmGetInteger(MAP, KEY))->valueInt)

// "to 'Type' (custom structures / ptrs)"
#define dmGetS2T(MAP, TYPE, KEY) ((TYPE*)dmEntryData(dmGetString(MAP, KEY)))
#define dmGetI2T(MAP, TYPE, KEY) ((TYPE*)dmEntryData(dmGetInteger(MAP, KEY)))

// existence check aliases
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
