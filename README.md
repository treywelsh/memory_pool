# Memory pool

Non thread safe implementation of a fixed size blocks memory pool.
Feature : memcheck aware

## BUILD
To build static and shared libraries, just run make.

## USAGE
```C
struct pool p;
void *block;
size_t align = sizeof(int);

ret = pool_init(&p, element_count, element_size, align);
if (ret) {
    /* handle error */
}

block = pool_alloc(&p);
if (block == NULL) {
    /* handle error */
}

pool_free(&p, block);
pool_clean(&p);
```

## TEST
In order to test only the library, you can build a test binary : 

Statically linked:  
make DEBUG=1 STATIC=1 test  
valgrind --leak-check=full ./test/pool_test

Dynamically linked:  
make DEBUG=1 STATIC=0 test  
LD_LIBRARY_PATH=lib valgrind --leak-check=full ./test/pool_test


## NOTES 
* check the size written in allocated block. You may corrupt the pool.
* pool_free do not check if you free previously allocated elements from the pool.
