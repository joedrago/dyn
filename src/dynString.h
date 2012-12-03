// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#ifndef DYNAMIC_STRING_H
#define DYNAMIC_STRING_H

#include "dynBase.h"

#include <stdarg.h>

typedef unsigned int rune;

// creation / destruction / cleanup
void dsCreate(rune **dsptr);
void dsDestroy(rune **dsptr);
void dsDestroyIndirect(rune *ds);
void dsClear(rune **dsptr);
rune *dsDup(const rune *text);
rune *dsDupf(const rune *format, ...);

// manipulation
void dsCopyLen(rune **dsptr, const rune *text, dynSize len);
void dsCopy(rune **dsptr, const rune *text);
void dsConcatLen(rune **dsptr, const rune *text, dynSize len);
void dsConcat(rune **dsptr, const rune *text);
void dsPrintf(rune **dsptr, const rune *format, ...);
void dsConcatv(rune **dsptr, const rune *format, va_list args);
void dsConcatf(rune **dsptr, const rune *format, ...);
void dsSetLength(rune **dsptr, dynSize newLength);
void dsCalcLength(rune **dsptr);
void dsSetCapacity(rune **dsptr, dynSize newCapacity);

// information / testing
int dsCmp(rune **dsptr, rune **other);
dynSize dsLength(rune **dsptr);
dynSize dsCapacity(rune **dsptr);

#endif
