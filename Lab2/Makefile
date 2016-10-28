all: th_alloc test misctest size2leveltest malloctest freetest exercise4test

th_alloc: th_alloc.c
	gcc -fPIC -Wall -Wextra -g -shared th_alloc.c -o th_alloc.so

test: tests/test.c
	gcc -Wall -Wextra -g tests/test.c -o test

misctest: tests/misctest.c
	gcc -Wall -Wextra -g tests/misctest.c -o misctest

size2leveltest: tests/size2leveltest.c
	gcc -Wall -Wextra -g tests/size2leveltest.c -o size2leveltest

malloctest: tests/malloctest.c
	gcc -Wall -Wextra -g tests/malloctest.c -o malloctest

freetest: tests/freetest.c
	gcc -Wall -Wextra -g tests/freetest.c -o freetest

exercise4test: tests/exercise4test.c
	gcc -Wall -Wextra -g tests/exercise4test.c -o exercise4test

clean:
	rm *test th_alloc.so

