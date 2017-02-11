# Memory pool

Non thread safe implementation of memory pool of fixed size block.
Feature : memcheck aware

NOTE : check the size written in allocated block. You may corrupt the pool.

To build libraries, just run make.

In order to test only the library, you can build a test binary : 

Statically linked :
make DEBUG=1 STATIC=1 test
./test/pool_test

Dynamically linked :
make DEBUG=1 STATIC=0 test
LD_LIBRARY_PATH=lib ./test/pool_test
