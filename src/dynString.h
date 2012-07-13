// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#ifndef DYNAMIC_STRING_H
#define DYNAMIC_STRING_H

#include <stdarg.h>

#ifdef DYNAMIC_STRING_PREFIX
#define dsCreate      DYNAMIC_STRING_PREFIX ## Create
#define dsDestroy     DYNAMIC_STRING_PREFIX ## Destroy
#define dsClear       DYNAMIC_STRING_PREFIX ## Clear
#define dsCopyLen     DYNAMIC_STRING_PREFIX ## CopyLen
#define dsCopy        DYNAMIC_STRING_PREFIX ## Copy
#define dsConcatLen   DYNAMIC_STRING_PREFIX ## ConcatLen
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

#ifndef DYNAMIC_STRING_SIZE_TYPE
#define DYNAMIC_STRING_SIZE_TYPE int
#undef DYNAMIC_STRING_SIZE_FORMAT
#define DYNAMIC_STRING_SIZE_FORMAT "%d"
#endif
#ifndef DYNAMIC_STRING_SIZE_FORMAT
#error If you define DYNAMIC_STRING_SIZE_TYPE, you must also define DYNAMIC_STRING_SIZE_FORMAT.
#endif

// creation / destruction / cleanup
void dsCreate(char **dsptr);
void dsDestroy(char **dsptr);
void dsClear(char **dsptr);

// manipulation
void dsCopyLen(char **dsptr, const char *text, DYNAMIC_STRING_SIZE_TYPE len);
void dsCopy(char **dsptr, const char *text);
void dsConcatLen(char **dsptr, const char *text, DYNAMIC_STRING_SIZE_TYPE len);
void dsConcat(char **dsptr, const char *text);
void dsPrintf(char **dsptr, const char *format, ...);
void dsConcatv(char **dsptr, const char *format, va_list args);
void dsConcatf(char **dsptr, const char *format, ...);
void dsSetLength(char **dsptr, DYNAMIC_STRING_SIZE_TYPE newLength);
void dsCalcLength(char **dsptr);
void dsSetCapacity(char **dsptr, DYNAMIC_STRING_SIZE_TYPE newCapacity);

// information / testing
int dsCmp(char **dsptr, char **other);
DYNAMIC_STRING_SIZE_TYPE dsLength(char **dsptr);
DYNAMIC_STRING_SIZE_TYPE dsCapacity(char **dsptr);

// Undefine these to minimize name clashing with preexisting code
#ifdef DYNAMIC_STRING_PREFIX
#undef dsCreate
#undef dsDestroy
#undef dsClear
#undef dsCopyLen
#undef dsCopy
#undef dsConcatLen
#undef dsConcat
#undef dsPrintf
#undef dsConcatv
#undef dsConcatf
#undef dsSetLength
#undef dsCalcLength
#undef dsSetCapacity
#undef dsCmp
#undef dsLength
#undef dsCapacity
#endif

#endif
