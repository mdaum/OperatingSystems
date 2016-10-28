# Lab2 README #

Written by Max Daum and James Barbour

### Files/Directories Included ###
* th_alloc.c (our mem allocator)
* makefile (builds th_alloc and all tests)
* tests directory (contains all tests used)

### Test Cases ###

* Note that though we have 5 individual "tests" for malloc/free, each set of tests resides in its own c file for ease of use.

### Challenge Problems ###

* Allows allocations/frees over 4KB, we simply return the larger alloc to OS after freeing each time.
* Calloc implemented, and can also calloc allocations over 4KB.

