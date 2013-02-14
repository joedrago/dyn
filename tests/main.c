// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2012.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#include "dyn.h"

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

static void destroyObject(Object **obj)
{
    printf("destroying object %s\n", (*obj)->name);
    free(*obj);
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
    printf("String [P:0x%p L:" dynSizeFormat " C:" dynSizeFormat "]: \"%s\"\n", 
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
    daCreate(&objects, 0);
    daDestroy(&objects, NULL);
}

void test_daDestroy()
{
    Object **objects = NULL;
    daDestroy(&objects, (dynDestroyFunc)destroyObject); // should do nothing (no lazy creation)
    fillObjects(&objects);
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daDestroyP1()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daDestroyP1(&objects, (dynDestroyFuncP1)destroyObjectP1, "test_daDestroyP1");
}

void test_daDestroyP2()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daDestroyP2(&objects, (dynDestroyFuncP2)destroyObjectP2, "test_daDestroyP1", "pee two");
}

void test_daClear()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daClear(&objects, (dynDestroyFunc)destroyObject);
    daDestroy(&objects, NULL);
}

void test_daClearP1()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daClearP1(&objects, (dynDestroyFuncP1)destroyObjectP1, "clearP1 p1");
    daDestroy(&objects, NULL);
}

void test_daClearP2()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daClearP2(&objects, (dynDestroyFuncP2)destroyObjectP2, "clearP2 p1", "clearP2 p2");
    daDestroy(&objects, NULL);
}

