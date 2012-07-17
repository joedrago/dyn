// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#include "dynArray.h"

#include <stdlib.h>
#include <string.h>

// ------------------------------------------------------------------------------------------------
// Constants

#define DYNAMIC_ARRAY_INITIAL_SIZE 2

// ------------------------------------------------------------------------------------------------
// Internal structures

#define dynArraySize dynSize

typedef struct dynArray
{
    char **values;
    dynArraySize size;
    dynArraySize capacity;
} dynArray;

// ------------------------------------------------------------------------------------------------
// Internal helper functions

// workhorse function that does all of the allocation and copying
static dynArray *daChangeCapacity(dynArraySize newCapacity, char ***prevptr)
{
    dynArray *newArray;
    dynArray *prevArray = NULL;
    if(prevptr && *prevptr)
    {
        prevArray = (dynArray *)((char *)(*prevptr) - sizeof(dynArray));
        if(newCapacity == prevArray->capacity)
            return prevArray;
    }

    newArray = (dynArray *)calloc(1, sizeof(dynArray) + (sizeof(char*) * newCapacity));
    newArray->capacity = newCapacity;
    newArray->values = (char **)(((char *)newArray) + sizeof(dynArray));
    if(prevptr)
    {
        if(prevArray)
        {
            int copyCount = prevArray->size;
            if(copyCount > newArray->capacity)
                copyCount = newArray->capacity;
            memcpy(newArray->values, prevArray->values, sizeof(char*) * copyCount);
            newArray->size = copyCount;
            free(prevArray);
        }
        *prevptr = newArray->values;
    }
    return newArray;
}

// finds / lazily creates a dynArray from a regular ptr**
static dynArray *daGet(char ***daptr, int autoCreate)
{
    dynArray *da = NULL;
    if(daptr && *daptr)
    {
        // Move backwards one struct's worth (in bytes) to find the actual struct
        da = (dynArray *)((char *)(*daptr) - sizeof(dynArray));
    }
    else
    {
        if(autoCreate)
        {
            // Create a new dynamic array
            da = daChangeCapacity(DYNAMIC_ARRAY_INITIAL_SIZE, daptr);
        }
    }
    return da;
}

// this assumes you've already destroyed any soon-to-be orphaned values at the end
static void daChangeSize(char ***daptr, dynSize newSize)
{
    dynArray *da = daGet((char ***)daptr, 1);
    if(da->size == newSize)
        return;

    if(newSize > da->capacity)
    {
        da = daChangeCapacity(newSize, daptr);
    }
    if(newSize > da->size)
    {
        memset(da->values + da->size, 0, sizeof(char*) * (newSize - da->size));
    }
    da->size = newSize;
}

// calls daChangeCapacity in preparation for new data, if necessary
static dynArray *daMakeRoom(char ***daptr, int incomingCount)
{
    dynArray *da = daGet((char ***)daptr, 1);
    int capacityNeeded = da->size + incomingCount;
    int newCapacity = da->capacity;
    while(newCapacity < capacityNeeded)
        newCapacity *= 2; // is this dumb?
    if(newCapacity != da->capacity)
    {
        da = daChangeCapacity(newCapacity, daptr);
    }
    return da;
}

// clears [start, (end-1)]
static void daClearRange(dynArray *da, int start, int end, dynDestroyFunc destroyFunc)
{
    if(destroyFunc)
    {
        int i;
        for(i = start; i < end; ++i)
        {
            if(da->values[i])
                destroyFunc(da->values[i]);
        }
    }
}

static void daClearRangeP1(dynArray *da, int start, int end, dynDestroyFuncP1 destroyFunc, void *p1)
{
    if(destroyFunc)
    {
        int i;
        for(i = start; i < end; ++i)
        {
            if(da->values[i])
                destroyFunc(p1, da->values[i]);
        }
    }
}

static void daClearRangeP2(dynArray *da, int start, int end, dynDestroyFuncP2 destroyFunc, void *p1, void *p2)
{
    if(destroyFunc)
    {
        int i;
        for(i = start; i < end; ++i)
        {
            if(da->values[i])
                destroyFunc(p1, p2, da->values[i]);
        }
    }
}

// ------------------------------------------------------------------------------------------------
// creation / destruction / cleanup

void daCreate(void *daptr)
{
    daGet(daptr, 1);
}

void daDestroy(void *daptr, dynDestroyFunc destroyFunc)
{
    dynArray *da = daGet((char ***)daptr, 0);
    if(da)
    {
        daClear(daptr, destroyFunc);
        free(da);
        *((char ***)daptr) = NULL;
    }
}

void daDestroyP1(void *daptr, dynDestroyFuncP1 destroyFunc, void *p1)
{
    dynArray *da = daGet((char ***)daptr, 0);
    if(da)
    {
        daClearP1(daptr, destroyFunc, p1);
        free(da);
        *((char ***)daptr) = NULL;
    }
}

void daDestroyP2(void *daptr, dynDestroyFuncP2 destroyFunc, void *p1, void *p2)
{
    dynArray *da = daGet((char ***)daptr, 0);
    if(da)
    {
        daClearP2(daptr, destroyFunc, p1, p2);
        free(da);
        *((char ***)daptr) = NULL;
    }
}

