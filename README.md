# dyn

Everyone else has a C library for arrays, strings, and hashes. Why can't I?

## Example Assumptions

Assume all examples below have access to an "Object" and associated functions, such as:

```C
typedef struct Object
{
    int a;
    int b;
} Object;
Object *createObject(...);           // allocs object, sets data from args
void destroyObject(Object *obj);     // frees obj and all member data
void destroyObjectPtr(Object **obj); // calls destroyObject on *obj, NULLs out *obj
```

## Array Examples

### Pointer array

```C
Object **objects = NULL;
int i;
for(i = 0; i < 5; ++i)
{
    Object *obj = createObject(...);
    daPush(&objects, obj);
}
// objects[0] - objects[4] have data in them
daDestroy(&objects, destroyObjectPtr);
```

### Integer array

```C
int i;
int *ints = NULL;

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
```
