// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#include "dynArray.h"
#include "dynMap.h"

#include <stdlib.h>
#include <string.h>

// ------------------------------------------------------------------------------------------------
// Constants

#define DYNAMIC_MAP_INITIAL_SIZE 2

// ------------------------------------------------------------------------------------------------
// Internal structures

#define dynMapSize DYNAMIC_MAP_SIZE_TYPE

typedef struct dynMapEntry
{
    union
    {
        char *keyStr;
        DYNAMIC_MAP_SIZE_TYPE keyInt;
    };
    DYNAMIC_MAP_SIZE_TYPE index;
    struct dynMapEntry *next;
} dynMapEntry;

typedef struct dynMap
{
    char *values;
    char *used;
    dynMapEntry **table;
    int keyType;
    dynMapSize width;
    dynMapSize valueSize;
    dynMapSize capacity;
    dynMapSize count;
} dynMap;

// ------------------------------------------------------------------------------------------------
// Internal helper functions

// workhorse function that does all of the allocation and copying
static dynMap *dmChangeCapacity(char **prevptr, dynMapSize newCapacity, dynMapSize valueSize, dynMapSize width)
{
    dynMap *newMap;
    dynMap *prevMap = NULL;
    if(prevptr && *prevptr)
    {
        prevMap = (dynMap *)((char *)(*prevptr) - sizeof(dynMap));
        if(newCapacity == prevMap->capacity)
            return prevMap;
        valueSize = prevMap->valueSize;
    }

    newMap = (dynMap *)calloc(1, sizeof(dynMap) + (valueSize * newCapacity) + (sizeof(char) * newCapacity));
    newMap->capacity = newCapacity;
    newMap->valueSize = valueSize;
    newMap->values = (char *)(((char *)newMap) + sizeof(dynMap));
    newMap->used   = (char *)(((char *)newMap) + sizeof(dynMap) + (valueSize * newCapacity));
    if(prevptr)
    {
        if(prevMap)
        {
            int copyCount = prevMap->capacity;
            if(copyCount > newMap->capacity)
                copyCount = newMap->capacity;
            memcpy(newMap->values, prevMap->values, valueSize * copyCount);
            memcpy(newMap->used, prevMap->used, sizeof(char) * copyCount);
            newMap->width = prevMap->width;
            newMap->table = prevMap->table;
            newMap->count = prevMap->count;
            newMap->keyType = prevMap->keyType;
            free(prevMap);
        }
        else
        {
            // create a new table
            newMap->width = DYNAMIC_MAP_DEFAULT_WIDTH;
            newMap->table = calloc(newMap->width, sizeof(dynMapEntry*));
            newMap->keyType = DYNAMIC_MAP_DEFAULT_KEYTYPE;
        }
        *prevptr = newMap->values;
    }
    return newMap;
}

// finds / lazily creates a dynMap from a regular ptr**
static dynMap *dmGetPtr(char **dmptr, int autoCreate)
{
    dynMap *dm = NULL;
    if(dmptr && *dmptr)
    {
        // Move backwards one struct's worth (in bytes) to find the actual struct
        dm = (dynMap *)((char *)(*dmptr) - sizeof(dynMap));
    }
    else
    {
        if(autoCreate)
        {
            // Create a new map
            dm = dmChangeCapacity(dmptr, DYNAMIC_MAP_INITIAL_SIZE, DYNAMIC_MAP_DEFAULT_VALUE_SIZE, DYNAMIC_MAP_DEFAULT_WIDTH);
        }
    }
    return dm;
}

DYNAMIC_MAP_SIZE_TYPE dmNewIndex(void *dmptr, int tableIndex, void *value)
{
    DYNAMIC_MAP_SIZE_TYPE i;
    dynMapEntry *entry;
    dynMap *dm = dmGetPtr((char **)dmptr, 1);
    int found = 0;
    for(i = 0; i < dm->capacity; i++)
    {
        if(!dm->used[i])
        {
            dm->used[i] = 1;
            found = 1;
            break;
        }
    }
    if(!found)
    {
        i = dm->capacity;
        dm = dmChangeCapacity(dmptr, dm->capacity * 2, dm->valueSize, dm->width);
    }

    dm->count++;
    entry = (dynMapEntry *)calloc(1, sizeof(*entry));
    switch(dm->keyType)
    {
    case KEYTYPE_STRING:
        entry->keyStr = strdup((char *)value);
        break;
    case KEYTYPE_INTEGER:
        entry->keyInt = *((int *)value);
        break;
    }
    entry->next = dm->table[tableIndex];
    dm->table[tableIndex] = entry;
    entry->index = i;
    dm = dmGetPtr((char **)dmptr, 0);
    memset(dm->values + (dm->valueSize * entry->index), 0, dm->valueSize);
    return entry->index;
}

