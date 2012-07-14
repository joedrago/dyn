// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#include "dynArray.h"
#include "dynString.h"
#include "dynMap.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

// ------------------------------------------------------------------------------------------------
// Test Globals (sue me)

int totalErrors = 0;

// ------------------------------------------------------------------------------------------------
// Helper structs and functions for testing

void testFail(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "* ERROR: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    totalErrors++;
}

typedef struct Object
{
    const char *name;
} Object;

static void destroyObject(Object *obj)
{
    printf("destroying object %s\n", obj->name);
    free(obj);
}

static void destroyObjectP1(const char *p1, Object *obj)
{
    printf("destroying object (p1: \"%s\") %s\n", p1 ? p1 : "", obj->name);
    free(obj);
}

static void destroyObjectP2(const char *p1, const char *p2, Object *obj)
{
    printf("destroying object (p1: \"%s\", p2: \"%s\") %s\n", p1 ? p1 : "", p2 ? p2 : "", obj->name);
    free(obj);
}

static const char *names[] = {
    "A", "B", "C", "D", "E"
};

static void fillObjects(Object ***arr)
{
    int i;
    for(i = 0; i < 5; ++i)
    {
        Object *obj = (Object *)calloc(1, sizeof(*obj));
        obj->name = names[i];
        daPush(arr, obj);
    }
}

static Object *createObject(const char *name)
{
    Object *object = (Object *)calloc(1, sizeof(*object));
    object->name = name;
    return object;
}

static void printObjects(Object ***arr)
{
    int i;
    printf("[\n");
    for(i = 0; i < daSize(arr); ++i)
    {
        printf("    Object %d: %s\n", i, (*arr)[i]->name);
    }
    printf("]\n");
}

static void printString(char **str)
{
    printf("String [P:0x%p L:" DYNAMIC_STRING_SIZE_FORMAT " C:" DYNAMIC_STRING_SIZE_FORMAT "]: \"%s\"\n", 
        str,
        dsLength(str),
        dsCapacity(str),
        *str
        );
}

// ------------------------------------------------------------------------------------------------
// dynArray Tests

void test_daCreate()
{
    Object **objects = NULL;
    daCreate(&objects);
    daDestroy(&objects, NULL);
}

void test_daDestroy()
{
    Object **objects = NULL;
    daDestroy(&objects, (daDestroyFunc)destroyObject); // should do nothing (no lazy creation)
    fillObjects(&objects);
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daDestroyP1()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daDestroyP1(&objects, (daDestroyFuncP1)destroyObjectP1, "test_daDestroyP1");
}

void test_daDestroyP2()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daDestroyP2(&objects, (daDestroyFuncP2)destroyObjectP2, "test_daDestroyP1", "pee two");
}

void test_daClear()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daClear(&objects, (daDestroyFunc)destroyObject);
    daDestroy(&objects, NULL);
}

void test_daClearP1()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daClearP1(&objects, (daDestroyFuncP1)destroyObjectP1, "clearP1 p1");
    daDestroy(&objects, NULL);
}

void test_daClearP2()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daClearP2(&objects, (daDestroyFuncP2)destroyObjectP2, "clearP2 p1", "clearP2 p2");
    daDestroy(&objects, NULL);
}

