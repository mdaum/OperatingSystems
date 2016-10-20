all: th_alloc test size2leveltest

th_alloc: th_alloc.c
	gcc -fPIC -pedantic -Wall -Wextra -g -shared th_alloc.c -o th_alloc.so

test: tests/test.c
	gcc -pedantic -Wall -Wextra -g tests/test.c -o test

size2leveltest: tests/size2leveltest.c
	gcc -pedantic -Wall -Wextra -g tests/size2leveltest.c -o size2leveltest

clean:
	rm *test th_alloc.so

