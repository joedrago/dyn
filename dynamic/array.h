// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#ifdef DYNAMIC_ARRAY_PREFIX
#define daCreate      DYNAMIC_ARRAY_PREFIX ## Create
#define daDestroy     DYNAMIC_ARRAY_PREFIX ## Destroy
#define daDestroyFunc DYNAMIC_ARRAY_PREFIX ## DestroyFunc
#define daClear       DYNAMIC_ARRAY_PREFIX ## Clear
#define daShift       DYNAMIC_ARRAY_PREFIX ## Shift
#define daUnshift     DYNAMIC_ARRAY_PREFIX ## Unshift
#define daPush        DYNAMIC_ARRAY_PREFIX ## Push
#define daPop         DYNAMIC_ARRAY_PREFIX ## Pop
#define daInsert      DYNAMIC_ARRAY_PREFIX ## Insert
#define daErase       DYNAMIC_ARRAY_PREFIX ## Erase
#define daSetSize     DYNAMIC_ARRAY_PREFIX ## SetSize
#define daSize        DYNAMIC_ARRAY_PREFIX ## Size
#define daSetCapacity DYNAMIC_ARRAY_PREFIX ## SetCapacity
#define daCapacity    DYNAMIC_ARRAY_PREFIX ## Capacity
#define daSquash      DYNAMIC_ARRAY_PREFIX ## Squash
#define daCreateFn    DYNAMIC_ARRAY_PREFIX ## CreateFn
#endif

#ifndef DYNAMIC_ARRAY_SIZE_TYPE
#define DYNAMIC_ARRAY_SIZE_TYPE int
#endif

typedef (*daDestroyFunc)(void *userData, void *p);

void daCreate(void *daptr);
void daDestroy(void *daptr, daDestroyFunc destroyFunc, void *userData);
void daClear(void *daptr, daDestroyFunc destroyFunc, void *userData);
void daShift();
void daUnshift();
DYNAMIC_ARRAY_SIZE_TYPE daPush(void *daptr, void *entry);
void daPop();
void daInsert();
void daErase();
void daSetSize();
DYNAMIC_ARRAY_SIZE_TYPE daSize(void *daptr);
void daSetCapacity();
void daCapacity();
void daSquash();

#endif
