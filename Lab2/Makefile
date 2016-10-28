all: th_alloc test alltest calloctest largetest poisontest size2leveltest malloctest freetest exercise4test

th_alloc: th_alloc.c
	gcc -fPIC -Wall -Wextra -g -shared th_alloc.c -o th_alloc.so

test: tests/test.c
	gcc -Wall -Wextra -g tests/test.c -o test

alltest: tests/alltest.c
	gcc -Wall -Wextra -g tests/alltest.c -o alltest

calloctest: tests/calloctest.c
	gcc -Wall -Wextra -g tests/calloctest.c -o calloctest

reallocationtest: tests/reallocationtest.c
	gcc -Wall -Wextra -g tests/reallocationtest.c -o reallocationtest

largetest: tests/largetest.c
	gcc -Wall -Wextra -g tests/largetest.c -o largetest

poisontest: tests/poisontest.c
	gcc -Wall -Wextra -g tests/poisontest.c -o poisontest

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