static unsigned int djb2hash(const unsigned char *str)
{
    unsigned int hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

// ------------------------------------------------------------------------------------------------
// creation / destruction / cleanup

void dmCreate(void *dmptr, dmKeyType keyType, DYNAMIC_MAP_SIZE_TYPE valueSize, DYNAMIC_MAP_SIZE_TYPE tableWidth)
{
    dynMap *dm;
    dmDestroy(dmptr, NULL);
    dm = dmChangeCapacity(dmptr, DYNAMIC_MAP_INITIAL_SIZE, valueSize, tableWidth);
    dm->keyType = keyType;
}

void dmDestroy(void *dmptr, dmDestroyFunc destroyFunc)
{
    dynMap *dm = dmGetPtr((char **)dmptr, 0);
    if(dm)
    {
        dmClear(dmptr, destroyFunc);
        free(dm);
        *((char **)dmptr) = NULL;
    }
}

static void dmDestroyEntry(dynMap *dm, dynMapEntry *p)
{
    if(dm->keyType == KEYTYPE_STRING)
        free(p->keyStr);
    free(p);
}

void dmClear(void *dmptr, dmDestroyFunc destroyFunc)
{
    dynMap *dm = dmGetPtr((char **)dmptr, 0);
    if(dm)
    {
        DYNAMIC_MAP_SIZE_TYPE tableIndex;
        for(tableIndex = 0; tableIndex < dm->width; ++tableIndex)
        {
            dynMapEntry *entry = dm->table[tableIndex];
            while(entry)
            {
                dynMapEntry *freeme = entry;
                char **value = (char**)(dm->values + (entry->index * dm->valueSize));
                if(destroyFunc && *value)
                    destroyFunc(*value);
                entry = entry->next;
                dmDestroyEntry(dm, freeme);
            }
        }
        memset(dm->table, 0, dm->width * sizeof(dynMapEntry*));
        memset(dm->used, 0, dm->capacity * sizeof(char));
    }
}

DYNAMIC_MAP_SIZE_TYPE dmStringIndex(void *dmptr, const char *str)
{
    dynMap *dm = dmGetPtr((char **)dmptr, 1);
    DYNAMIC_MAP_HASH_TYPE hash = (DYNAMIC_MAP_HASH_TYPE)djb2hash(str);
    DYNAMIC_MAP_SIZE_TYPE index = hash % dm->width;
    dynMapEntry *entry = dm->table[index];
    for( ; entry; entry = entry->next)
    {
        if(!strcmp(entry->keyStr, str))
            return entry->index;
    }

    // A new entry!
    return dmNewIndex(dmptr, index, (void*)str);
}

DYNAMIC_MAP_SIZE_TYPE dmIntegerIndex(void *dmptr, int key)
{
    dynMap *dm = dmGetPtr((char **)dmptr, 1);
    DYNAMIC_MAP_HASH_TYPE hash = (DYNAMIC_MAP_HASH_TYPE)key;
    DYNAMIC_MAP_SIZE_TYPE index = hash % dm->width;
    dynMapEntry *entry = dm->table[index];
    for( ; entry; entry = entry->next)
    {
        if(entry->keyInt == key)
            return entry->index;
    }

    // A new entry!
    return dmNewIndex(dmptr, index, (void*)&key);
}

int dmHasStringIndex(void *dmptr, const char *str)
{
    dynMap *dm = dmGetPtr((char **)dmptr, 1);
    DYNAMIC_MAP_HASH_TYPE hash = (DYNAMIC_MAP_HASH_TYPE)djb2hash(str);
    DYNAMIC_MAP_SIZE_TYPE index = hash % dm->width;
    dynMapEntry *entry = dm->table[index];
    for( ; entry; entry = entry->next)
    {
        if(!strcmp(entry->keyStr, str))
            return 1;
    }
    return 0;
}
