// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

// Additional copyright notification: This is a loose re-imagining of Dave
// Gamble's cJSON parser, using dyn constructs. As his was released under the
// MIT license and mine under Boost, all things licensing should be pretty
// friendly. I have left his original copyright notice intact at the bottom of
// this file, regardless of of how far this implementation eventually strays
// from the original source.

#include "dyn.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------

dynJSON *djCreate(int type)
{
    dynJSON *dj = (dynJSON *)calloc(1, sizeof(dynJSON));
    dj->type = type;
    return dj;
}

void djDestroy(dynJSON *dj)
{
    free(dj);
}

// ---------------------------------------------------------------------------

typedef struct dynJSONParser
{
    const char *text;
    char *error;
} dynJSONParser;

static void djParserClear(dynJSONParser *parser)
{
    parser->text = NULL;
    dsDestroy(&parser->error);
}

static dynJSON *djParseString(dynJSONParser *p)
{
    int escaped = 0;
    const char *ptr = p->text + 1;
    const char *src;
    char *ptr2;
    char *out;
    int len = 0;
    unsigned uc, uc2;

    if(*p->text != '\"')
    {
        // ERROR: expected string
        return NULL;
    }

    // Estimate length
    for(src = p->text + 1, len = 0, escaped = 0; *src && (*src != '\"'); ++src)
    {
        if(escaped || (p))
        {
            ++len;
        }
        escaped = 0;
    }

    out = (char *)cJSON_malloc(len + 1); /* This is how long we need for the string, roughly. */
    if(!out) { return 0; }

    ptr = str + 1; ptr2 = out;
    while(*ptr != '\"' && *ptr)
    {
        if(*ptr != '\\') { *ptr2++ = *ptr++; }
        else
        {
            ptr++;
            switch(*ptr)
            {
                case 'b': *ptr2++ = '\b'; break;
                case 'f': *ptr2++ = '\f'; break;
                case 'n': *ptr2++ = '\n'; break;
                case 'r': *ptr2++ = '\r'; break;
                case 't': *ptr2++ = '\t'; break;
                case 'u':    /* transcode utf16 to utf8. */
                    sscanf(ptr + 1, "%4x", &uc); ptr += 4; /* get the unicode char. */

                    if((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0) { break; } // check for invalid.

                    if(uc >= 0xD800 && uc <= 0xDBFF) // UTF16 surrogate pairs.
                    {
                        if(ptr[1] != '\\' || ptr[2] != 'u') { break; } // missing second-half of surrogate.
                        sscanf(ptr + 3, "%4x", &uc2); ptr += 6;
                        if(uc2 < 0xDC00 || uc2 > 0xDFFF) { break; } // invalid second-half of surrogate.
                        uc = 0x10000 | ((uc & 0x3FF) << 10) | (uc2 & 0x3FF);
                    }

                    len = 4; if(uc < 0x80) { len = 1; }
                    else if(uc < 0x800) { len = 2; }
                    else if(uc < 0x10000) { len = 3; } ptr2 += len;

                    switch(len)
                    {
                        case 4: *--ptr2 = ((uc | 0x80) & 0xBF); uc >>= 6;
                        case 3: *--ptr2 = ((uc | 0x80) & 0xBF); uc >>= 6;
                        case 2: *--ptr2 = ((uc | 0x80) & 0xBF); uc >>= 6;
                        case 1: *--ptr2 = (uc | firstByteMark[len]);
                    }
                    ptr2 += len;
                    break;
                default:  *ptr2++ = *ptr; break;
            }
            ptr++;
        }
    }
    *ptr2 = 0;
    if(*ptr == '\"') { ptr++; }
    item->valuestring = out;
    item->type = cJSON_String;
    return ptr;
}

static dynJSON *djParseValue(dynJSONParser *p)
{
    dynJSON *dj = NULL;
    if(p->text == NULL)
    {
        return NULL;
    }
    else if(!strncmp(p->text, "null", 4))
    {
        dj = djCreate(DJT_NULL);
        p->text += 4;
    }
    else if(!strncmp(p->text, "false", 5))
    {
        dj = djCreate(DJT_BOOL);
        dj->intVal = 0;
        p->text += 4;
    }
    else if(!strncmp(p->text, "true", 4))
    {
        dj = djCreate(DJT_BOOL);
        dj->intVal = 1;
        p->text += 4;
    }
    else if(*p->text == '\"')
    {
        dj = djParseString(p);
    }
    /*
    if (*p->text=='\"')             { return parse_string(item,p->text); }
    if (*p->text=='-' || (*p->text>='0' && *p->text<='9'))  { return parse_number(item,p->text); }
    if (*p->text=='[')              { return parse_array(item,p->text); }
    if (*p->text=='{')              { return parse_object(item,p->text); }
    */

    return dj;
}

dynJSON *djParse(const char *jsonText)
{
    dynJSONParser parser = {0};
    dynJSON *dj;

    parser.text = jsonText;
    dj = djParseValue(&parser);
    djParserClear(&parser);
    return dj;
}


// Dave Gamble's original copyright, intact:

/*
  Copyright (c) 2009 Dave Gamble

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

