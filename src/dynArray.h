// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#ifdef DYNAMIC_ARRAY_PREFIX
#define daCreate        DYNAMIC_ARRAY_PREFIX ## Create
#define daDestroy       DYNAMIC_ARRAY_PREFIX ## Destroy
#define daDestroyP1     DYNAMIC_ARRAY_PREFIX ## DestroyP1
#define daDestroyP2     DYNAMIC_ARRAY_PREFIX ## DestroyP2
#define daDestroyFunc   DYNAMIC_ARRAY_PREFIX ## DestroyFunc
#define daDestroyFuncP1 DYNAMIC_ARRAY_PREFIX ## DestroyFuncP1
#define daDestroyFuncP2 DYNAMIC_ARRAY_PREFIX ## DestroyFuncP2
#define daClear         DYNAMIC_ARRAY_PREFIX ## Clear
#define daClearP1       DYNAMIC_ARRAY_PREFIX ## ClearP1
#define daClearP2       DYNAMIC_ARRAY_PREFIX ## ClearP2
#define daShift         DYNAMIC_ARRAY_PREFIX ## Shift
#define daUnshift       DYNAMIC_ARRAY_PREFIX ## Unshift
#define daPush          DYNAMIC_ARRAY_PREFIX ## Push
#define daPop           DYNAMIC_ARRAY_PREFIX ## Pop
#define daInsert        DYNAMIC_ARRAY_PREFIX ## Insert
#define daErase         DYNAMIC_ARRAY_PREFIX ## Erase
#define daSetSize       DYNAMIC_ARRAY_PREFIX ## SetSize
#define daSetSizeP1     DYNAMIC_ARRAY_PREFIX ## SetSizeP1
#define daSetSizeP2     DYNAMIC_ARRAY_PREFIX ## SetSizeP2
#define daSize          DYNAMIC_ARRAY_PREFIX ## Size
#define daSetCapacity   DYNAMIC_ARRAY_PREFIX ## SetCapacity
#define daSetCapacityP1 DYNAMIC_ARRAY_PREFIX ## SetCapacityP1
#define daSetCapacityP2 DYNAMIC_ARRAY_PREFIX ## SetCapacityP2
#define daCapacity      DYNAMIC_ARRAY_PREFIX ## Capacity
#define daSquash        DYNAMIC_ARRAY_PREFIX ## Squash
#endif

#ifndef DYNAMIC_ARRAY_SIZE_TYPE
#define DYNAMIC_ARRAY_SIZE_TYPE int
#undef DYNAMIC_ARRAY_SIZE_FORMAT
#define DYNAMIC_ARRAY_SIZE_FORMAT "%d"
#endif
#ifndef DYNAMIC_ARRAY_SIZE_FORMAT
#error If you define DYNAMIC_ARRAY_SIZE_TYPE, you must also define DYNAMIC_ARRAY_SIZE_FORMAT.
#endif

// P1 and P2 are "prefixed arguments", useful for passing userdata into a destroy function

typedef (*daDestroyFunc)(void *p);
typedef (*daDestroyFuncP1)(void *p1, void *p);
typedef (*daDestroyFuncP2)(void *p1, void *p2, void *p);

// creation / destruction / cleanup
void daCreate(void *daptr);
void daDestroy(void *daptr, daDestroyFunc destroyFunc);
void daDestroyP1(void *daptr, daDestroyFuncP1 destroyFunc, void *p1);
void daDestroyP2(void *daptr, daDestroyFuncP2 destroyFunc, void *p1, void *p2);
void daClear(void *daptr, daDestroyFunc destroyFunc);
void daClearP1(void *daptr, daDestroyFuncP1 destroyFunc, void *p1);
void daClearP2(void *daptr, daDestroyFuncP2 destroyFunc, void *p1, void *p2);

// front/back manipulation
void *daShift(void *daptr);
void daUnshift(void *daptr, void *p);
DYNAMIC_ARRAY_SIZE_TYPE daPush(void *daptr, void *entry);
void *daPop(void *daptr);

// random access manipulation
void daInsert(void *daptr, DYNAMIC_ARRAY_SIZE_TYPE index, void *p);
void daErase(void *daptr, DYNAMIC_ARRAY_SIZE_TYPE index);

// Size manipulation
void daSetSize(void *daptr, DYNAMIC_ARRAY_SIZE_TYPE newSize, daDestroyFunc destroyFunc);
void daSetSizeP1(void *daptr, DYNAMIC_ARRAY_SIZE_TYPE newSize, daDestroyFuncP1 destroyFunc, void *p1);
void daSetSizeP2(void *daptr, DYNAMIC_ARRAY_SIZE_TYPE newSize, daDestroyFuncP2 destroyFunc, void *p1, void *p2);
DYNAMIC_ARRAY_SIZE_TYPE daSize(void *daptr);
void daSetCapacity(void *daptr, DYNAMIC_ARRAY_SIZE_TYPE newCapacity, daDestroyFunc destroyFunc);
void daSetCapacityP1(void *daptr, DYNAMIC_ARRAY_SIZE_TYPE newCapacity, daDestroyFuncP1 destroyFunc, void *p1);
void daSetCapacityP2(void *daptr, DYNAMIC_ARRAY_SIZE_TYPE newCapacity, daDestroyFuncP2 destroyFunc, void *p1, void *p2);
DYNAMIC_ARRAY_SIZE_TYPE daCapacity(void *daptr);
void daSquash(void *daptr);

// Undefine these to minimize name clashing with preexisting code
#ifdef DYNAMIC_ARRAY_PREFIX
#undef daCreate
#undef daDestroy
#undef daDestroyP1
#undef daDestroyP2
#undef daDestroyFunc
#undef daDestroyFuncP1
#undef daDestroyFuncP2
#undef daClear
#undef daClearP1
#undef daClearP2
#undef daShift
#undef daUnshift
#undef daPush
#undef daPop
#undef daInsert
#undef daErase
#undef daSetSize
#undef daSetSizeP1
#undef daSetSizeP2
#undef daSize
#undef daSetCapacity
#undef daSetCapacityP1
#undef daSetCapacityP2
#undef daCapacity
#undef daSquash
#endif

#endif
