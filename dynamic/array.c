#include "dynamic/array.h"

#include <stdlib.h>
#include <string.h>

#define dynArraySize DYNAMIC_ARRAY_SIZE_TYPE
#define DYNAMIC_ARRAY_INITIAL_SIZE 2

typedef struct dynArray
{
    char **entries;
    dynArraySize size;
    dynArraySize capacity;
} dynArray;

#define DYNAMIC_ARRAY_CALC_SIZE(newSize) \
    sizeof(dynArray) + (sizeof(char*) * newSize)

static dynArray *daChangeCapacity(dynArraySize newCapacity, char ***prevptr)
{
    dynArray *newArray = (dynArray *)calloc(1, DYNAMIC_ARRAY_CALC_SIZE(newCapacity));
    newArray->capacity = newCapacity;
    newArray->entries = (char **)(((char *)newArray) + sizeof(dynArray));
    if(prevptr)
    {
        if(*prevptr)
        {
            dynArray *prevArray = (dynArray *)((char *)(*prevptr) - sizeof(dynArray));
            int copyCount = prevArray->size;
            if(copyCount > newArray->capacity)
                copyCount = newArray->capacity;
            memcpy(newArray->entries, prevArray->entries, sizeof(char*) * copyCount);
            newArray->size = copyCount;
            free(prevArray);
        }
        *prevptr = newArray->entries;
    }
    return newArray;
}

static dynArray *daGet(char ***daptr)
{
    dynArray *da;
    if(daptr && *daptr)
    {
        // Move backwards one struct's worth (in bytes) to find the actual struct
        da = (dynArray *)((char *)(*daptr) - sizeof(dynArray));
    }
    else
    {
        // Create a new dynamic array
        da = daChangeCapacity(DYNAMIC_ARRAY_INITIAL_SIZE, daptr);
    }
    return da;
}

static dynArray *daMakeRoom(char ***daptr, int incomingCount)
{
    dynArray *da = daGet((char ***)daptr);
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

void daCreate(void *daptr)
{
    daGet(daptr);
}

void daClear(void *daptr, daDestroyFunc destroyFunc, void *userData)
{
    dynArray *da = daGet((char ***)daptr);
    if(destroyFunc)
    {
        int i;
        for(i=0; i<da->size; i++)
        {
            destroyFunc(userData, da->entries[i]);
        }
    }
}

void daDestroy(void *daptr, daDestroyFunc destroyFunc, void *userData)
{
    dynArray *da = daGet((char ***)daptr);
    daClear(daptr, destroyFunc, userData);
    free(da);
}

dynArraySize daSize(void *daptr)
{
    dynArray *da = daGet((char ***)daptr);
    return da->size;
}

dynArraySize daPush(void *daptr, void *entry)
{
    dynArray *da = daMakeRoom(daptr, 1);
    da->entries[da->size++] = entry;
    return da->size - 1;
}