void test_daShift()
{
    Object **objects = NULL;
    Object *obj;
    fillObjects(&objects);
    while(daShift(&objects, &obj))
    {
        printf("Shifted Object: %s\n", obj->name);
        destroyObject(&obj);
    }
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daUnshift()
{
    Object **objects = NULL;
    int i;
    for(i = 0; i < 5; ++i)
    {
    }
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
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
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daPop()
{
    Object **objects = NULL;
    Object *obj;
    fillObjects(&objects);
    while(daPop(&objects, &obj))
    {
        printf("Popped Object: %s\n", obj->name);
        destroyObject(&obj);
    }
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
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
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daErase()
{
    Object **objects = NULL;
    fillObjects(&objects);
    destroyObject(&objects[1]);
    daErase(&objects, 1);
    destroyObject(&objects[3]);
    daErase(&objects, 3);
    printObjects(&objects);
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daEraseP1()
{
    Object **objects = NULL;
    fillObjects(&objects);
    destroyObject(&objects[1]);
    daErase(&objects, 1);
    destroyObject(&objects[3]);
    daErase(&objects, 3);
    printObjects(&objects);
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daEraseP2()
{
    Object **objects = NULL;
    fillObjects(&objects);
    destroyObject(&objects[1]);
    daErase(&objects, 1);
    destroyObject(&objects[3]);
    daErase(&objects, 3);
    printObjects(&objects);
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daSetSize()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daSetSize(&objects, 4, (dynDestroyFunc)destroyObject);
    daSetSize(&objects, 2, (dynDestroyFunc)destroyObject);
    daSetSize(&objects, 2, (dynDestroyFunc)destroyObject);
    printObjects(&objects);
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daSetSizeP1()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daSetSizeP1(&objects, 4, (dynDestroyFuncP1)destroyObjectP1, "daSetSizeP1 p1");
    daSetSizeP1(&objects, 2, (dynDestroyFuncP1)destroyObjectP1, "daSetSizeP1 p1");
    daSetSizeP1(&objects, 2, (dynDestroyFuncP1)destroyObjectP1, "daSetSizeP1 p1");
    printObjects(&objects);
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daSetSizeP2()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daSetSizeP2(&objects, 4, (dynDestroyFuncP2)destroyObjectP2, "p1", "p2");
    daSetSizeP2(&objects, 2, (dynDestroyFuncP2)destroyObjectP2, "p1", "p2");
    daSetSizeP2(&objects, 2, (dynDestroyFuncP2)destroyObjectP2, "p1", "p2");
    printObjects(&objects);
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daSize()
{
    Object **objects = NULL;
    daSize(&objects); // daSize() should never lazily create a dynArray
    if(objects)
        testFail("daSize is lazily creating dynArrays");
    fillObjects(&objects);
    printf("size: " dynSizeFormat "\n", daSize(&objects));
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daSetCapacity()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daSetCapacity(&objects, 10, (dynDestroyFunc)destroyObject);
    daSetCapacity(&objects, 3, (dynDestroyFunc)destroyObject);
    daSetCapacity(&objects, 10, (dynDestroyFunc)destroyObject);
    printObjects(&objects);
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daSetCapacityP1()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daSetCapacityP1(&objects, 10, (dynDestroyFuncP1)destroyObjectP1, "p1");
    daSetCapacityP1(&objects, 3, (dynDestroyFuncP1)destroyObjectP1, "p1");
    daSetCapacityP1(&objects, 10, (dynDestroyFuncP1)destroyObjectP1, "p1");
    printObjects(&objects);
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daSetCapacityP2()
{
    Object **objects = NULL;
    fillObjects(&objects);
    daSetCapacityP2(&objects, 10, (dynDestroyFuncP2)destroyObjectP2, "p1", "p2");
    daSetCapacityP2(&objects, 3, (dynDestroyFuncP2)destroyObjectP2, "p1", "p2");
    daSetCapacityP2(&objects, 10, (dynDestroyFuncP2)destroyObjectP2, "p1", "p2");
    printObjects(&objects);
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daCapacity()
{
    Object **objects = NULL;
    daCapacity(&objects); // daSize() should never lazily create a dynArray
    if(objects)
        testFail("daCapacity is lazily creating dynArrays");
    fillObjects(&objects);
    printf("capacity: " dynSizeFormat "\n", daCapacity(&objects));
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}

void test_daSquash()
{
    Object **objects = NULL;
    fillObjects(&objects);
    destroyObject(&objects[1]);
    objects[1] = NULL;
    destroyObject(&objects[3]);
    objects[3] = NULL;
    daSquash(&objects);
    printObjects(&objects);
    daDestroy(&objects, (dynDestroyFunc)destroyObject);
}


void test_da8()
{
    int i;
    char *chars = NULL;

    daCreate(&chars, sizeof(char));
    daPushU8(&chars, 'A');
    daPushU8(&chars, 'B');
    daPushU8(&chars, 'C');
    daPushU8(&chars, 'D');
    for(i = 0; i < daSize(&chars); ++i)
    {
        printf("char %d: %c\n", i, chars[i]);
    }
    daDestroy(&chars, NULL);
}

void test_da32()
{
    int i;
    int *ints = NULL;
    float *floats = NULL;

    daCreate(&ints, sizeof(int));
    daPushU32(&ints, 5);
    daPushU32(&ints, 7);
    daPushU32(&ints, 9);
    daPushU32(&ints, 11);
    for(i = 0; i < daSize(&ints); ++i)
    {
        printf("int %d: %d\n", i, ints[i]);
    }
    daDestroy(&ints, NULL);

    daCreate(&floats, sizeof(float));
    daPushF32(&floats, 5.0f);
    daPushF32(&floats, 7.0f);
    daPushF32(&floats, 9.0f);
    daPushF32(&floats, 11.0f);
    for(i = 0; i < daSize(&floats); ++i)
    {
        printf("float %d: %2.2f\n", i, floats[i]);
    }
    daDestroy(&floats, NULL);
}

void test_daStruct()
{
    int i;
    struct derp
    {
        int a;
        int b;
    } temp, *objs = NULL;

    daCreate(&objs, sizeof(struct derp));

    temp.a = 5;
    temp.b = 50;
    daPush(&objs, temp);
    temp.a = 6;
    temp.b = 60;
    daPush(&objs, temp);
    temp.a = 7;
    temp.b = 70;
    daPush(&objs, temp);

    for(i = 0; i < daSize(&objs); ++i)
    {
        printf("int %d: %d / %d\n", i, objs[i].a, objs[i].b);
    }
    daDestroy(&objs, NULL);
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

void test_dmGetS()
{
    dynMap *dm = dmCreate(KEYTYPE_STRING, 0);
    dmGetS2P(dm, "Foo") = "A";
    dmGetS2P(dm, "Bar") = "B";
    dmGetS2P(dm, "Baz") = "C";
    printf("Foo: %s\n", (char *)dmGetS2P(dm, "Foo"));
    printf("Bar: %s\n", (char *)dmGetS2P(dm, "Bar"));
    printf("Baz: %s\n", (char *)dmGetS2P(dm, "Baz"));
    dmDestroy(dm, NULL);
}

#define GETI_COUNT 100000
void test_dmGetI()
{
    dynMap *dm = dmCreate(KEYTYPE_INTEGER, 0);
    int i;
    printf("count: %d, mod: %d, split: %d, width: %d, capacity: %d\n", dm->count, dm->mod, dm->split, daSize(&dm->table), daCapacity(&dm->table));
    for(i = 0; i < GETI_COUNT; ++i)
    {
        dmGetI2I(dm, i * 16) = i * 10;
    }
    printf("count: %d, mod: %d, split: %d, width: %d, capacity: %d\n", dm->count, dm->mod, dm->split, daSize(&dm->table), daCapacity(&dm->table));
    for(i = 0; i < GETI_COUNT; ++i)
    {
        dmEraseInteger(dm, i * 16, NULL);
    }
    printf("count: %d, mod: %d, split: %d, width: %d, capacity: %d\n", dm->count, dm->mod, dm->split, daSize(&dm->table), daCapacity(&dm->table));
    dmDestroy(dm, NULL);
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
//    TEST(daDestroyP1);
//    TEST(daDestroyP2);
    TEST(daClear);
//    TEST(daClearP1);
//    TEST(daClearP2);
    TEST(daShift);
    TEST(daUnshift);
    TEST(daPush);
    TEST(daPop);
    TEST(daInsert);
    TEST(daErase);
//    TEST(daEraseP1);
//    TEST(daEraseP2);
    TEST(daSetSize);
//    TEST(daSetSizeP1);
//    TEST(daSetSizeP2);
    TEST(daSize);
    TEST(daSetCapacity);
//    TEST(daSetCapacityP1);
//    TEST(daSetCapacityP2);
    TEST(daCapacity);
    TEST(daSquash);
    TEST(da8);
    TEST(da32);
    TEST(daStruct);

    TEST(dsCreate);
    TEST(dsClear);
    TEST(dsCopy);
    TEST(dsPrintf);
    TEST(dsSetLength);
    TEST(dsCalcLength);
    TEST(dsSetCapacity);

    TEST(dmGetS);
    TEST(dmGetI);

    printf("\nTotal errors: %d\n\n", totalErrors);
    return 0;
}
