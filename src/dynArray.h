// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#ifndef DYNARRAY_H
#define DYNARRAY_H

#include "dynBase.h"

// creation / destruction / cleanup
void daCreate(void *daptr);
void daDestroy(void *daptr, void * /*dynDestroyFunc*/ destroyFunc);
void daDestroyP1(void *daptr, void * /*dynDestroyFuncP1*/ destroyFunc, void *p1);
void daDestroyP2(void *daptr, void * /*dynDestroyFuncP2*/ destroyFunc, void *p1, void *p2);
void daDestroyStrings(void *daptr);
void daClear(void *daptr, void * /*dynDestroyFunc*/ destroyFunc);
void daClearP1(void *daptr, void * /*dynDestroyFuncP1*/ destroyFunc, void *p1);
void daClearP2(void *daptr, void * /*dynDestroyFuncP2*/ destroyFunc, void *p1, void *p2);
void daClearStrings(void *daptr);

// front/back manipulation
void *daShift(void *daptr);
void daUnshift(void *daptr, void *p);
dynSize daPush(void *daptr, void *entry);
void *daPop(void *daptr);

// random access manipulation
void daInsert(void *daptr, dynSize index, void *p);
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

#endif
