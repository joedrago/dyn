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

#ifdef WIN32
#define va_copy(dest, src) ((void)((dest) = (src)))
#endif

// ------------------------------------------------------------------------------------------------
// Recreate these definition overrides, as they were unset at the end of the header

#ifdef DYNAMIC_STRING_PREFIX
#define dsCreate      DYNAMIC_STRING_PREFIX ## Create
#define dsDestroy     DYNAMIC_STRING_PREFIX ## Destroy
#define dsClear       DYNAMIC_STRING_PREFIX ## Clear
#define dsCopyLen     DYNAMIC_STRING_PREFIX ## CopyLen
#define dsCopy        DYNAMIC_STRING_PREFIX ## Copy
#define dsConcat      DYNAMIC_STRING_PREFIX ## Concat
#define dsPrintf      DYNAMIC_STRING_PREFIX ## Printf
#define dsConcatv     DYNAMIC_STRING_PREFIX ## Concatv
#define dsConcatf     DYNAMIC_STRING_PREFIX ## Concatf
#define dsSetLength   DYNAMIC_STRING_PREFIX ## SetLength
#define dsCalcLength  DYNAMIC_STRING_PREFIX ## CalcLength
#define dsSetCapacity DYNAMIC_STRING_PREFIX ## SetCapacity
#define dsCmp         DYNAMIC_STRING_PREFIX ## Cmp
#define dsLength      DYNAMIC_STRING_PREFIX ## Length
#define dsCapacity    DYNAMIC_STRING_PREFIX ## Capacity
#endif

// ------------------------------------------------------------------------------------------------
// Constants

// ------------------------------------------------------------------------------------------------
// Internal structures

typedef struct dynString
{
    char *buffer;
    int length;
    int capacity;
} dynString;

#define dynStringSize DYNAMIC_STRING_SIZE_TYPE

// ------------------------------------------------------------------------------------------------
// Internal helper functions

// workhorse function that does all of the allocation and copying
static dynString *dsChangeCapacity(dynStringSize newCapacity, char **prevptr)
{
    dynString *newString;
    dynString *prevString = NULL;
    if(prevptr && *prevptr)
    {
        prevString = (dynString *)((char *)(*prevptr) - sizeof(dynString));
        if(newCapacity == prevString->capacity)
            return prevString;
    }

    newString = (dynString *)calloc(1, sizeof(dynString) + (sizeof(char) * (newCapacity + 1)));
    newString->capacity = newCapacity;
    newString->buffer = ((char *)newString) + sizeof(dynString);
    if(prevptr)
    {
        if(prevString)
        {
            int copyCount = prevString->length;
            if(copyCount > newString->capacity)
                copyCount = newString->capacity;
            memcpy(newString->buffer, prevString->buffer, sizeof(char) * (copyCount + 1)); // + null terminator
            newString->length = copyCount;
            free(prevString);
        }
        *prevptr = newString->buffer;
    }
    return newString;
}

// finds / lazily creates a dynString from a regular ptr*
static dynString *dsGet(char **dsptr, int autoCreate)
{
    dynString *ds = NULL;
    if(dsptr && *dsptr)
    {
        // Move backwards one struct's worth (in bytes) to find the actual struct
        ds = (dynString *)((char *)(*dsptr) - sizeof(dynString));
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
static dynString *dsMakeRoom(char **dsptr, int len, int append)
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

// ------------------------------------------------------------------------------------------------
// creation / destruction / cleanup

void dsCreate(char **dsptr)
{
    dsClear(dsptr);
}

void dsDestroy(char **dsptr)
{
    dynString *ds = dsGet(dsptr, 0);
    if(ds)
    {
        dsClear(dsptr);
        free(ds);
        *dsptr = 0;
    }
}

void dsClear(char **dsptr)
{
    dynString *ds = dsGet(dsptr, 1);
    ds->buffer[0] = 0;
    ds->length = 0;
}

// ------------------------------------------------------------------------------------------------
// manipulation

void dsCopyLen(char **dsptr, const char *text, DYNAMIC_STRING_SIZE_TYPE len)
{
    dynString *ds = dsMakeRoom(dsptr, len, 0);
    memcpy(ds->buffer, text, len);
    ds->length = len;
    ds->buffer[ds->length] = 0;
}

void dsCopy(char **dsptr, const char *text)
{
    dsCopyLen(dsptr, text, (DYNAMIC_STRING_SIZE_TYPE)strlen(text));
}

void dsConcatLen(char **dsptr, const char *text, DYNAMIC_STRING_SIZE_TYPE len)
{
    dynString *ds = dsMakeRoom(dsptr, len, 1);
    memcpy(ds->buffer + ds->length, text, len);
    ds->length += len;
    ds->buffer[ds->length] = 0;
}

void dsConcat(char **dsptr, const char *text)
{
    dsConcatLen(dsptr, text, (DYNAMIC_STRING_SIZE_TYPE)strlen(text));
}

void dsPrintf(char **dsptr, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    dsClear(dsptr);
    dsConcatv(dsptr, format, args);
    va_end(args);
}

void dsConcatv(char **dsptr, const char *format, va_list args)
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

void dsConcatf(char **dsptr, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    dsConcatv(dsptr, format, args);
    va_end(args);
}

void dsSetLength(char **dsptr, DYNAMIC_STRING_SIZE_TYPE newLength)
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
        memset(ds->buffer + ds->length, ' ', sizeof(char) * (newLength - ds->length));
    }
    ds->length = newLength;
    ds->buffer[ds->length] = 0;
}

void dsCalcLength(char **dsptr)
{
    dynString *ds = dsGet(dsptr, 0);
    if(ds)
    {
        dsSetLength(dsptr, strlen(ds->buffer));
    }
}

void dsSetCapacity(char **dsptr, DYNAMIC_STRING_SIZE_TYPE newCapacity)
{
    dynString *ds = dsChangeCapacity(newCapacity, dsptr);
    ds->buffer[ds->length] = 0;
}

// ------------------------------------------------------------------------------------------------
// information / testing

int dsCmp(char **dsptr, char **other)
{
    const char *s1 = *dsptr;
    const char *s2 = *other;
    if(!s1)
        s1 = "";
    if(!s2)
        s2 = "";

    return strcmp(s1, s2);
}

DYNAMIC_STRING_SIZE_TYPE dsLength(char **dsptr)
{
    dynString *ds = dsGet(dsptr, 0);
    if(ds)
    {
        return ds->length;
    }
    return 0;
}

DYNAMIC_STRING_SIZE_TYPE dsCapacity(char **dsptr)
{
    dynString *ds = dsGet(dsptr, 0);
    if(ds)
    {
        return ds->capacity;
    }
    return 0;
}
