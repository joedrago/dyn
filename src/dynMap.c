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
#define DYNAMIC_MAP_DEFAULT_WIDTH 7

// ------------------------------------------------------------------------------------------------
// Internal structures

// ------------------------------------------------------------------------------------------------
// Internal helper functions

static dynMapEntry *dmNewEntry(dynMap *dm, int tableIndex, void *value)
{
    dynMapEntry *entry;
    int found = 0;

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
    entry->value64 = 0;
    return entry;
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

dynMap *dmCreate(dmKeyType keyType, dynInt estimatedSize)
{
    dynInt tableWidth = DYNAMIC_MAP_DEFAULT_WIDTH; // TODO: use estimatedSize to choose this
    dynMap *dm = (dynMap *)calloc(1, sizeof(*dm));
    dm->keyType = keyType;
    dm->width = tableWidth;
    dm->table = calloc(dm->width, sizeof(dynMapEntry*));
    return dm;
}

void dmDestroy(dynMap *dm, dynDestroyFunc destroyFunc)
{
    if(dm)
    {
        dmClear(dm, destroyFunc);
        free(dm);
    }
}

static void dmDestroyEntry(dynMap *dm, dynMapEntry *p)
{
    if(dm->keyType == KEYTYPE_STRING)
        free(p->keyStr);
    free(p);
}

void dmClear(dynMap *dm, dynDestroyFunc destroyFunc)
{
    if(dm)
    {
        dynInt tableIndex;
        for(tableIndex = 0; tableIndex < dm->width; ++tableIndex)
        {
            dynMapEntry *entry = dm->table[tableIndex];
            while(entry)
            {
                dynMapEntry *freeme = entry;
                if(destroyFunc && entry->valuePtr)
                    destroyFunc(entry->valuePtr);
                entry = entry->next;
                dmDestroyEntry(dm, freeme);
            }
        }
        memset(dm->table, 0, dm->width * sizeof(dynMapEntry*));
    }
}

static dynMapEntry *dmFindString(dynMap *dm, const char *key, int autoCreate)
{
    dynMapHash hash = (dynMapHash)djb2hash(key);
    dynInt index = hash % dm->width;
    dynMapEntry *entry = dm->table[index];
    for( ; entry; entry = entry->next)
    {
        if(!strcmp(entry->keyStr, key))
            return entry;
    }

    if(autoCreate)
    {
        // A new entry!
        return dmNewEntry(dm, index, (void*)key);
    }
    return NULL;
}

static dynMapEntry *dmFindInteger(dynMap *dm, dynInt key, int autoCreate)
{
    dynMapHash hash = (dynMapHash)key;
    dynInt index = hash % dm->width;
    dynMapEntry *entry = dm->table[index];
    for( ; entry; entry = entry->next)
    {
        if(entry->keyInt == key)
            return entry;
    }

    if(autoCreate)
    {
        // A new entry!
        return dmNewEntry(dm, index, (void*)&key);
    }
    return NULL;
}

dynMapEntry *dmGetString(dynMap *dm, const char *key)
{
    return dmFindString(dm, key, 1);
}

int dmHasString(dynMap *dm, const char *key)
{
    return (dmFindString(dm, key, 0) != NULL);
}

void dmEraseString(dynMap *dm, const char *key, dynDestroyFunc destroyFunc)
{
    dynMapHash hash = (dynMapHash)djb2hash(key);
    dynInt index = hash % dm->width;
    dynMapEntry *prev = NULL;
    dynMapEntry *entry = dm->table[index];
    for( ; entry; prev = entry, entry = entry->next)
    {
        if(!strcmp(entry->keyStr, key))
        {
            if(prev)
            {
                prev->next = entry->next;
            }
            else
            {
                dm->table[index] = entry->next;
            }
            if(destroyFunc && entry->valuePtr)
                destroyFunc(entry->valuePtr);
            dmDestroyEntry(dm, entry);
            return;
        }
    }
}

dynMapEntry *dmGetInteger(dynMap *dm, dynInt key)
{
    return dmFindInteger(dm, key, 1);
}

int dmHasInteger(dynMap *dm, dynInt key)
{
    return (dmFindInteger(dm, key, 0) != NULL);
}

void dmEraseInteger(dynMap *dm, int key, dynDestroyFunc destroyFunc)
{
    dynMapHash hash = (dynMapHash)key;
    dynInt index = hash % dm->width;
    dynMapEntry *prev = NULL;
    dynMapEntry *entry = dm->table[index];
    for( ; entry; prev = entry, entry = entry->next)
    {
        if(entry->keyInt == key)
        {
            if(prev)
            {
                prev->next = entry->next;
            }
            else
            {
                dm->table[index] = entry->next;
            }
            if(destroyFunc && entry->valuePtr)
                destroyFunc(entry->valuePtr);
            dmDestroyEntry(dm, entry);
            return;
        }
    }
}
