// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#include "dyn.h"

#include <stdlib.h>
#include <string.h>

// ------------------------------------------------------------------------------------------------
// Constants and Macros

#define DYNAMIC_ARRAY_INITIAL_SIZE 2

#define dynArrayToValues(da) (char **)(((char *)da) + sizeof(dynArray))
#define dynValuesToArray(daptr) (dynArray *)((char *)(*daptr) - sizeof(dynArray))

// ------------------------------------------------------------------------------------------------
// Internal structures

typedef struct dynArray
{
    dynSize size;
    dynSize capacity;
    dynSize elementSize;
} dynArray;

// ------------------------------------------------------------------------------------------------
// Internal helper functions

// workhorse function that does all of the allocation and copying
static dynArray *daChangeCapacity(dynSize newCapacity, dynSize elementSize, char ***prevptr)
{
    dynArray *newArray;
    dynArray *prevArray = NULL;
    char **newValues;
    if(prevptr && *prevptr)
    {
        prevArray = dynValuesToArray(prevptr);
        elementSize = prevArray->elementSize;
        if(newCapacity == prevArray->capacity)
            return prevArray;
    }

    if(elementSize == 0)
    {
        // If nobody calls daCreate(), assume they are making an array of pointers
        elementSize = sizeof(char*);
    }

    newArray = (dynArray *)calloc(1, sizeof(dynArray) + (elementSize * newCapacity));
    newArray->elementSize = elementSize;
    newArray->capacity = newCapacity;
    newValues = dynArrayToValues(newArray);
    if(prevptr)
    {
        if(prevArray)
        {
            char **prevValues = dynArrayToValues(prevArray);
            int copyCount = prevArray->size;
            if(copyCount > newArray->capacity)
                copyCount = newArray->capacity;
            memcpy(newValues, prevValues, sizeof(char*) * copyCount);
            newArray->size = copyCount;
            free(prevArray);
        }
        *prevptr = newValues;
    }
    return newArray;
}

// finds / lazily creates a dynArray from a regular ptr**
static dynArray *daGet(char ***daptr, dynSize elementSize, int autoCreate)
{
    dynArray *da = NULL;
    if(daptr && *daptr)
    {
        // Move backwards one struct's worth (in bytes) to find the actual struct
        da = dynValuesToArray(daptr);
    }
    else
    {
        if(autoCreate)
        {
            // Create a new dynamic array
            da = daChangeCapacity(DYNAMIC_ARRAY_INITIAL_SIZE, elementSize, daptr);
        }
    }
    return da;
}

// this assumes you've already destroyed any soon-to-be orphaned values at the end
static void daChangeSize(char ***daptr, dynSize newSize)
{
    dynArray *da = daGet((char ***)daptr, 0, 1);
    if(da->size == newSize)
        return;

    if(newSize > da->capacity)
    {
        da = daChangeCapacity(newSize, 0, daptr);
    }
    if(newSize > da->size)
    {
        char **values = dynArrayToValues(da);
        memset(values + da->size, 0, sizeof(char*) * (newSize - da->size));
    }
    da->size = newSize;
}

// calls daChangeCapacity in preparation for new data, if necessary
static dynArray *daMakeRoom(char ***daptr, int incomingCount)
{
    dynArray *da = daGet((char ***)daptr, 0, 1);
    int capacityNeeded = da->size + incomingCount;
    int newCapacity = da->capacity;
    while(newCapacity < capacityNeeded)
        newCapacity *= 2; // is this dumb?
    if(newCapacity != da->capacity)
    {
        da = daChangeCapacity(newCapacity, 0, daptr);
    }
    return da;
}

// clears [start, (end-1)]
static void daClearRange(dynArray *da, int start, int end, void * destroyFunc)
{
    dynDestroyFunc func = destroyFunc;
    if(func)
    {
        int i;
        char **values = dynArrayToValues(da);
        for(i = start; i < end; ++i)
        {
            func(values[i]);
        }
    }
}

static void daClearRangeP1(dynArray *da, int start, int end, void * destroyFunc, void *p1)
{
    dynDestroyFuncP1 func = destroyFunc;
    if(func)
    {
        int i;
        char **values = dynArrayToValues(da);
        for(i = start; i < end; ++i)
        {
            func(p1, values[i]);
        }
    }
}

static void daClearRangeP2(dynArray *da, int start, int end, void * destroyFunc, void *p1, void *p2)
{
    dynDestroyFuncP2 func = destroyFunc;
    if(func)
    {
        int i;
        char **values = dynArrayToValues(da);
        for(i = start; i < end; ++i)
        {
            func(p1, p2, values[i]);
        }
    }
}

// ------------------------------------------------------------------------------------------------
// creation / destruction / cleanup

void daCreate(void *daptr, dynSize elementSize)
{
    daGet(daptr, elementSize, 1);
}

void daDestroy(void *daptr, void * destroyFunc)
{
    dynArray *da = daGet((char ***)daptr, 0, 0);
    if(da)
    {
        daClear(daptr, destroyFunc);
        free(da);
        *((char ***)daptr) = NULL;
    }
}

void daDestroyStrings(void *daptr)
{
    daDestroy(daptr, dsDestroyIndirect);
}

void daDestroyP1(void *daptr, void * destroyFunc, void *p1)
{
    dynArray *da = daGet((char ***)daptr, 0, 0);
    if(da)
    {
        daClearP1(daptr, destroyFunc, p1);
        free(da);
        *((char ***)daptr) = NULL;
    }
}