void daClear(void *daptr, dynDestroyFunc destroyFunc)
{
    dynArray *da = daGet((char ***)daptr, 0);
    if(da)
    {
        daClearRange(da, 0, da->size, destroyFunc);
        da->size = 0;
    }
}

void daClearP1(void *daptr, dynDestroyFuncP1 destroyFunc, void *p1)
{
    dynArray *da = daGet((char ***)daptr, 0);
    if(da)
    {
        daClearRangeP1(da, 0, da->size, destroyFunc, p1);
        da->size = 0;
    }
}

void daClearP2(void *daptr, dynDestroyFuncP2 destroyFunc, void *p1, void *p2)
{
    dynArray *da = daGet((char ***)daptr, 0);
    if(da)
    {
        daClearRangeP2(da, 0, da->size, destroyFunc, p1, p2);
        da->size = 0;
    }
}

// ------------------------------------------------------------------------------------------------
// Front/back manipulation

// aka "pop front"
void *daShift(void *daptr)
{
    dynArray *da = daGet((char ***)daptr, 0);
    if(da && da->size > 0)
    {
        void *ret = da->values[0];
        memmove(da->values, da->values + 1, sizeof(char*) * da->size);
        --da->size;
        return ret;
    }
    return NULL;
}

void daUnshift(void *daptr, void *p)
{
    dynArray *da = daMakeRoom(daptr, 1);
    if(da->size > 0)
    {
        memmove(da->values + 1, da->values, sizeof(char*) * da->size);
    }
    da->values[0] = p;
    da->size++;
}

dynSize daPush(void *daptr, void *entry)
{
    dynArray *da = daMakeRoom(daptr, 1);
    da->values[da->size++] = entry;
    return da->size - 1;
}

void *daPop(void *daptr)
{
    dynArray *da = daGet((char ***)daptr, 0);
    if(da && (da->size > 0))
    {
        return da->values[--da->size];
    }
    return NULL;
}

// ------------------------------------------------------------------------------------------------
// Random access manipulation

void daInsert(void *daptr, dynSize index, void *p)
{
    dynArray *da = daMakeRoom(daptr, 1);
    if((index < 0) || (!da->size) || (index >= da->size))
    {
        daPush(daptr, p);
    }
    else
    {
        memmove(da->values + index + 1, da->values + index, sizeof(char*) * (da->size - index));
        da->values[index] = p;
        ++da->size;
    }
}

void daErase(void *daptr, dynSize index)
{
    dynArray *da = daGet((char ***)daptr, 0);
    if(!da)
        return;
    if((index < 0) || (!da->size) || (index >= da->size))
        return;

    memmove(da->values + index, da->values + index + 1, sizeof(char*) * (da->size - index));
    --da->size;
}

// ------------------------------------------------------------------------------------------------
// Size manipulation

void daSetSize(void *daptr, dynSize newSize, dynDestroyFunc destroyFunc)
{
    dynArray *da = daGet((char ***)daptr, 1);
    daClearRange(da, newSize, da->size, destroyFunc);
    daChangeSize(daptr, newSize);
}

void daSetSizeP1(void *daptr, dynSize newSize, dynDestroyFuncP1 destroyFunc, void *p1)
{
    dynArray *da = daGet((char ***)daptr, 1);
    daClearRangeP1(da, newSize, da->size, destroyFunc, p1);
    daChangeSize(daptr, newSize);
}

void daSetSizeP2(void *daptr, dynSize newSize, dynDestroyFuncP2 destroyFunc, void *p1, void *p2)
{
    dynArray *da = daGet((char ***)daptr, 1);
    daClearRangeP2(da, newSize, da->size, destroyFunc, p1, p2);
    daChangeSize(daptr, newSize);
}

dynSize daSize(void *daptr)
{
    dynArray *da = daGet((char ***)daptr, 0);
    if(da)
        return da->size;
    return 0;
}

void daSetCapacity(void *daptr, dynSize newCapacity, dynDestroyFunc destroyFunc)
{
    dynArray *da = daGet((char ***)daptr, 1);
    daClearRange(da, newCapacity, da->size, destroyFunc);
    daChangeCapacity(newCapacity, daptr);
}

void daSetCapacityP1(void *daptr, dynSize newCapacity, dynDestroyFuncP1 destroyFunc, void *p1)
{
    dynArray *da = daGet((char ***)daptr, 1);
    daClearRangeP1(da, newCapacity, da->size, destroyFunc, p1);
    daChangeCapacity(newCapacity, daptr);
}

void daSetCapacityP2(void *daptr, dynSize newCapacity, dynDestroyFuncP2 destroyFunc, void *p1, void *p2)
{
    dynArray *da = daGet((char ***)daptr, 1);
    daClearRangeP2(da, newCapacity, da->size, destroyFunc, p1, p2);
    daChangeCapacity(newCapacity, daptr);
}

dynSize daCapacity(void *daptr)
{
    dynArray *da = daGet((char ***)daptr, 0);
    if(da)
        return da->capacity;
    return 0;
}

void daSquash(void *daptr)
{
    dynArray *da = daGet((char ***)daptr, 0);
    if(da)
    {
        int head = 0;
        int tail = 0;
        for( ; tail < da->size ; tail++)
        {
            if(da->values[tail] != NULL)
            {
                da->values[head] = da->values[tail];
                head++;
            }
        }
        da->size = head;
    }
}