void test_daShift()
{
    Object **objects = NULL;
    Object *obj;
    fillObjects(&objects);
    while(obj = daShift(&objects))
    {
        printf("Shifted Object: %s\n", obj->name);
        destroyObject(obj);
    }
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daUnshift()
{
    Object **objects = NULL;
    int i;
    for(i = 0; i < 5; ++i)
    {
    }
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daPush()
{
    Object **objects = NULL;
    int i;
    for(i = 0; i < 5; ++i)
    {
        Object *obj = (Object *)calloc(1, sizeof(*obj));
        obj->name = names[i];
        daPush(&objects, obj);
    }
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daPop()
{
    Object **objects = NULL;
    Object *obj;
    fillObjects(&objects);
    while(obj = daPop(&objects))
    {
        printf("Popped Object: %s\n", obj->name);
        destroyObject(obj);
    }
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daInsert()
{
    Object **objects = NULL;
    Object *obj;
    int i;
    for(i = 0; i < 5; ++i)
    {
        obj = (Object *)calloc(1, sizeof(*obj));
        obj->name = names[i];
        daInsert(&objects, 2, obj);
        printObjects(&objects);
    }

    obj = (Object *)calloc(1, sizeof(*obj));
    obj->name = "first";
    daInsert(&objects, 0, obj);
    obj = (Object *)calloc(1, sizeof(*obj));
    obj->name = "last";
    daInsert(&objects, daSize(&objects), obj);
    printObjects(&objects);
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daErase()
{
    Object **objects = NULL;
    fillObjects(&objects);
    destroyObject(objects[1]);
    daErase(&objects, 1);
    destroyObject(objects[3]);
    daErase(&objects, 3);
    printObjects(&objects);
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daEraseP1()
{
    Object **objects = NULL;
    fillObjects(&objects);
    destroyObject(objects[1]);
    daErase(&objects, 1);
    destroyObject(objects[3]);
    daErase(&objects, 3);
    printObjects(&objects);
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daEraseP2()
{
    Object **objects = NULL;
    fillObjects(&objects);
    destroyObject(objects[1]);
    daErase(&objects, 1);
    destroyObject(objects[3]);
    daErase(&objects, 3);
    printObjects(&objects);
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daSetSize()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daSetSize(&objects, 20, (daDestroyFunc)destroyObject);
    daSetSize(&objects, 2, (daDestroyFunc)destroyObject);
    daSetSize(&objects, 20, (daDestroyFunc)destroyObject);
    daSetSize(&objects, 2, (daDestroyFunc)destroyObject);
    printObjects(&objects);
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daSetSizeP1()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daSetSizeP1(&objects, 20, (daDestroyFuncP1)destroyObjectP1, "daSetSizeP1 p1");
    daSetSizeP1(&objects, 2, (daDestroyFuncP1)destroyObjectP1, "daSetSizeP1 p1");
    daSetSizeP1(&objects, 20, (daDestroyFuncP1)destroyObjectP1, "daSetSizeP1 p1");
    daSetSizeP1(&objects, 2, (daDestroyFuncP1)destroyObjectP1, "daSetSizeP1 p1");
    printObjects(&objects);
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daSetSizeP2()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daSetSizeP2(&objects, 20, (daDestroyFuncP2)destroyObjectP2, "p1", "p2");
    daSetSizeP2(&objects, 2, (daDestroyFuncP2)destroyObjectP2, "p1", "p2");
    daSetSizeP2(&objects, 20, (daDestroyFuncP2)destroyObjectP2, "p1", "p2");
    daSetSizeP2(&objects, 2, (daDestroyFuncP2)destroyObjectP2, "p1", "p2");
    printObjects(&objects);
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daSize()
{
    Object **objects = NULL;
    daSize(&objects); // daSize() should never lazily create a dynArray
    if(objects)
        testFail("daSize is lazily creating dynArrays");
    fillObjects(&objects);
    printf("size: " DYNAMIC_ARRAY_SIZE_FORMAT "\n", daSize(&objects));
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daSetCapacity()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daSetCapacity(&objects, 10, (daDestroyFunc)destroyObject);
    daSetCapacity(&objects, 3, (daDestroyFunc)destroyObject);
    daSetCapacity(&objects, 10, (daDestroyFunc)destroyObject);
    printObjects(&objects);
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daSetCapacityP1()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daSetCapacityP1(&objects, 10, (daDestroyFuncP1)destroyObjectP1, "p1");
    daSetCapacityP1(&objects, 3, (daDestroyFuncP1)destroyObjectP1, "p1");
    daSetCapacityP1(&objects, 10, (daDestroyFuncP1)destroyObjectP1, "p1");
    printObjects(&objects);
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daSetCapacityP2()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daSetCapacityP2(&objects, 10, (daDestroyFuncP2)destroyObjectP2, "p1", "p2");
    daSetCapacityP2(&objects, 3, (daDestroyFuncP2)destroyObjectP2, "p1", "p2");
    daSetCapacityP2(&objects, 10, (daDestroyFuncP2)destroyObjectP2, "p1", "p2");
    printObjects(&objects);
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daCapacity()
{
    Object **objects = NULL;
    daCapacity(&objects); // daSize() should never lazily create a dynArray
    if(objects)
        testFail("daCapacity is lazily creating dynArrays");
    fillObjects(&objects);
    printf("capacity: " DYNAMIC_ARRAY_SIZE_FORMAT "\n", daCapacity(&objects));
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

void test_daSquash()
{
    Object **objects = NULL;
    fillObjects(&objects);
    destroyObject(objects[1]);
    objects[1] = NULL;
    destroyObject(objects[3]);
    objects[3] = NULL;
    daSquash(&objects);
    printObjects(&objects);
    daDestroy(&objects, (daDestroyFunc)destroyObject);
}

// ------------------------------------------------------------------------------------------------
// dynString Tests

void test_dsCreate()
{
    char *str = NULL;
    dsCreate(&str);
    dsDestroy(&str);
    dsCreate(&str);
    dsDestroy(&str);
}

void test_dsClear()
{
    char *str = NULL;
    dsClear(&str);
    printString(&str);
    dsDestroy(&str);
}

void test_dsCopy()
{
    char *str = NULL;
    dsCopy(&str, "here is a string");
    printString(&str);
    dsCopy(&str, "I changed it");
    printString(&str);
    dsCopy(&str, "I changed it more");
    printString(&str);
    dsCopy(&str, "I changed it less");
    printString(&str);
    dsDestroy(&str);
}

void test_dsPrintf()
{
    char *str = NULL;
    dsPrintf(&str, "here is a string with numbers %d, %d, %d, %d, %d", 1, 2, 3, 4, 5);
    printString(&str);
    dsPrintf(&str, "%d, %d, %d, %d, %d", 1, 2, 3, 4, 5);
    printString(&str);
    dsDestroy(&str);
}

void test_dsSetLength()
{
    char *str = NULL;
    dsCopy(&str, "here");
    printString(&str);
    dsSetLength(&str, 2);
    printString(&str);
    dsSetLength(&str, 8);
    printString(&str);
    dsSetLength(&str, 6);
    printString(&str);
    dsSetLength(&str, 8);
    printString(&str);
    dsSetLength(&str, 10);
    printString(&str);
    dsDestroy(&str);
}

void test_dsCalcLength()
{
    char *str = NULL;
    dsCopy(&str, "here");
    printString(&str);
    str[2] = 0;
    dsCalcLength(&str);
    printString(&str);
    dsDestroy(&str);
}

void test_dsSetCapacity()
{
    char *str = NULL;
    dsCopy(&str, "here");
    printString(&str);
    dsSetCapacity(&str, 2);
    printString(&str);
    dsSetCapacity(&str, 8);
    printString(&str);
    dsSetCapacity(&str, 6);
    printString(&str);
    dsSetCapacity(&str, 8);
    printString(&str);
    dsSetCapacity(&str, 10);
    printString(&str);
    dsDestroy(&str);
}

// ------------------------------------------------------------------------------------------------
// dynMap Tests

void test_dmCreate()
{
    char *integers = NULL;
    dmCreate(&integers, KEYTYPE_STRING, sizeof(char), DYNAMIC_MAP_DEFAULT_WIDTH);
    dmGetS(integers, "Foo") = 'A';
    dmGetS(integers, "Bar") = 'B';
    dmGetS(integers, "Baz") = 'C';
    printf("Foo: %c\n", dmGetS(integers, "Foo"));
    printf("Bar: %c\n", dmGetS(integers, "Bar"));
    printf("Baz: %c\n", dmGetS(integers, "Baz"));
    dmDestroy(&integers, NULL);
}

void test_dmGetS()
{
    Object **objects = NULL;
    dmGetS(objects, "Foo") = createObject("Herp");
    dmGetS(objects, "Bar") = createObject("Derp");
    dmGetS(objects, "Baz") = createObject("Skerp");
    printf("Foo: 0x%p\n", dmGetS(objects, "Foo"));
    printf("Bar: 0x%p\n", dmGetS(objects, "Bar"));
    printf("Baz: 0x%p\n", dmGetS(objects, "Baz"));
    dmClear(&objects, (dmDestroyFunc)destroyObject);
    printf("Foo: 0x%p\n", dmGetS(objects, "Foo"));
    printf("Bar: 0x%p\n", dmGetS(objects, "Bar"));
    printf("Baz: 0x%p\n", dmGetS(objects, "Baz"));
    printf("has index Foo: %d\n", dmHasStringIndex(&objects, "Foo"));
    printf("has index Wat: %d\n", dmHasStringIndex(&objects, "Wat"));
    dmDestroy(&objects, (dmDestroyFunc)destroyObject);
}

void test_dmGetI()
{
    int *integers = NULL;
    dmCreate(&integers, KEYTYPE_INTEGER, sizeof(int), DYNAMIC_MAP_DEFAULT_WIDTH);
    dmGetI(integers, 15) = 150;
    dmGetI(integers, 16) = 160;
    dmGetI(integers, 17) = 170;
    printf("15: %d\n", dmGetI(integers, 15));
    printf("16: %d\n", dmGetI(integers, 16));
    printf("17: %d\n", dmGetI(integers, 17));
    dmDestroy(&integers, NULL);
}

// ------------------------------------------------------------------------------------------------
// "Harness"

#define TEST(FUNC) \
    printf("----------------------------------------------------------------\n"); \
    printf("Test: " #FUNC "\n"); \
    prevErrors = totalErrors; \
    test_ ## FUNC(); \
    if(totalErrors > prevErrors) printf("FAILED: %d error(s)\n", totalErrors - prevErrors); else printf("SUCCESS\n") \

int main(int argc, char **argv)
{
    int prevErrors = 0;

    TEST(daCreate);
    TEST(daDestroy);
    TEST(daDestroyP1);
    TEST(daDestroyP2);
    TEST(daClear);
    TEST(daClearP1);
    TEST(daClearP2);
    TEST(daShift);
    TEST(daUnshift);
    TEST(daPush);
    TEST(daPop);
    TEST(daInsert);
    TEST(daErase);
    TEST(daEraseP1);
    TEST(daEraseP2);
    TEST(daSetSize);
    TEST(daSetSizeP1);
    TEST(daSetSizeP2);
    TEST(daSize);
    TEST(daSetCapacity);
    TEST(daSetCapacityP1);
    TEST(daSetCapacityP2);
    TEST(daCapacity);
    TEST(daSquash);

    TEST(dsCreate);
    TEST(dsClear);
    TEST(dsCopy);
    TEST(dsPrintf);
    TEST(dsSetLength);
    TEST(dsCalcLength);
    TEST(dsSetCapacity);

    TEST(dmCreate);
    TEST(dmGetS);
    TEST(dmGetI);

    printf("\nTotal errors: %d\n\n", totalErrors);
    return 0;
}