void daDestroyP2(void *daptr, void * destroyFunc, void *p1, void *p2)
{
    dynArray *da = daGet((char ***)daptr, 0, 0);
    if(da)
    {
        daClearP2(daptr, destroyFunc, p1, p2);
        free(da);
        *((char ***)daptr) = NULL;
    }
}

void daClear(void *daptr, void * destroyFunc)
{
    dynArray *da = daGet((char ***)daptr, 0, 0);
    if(da)
    {
        daClearRange(da, 0, da->size, destroyFunc);
        da->size = 0;
    }
}

void daClearP1(void *daptr, void * destroyFunc, void *p1)
{
    dynArray *da = daGet((char ***)daptr, 0, 0);
    if(da)
    {
        daClearRangeP1(da, 0, da->size, destroyFunc, p1);
        da->size = 0;
    }
}

void daClearP2(void *daptr, void * destroyFunc, void *p1, void *p2)
{
    dynArray *da = daGet((char ***)daptr, 0, 0);
    if(da)
    {
        daClearRangeP2(da, 0, da->size, destroyFunc, p1, p2);
        da->size = 0;
    }
}

void daClearStrings(void *daptr)
{
    daClear(daptr, dsDestroyIndirect);
}

// ------------------------------------------------------------------------------------------------
// Front/back manipulation

// aka "pop front"
void *daShift(void *daptr)
{
    dynArray *da = daGet((char ***)daptr, 0, 0);
    if(da && da->size > 0)
    {
        char **values = dynArrayToValues(da);
        void *ret = values[0];
        memmove(values, values + 1, sizeof(char*) * da->size);
        --da->size;
        return ret;
    }
    return NULL;
}

void daUnshift(void *daptr, void *p)
{
    dynArray *da = daMakeRoom(daptr, 1);
    char **values = dynArrayToValues(da);
    if(da->size > 0)
    {
        memmove(values + 1, values, sizeof(char*) * da->size);
    }
    values[0] = p;
    da->size++;
}

dynSize daPush(void *daptr, void *entry)
{
    dynArray *da = daMakeRoom(daptr, 1);
    char **values = dynArrayToValues(da);
    values[da->size++] = entry;
    return da->size - 1;
}

void *daPop(void *daptr)
{
    dynArray *da = daGet((char ***)daptr, 0, 0);
    if(da && (da->size > 0))
    {
        char **values = dynArrayToValues(da);
        return values[--da->size];
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
        char **values = dynArrayToValues(da);
        memmove(values + index + 1, values + index, sizeof(char*) * (da->size - index));
        values[index] = p;
        ++da->size;
    }
}

void daErase(void *daptr, dynSize index)
{
    dynArray *da = daGet((char ***)daptr, 0, 0);
    char **values = dynArrayToValues(da);
    if(!da)
        return;
    if((index < 0) || (!da->size) || (index >= da->size))
        return;

    memmove(values + index, values + index + 1, sizeof(char*) * (da->size - index));
    --da->size;
}

// ------------------------------------------------------------------------------------------------
// Size manipulation

void daSetSize(void *daptr, dynSize newSize, void * destroyFunc)
{
    dynArray *da = daGet((char ***)daptr, 0, 1);
    daClearRange(da, newSize, da->size, destroyFunc);
    daChangeSize(daptr, newSize);
}

void daSetSizeP1(void *daptr, dynSize newSize, void * destroyFunc, void *p1)
{
    dynArray *da = daGet((char ***)daptr, 0, 1);
    daClearRangeP1(da, newSize, da->size, destroyFunc, p1);
    daChangeSize(daptr, newSize);
}

void daSetSizeP2(void *daptr, dynSize newSize, void * destroyFunc, void *p1, void *p2)
{
    dynArray *da = daGet((char ***)daptr, 0, 1);
    daClearRangeP2(da, newSize, da->size, destroyFunc, p1, p2);
    daChangeSize(daptr, newSize);
}

dynSize daSize(void *daptr)
{
    dynArray *da = daGet((char ***)daptr, 0, 0);
    if(da)
        return da->size;
    return 0;
}

void daSetCapacity(void *daptr, dynSize newCapacity, void * destroyFunc)
{
    dynArray *da = daGet((char ***)daptr, 0, 1);
    daClearRange(da, newCapacity, da->size, destroyFunc);
    daChangeCapacity(newCapacity, 0, daptr);
}

void daSetCapacityP1(void *daptr, dynSize newCapacity, void * destroyFunc, void *p1)
{
    dynArray *da = daGet((char ***)daptr, 0, 1);
    daClearRangeP1(da, newCapacity, da->size, destroyFunc, p1);
    daChangeCapacity(newCapacity, 0, daptr);
}

void daSetCapacityP2(void *daptr, dynSize newCapacity, void * destroyFunc, void *p1, void *p2)
{
    dynArray *da = daGet((char ***)daptr, 0, 1);
    daClearRangeP2(da, newCapacity, da->size, destroyFunc, p1, p2);
    daChangeCapacity(newCapacity, 0, daptr);
}

dynSize daCapacity(void *daptr)
{
    dynArray *da = daGet((char ***)daptr, 0, 0);
    if(da)
        return da->capacity;
    return 0;
}

void daSquash(void *daptr)
{
    dynArray *da = daGet((char ***)daptr, 0, 0);
    if(da)
    {
        int head = 0;
        int tail = 0;
        char **values = dynArrayToValues(da);
        for( ; tail < da->size ; tail++)
        {
            if(values[tail] != NULL)
            {
                values[head] = values[tail];
                head++;
            }
        }
        da->size = head;
    }
}
