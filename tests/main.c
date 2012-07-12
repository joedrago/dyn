#include "dynamic/array.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct Object
{
    const char *name;
} Object;

static void destroyObject(void *userData, Object *obj)
{
    printf("destroying object %s\n", obj->name);
    free(obj);
}

static const char *names[] = {
    "A",
    "B",
    "C",
    "D",
    "E"
};

int main(int argc, char **argv)
{
    int i;
    Object **objects = NULL;
    daCreate(&objects);
    for(i = 0; i < 5; i++)
    {
        Object *obj = (Object *)calloc(1, sizeof(*obj));
        obj->name = names[i];
        daPush(&objects, obj);
    }
    for(i = 0; i < 5; i++)
    {
        printf("Object %d: %s\n", i, objects[i]->name);
    }
    daDestroy(&objects, (daDestroyFunc)destroyObject, NULL);
    return 0;
}

