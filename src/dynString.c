// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#include "dynString.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#define va_copy(dest, src) ((void)((dest) = (src)))
#endif

// ------------------------------------------------------------------------------------------------
// Constants

// ------------------------------------------------------------------------------------------------
// Internal structures

typedef struct dynString
{
    rune *buffer;
    int length;
    int capacity;
} dynString;

// ------------------------------------------------------------------------------------------------
// Internal helper functions

// workhorse function that does all of the allocation and copying
static dynString *dsChangeCapacity(dynSize newCapacity, rune **prevptr)
{
    dynString *newString;
    dynString *prevString = NULL;
    if(prevptr && *prevptr)
    {
        prevString = (dynString *)((rune *)(*prevptr) - sizeof(dynString));
        if(newCapacity == prevString->capacity)
            return prevString;
    }

    newString = (dynString *)calloc(1, sizeof(dynString) + (sizeof(rune) * (newCapacity + 1)));
    newString->capacity = newCapacity;
    newString->buffer = ((rune *)newString) + sizeof(dynString);
    if(prevptr)
    {
        if(prevString)
        {
            int copyCount = prevString->length;
            if(copyCount > newString->capacity)
                copyCount = newString->capacity;
            memcpy(newString->buffer, prevString->buffer, sizeof(rune) * (copyCount + 1)); // + null terminator
            newString->length = copyCount;
            free(prevString);
        }
        *prevptr = newString->buffer;
    }
    return newString;
}

// finds / lazily creates a dynString from a regular ptr*
static dynString *dsGet(rune **dsptr, int autoCreate)
{
    dynString *ds = NULL;
    if(dsptr && *dsptr)
    {
        // Move backwards one struct's worth (in bytes) to find the actual struct
        ds = (dynString *)((rune *)(*dsptr) - sizeof(dynString));
    }
    else
    {
        if(autoCreate)
        {
            // Create a new dynamic array
            ds = dsChangeCapacity(0, dsptr);
        }
    }
    return ds;
}

// calls dsChangeCapacity in preparation for new dsta, if necessary
static dynString *dsMakeRoom(rune **dsptr, int len, int append)
{
    int currCapacity = dsCapacity(dsptr);
    int capacityNeeded = len;
    if(append)
        capacityNeeded += dsLength(dsptr);
    if(capacityNeeded > currCapacity)
    {
        return dsChangeCapacity(capacityNeeded, dsptr);
    }
    return dsGet(dsptr, 1);
}

static dynSize runelen(const rune *str)
{
    dynSize i = 0;
    while(*str)
    {
        ++i;
        ++str;
    }
    return i;
}

static int runecmp(const rune *a, const rune *b)
{
    unsigned char uc1, uc2;
    while (*a && (*a == *b))
    {
        ++a;
        ++b;
    }
    if(*a < *b)
    {
        return -1;
    }
    if(*a > *b)
    {
        return 1;
    }
    return 0;
}

// ------------------------------------------------------------------------------------------------
// creation / destruction / cleanup

void dsCreate(rune **dsptr)
{
    dsClear(dsptr);
}

void dsDestroy(rune **dsptr)
{
    dynString *ds = dsGet(dsptr, 0);
    if(ds)
    {
        dsClear(dsptr);
        free(ds);
        *dsptr = 0;
    }
}

void dsDestroyIndirect(rune *ds)
{
    rune *p = ds;
    dsDestroy(&p);
}

void dsClear(rune **dsptr)
{
    dynString *ds = dsGet(dsptr, 1);
    ds->buffer[0] = 0;
    ds->length = 0;
}

rune *dsDup(const rune *text)
{
    rune *dup = NULL;
    dsCopy(&dup, text);
    return dup;
}

rune *dsDupf(const rune *format, ...)
{
    rune *dup = NULL;
    va_list args;
    va_start(args, format);
    dsClear(&dup);
    dsConcatv(&dup, format, args);
    va_end(args);
    return dup;
}

// ------------------------------------------------------------------------------------------------
// manipulation

void dsCopyLen(rune **dsptr, const rune *text, dynSize len)
{
    dynString *ds = dsMakeRoom(dsptr, len, 0);
    memcpy(ds->buffer, text, sizeof(rune) * len);
    ds->length = len;
    ds->buffer[ds->length] = 0;
}

void dsCopy(rune **dsptr, const rune *text)
{
    dsCopyLen(dsptr, text, runelen(text));
}

void dsConcatLen(rune **dsptr, const rune *text, dynSize len)
{
    dynString *ds = dsMakeRoom(dsptr, len, 1);
    memcpy(ds->buffer + ds->length, text, sizeof(rune) * len);
    ds->length += len;
    ds->buffer[ds->length] = 0;
}

void dsConcat(rune **dsptr, const rune *text)
{
    dsConcatLen(dsptr, text, runelen(text));
}

#if 0
void dsPrintf(rune **dsptr, const rune *format, ...)
{
    va_list args;
    va_start(args, format);
    dsClear(dsptr);
    dsConcatv(dsptr, format, args);
    va_end(args);
}

void dsConcatv(rune **dsptr, const rune *format, va_list args)
{
    dynString *ds;
    int textLen;
    va_list argsCopy;
    va_copy(argsCopy, args);

    textLen = vsnprintf(NULL, 0, format, argsCopy);
    va_end(argsCopy);

    if(textLen == 0)
    {
        dsGet(dsptr, 1);
        return;
    }

    ds = dsMakeRoom(dsptr, textLen, 1);
    vsnprintf(ds->buffer + ds->length, textLen + 1, format, args);
    ds->length += textLen;
}

void dsConcatf(rune **dsptr, const rune *format, ...)
{
    va_list args;
    va_start(args, format);
    dsConcatv(dsptr, format, args);
    va_end(args);
}
#endif

void dsSetLength(rune **dsptr, dynSize newLength)
{
    dynString *ds;

    if(dsLength(dsptr) == newLength)
        return;

    if(newLength > dsCapacity(dsptr))
    {
        ds = dsChangeCapacity(newLength, dsptr);
    }
    else
    {
        ds = dsGet(dsptr, 1);
    }
    if(newLength > ds->length)
    {
        memset(ds->buffer + ds->length, ' ', sizeof(rune) * (newLength - ds->length));
    }
    ds->length = newLength;
    ds->buffer[ds->length] = 0;
}

void dsCalcLength(rune **dsptr)
{
    dynString *ds = dsGet(dsptr, 0);
    if(ds)
    {
        dsSetLength(dsptr, runelen(ds->buffer));
    }
}

void dsSetCapacity(rune **dsptr, dynSize newCapacity)
{
    dynString *ds = dsChangeCapacity(newCapacity, dsptr);
    ds->buffer[ds->length] = 0;
}

// ------------------------------------------------------------------------------------------------
// information / testing

int dsCmp(rune **dsptr, rune **other)
{
    const rune *s1 = *dsptr;
    const rune *s2 = *other;
    const rune emptyString[] = { 0 };
    if(!s1)
        s1 = emptyString;
    if(!s2)
        s2 = emptyString;

    return runecmp(s1, s2);
}

dynSize dsLength(rune **dsptr)
{
    dynString *ds = dsGet(dsptr, 0);
    if(ds)
    {
        return ds->length;
    }
    return 0;
}

dynSize dsCapacity(rune **dsptr)
{
    dynString *ds = dsGet(dsptr, 0);
    if(ds)
    {
        return ds->capacity;
    }
    return 0;
}
