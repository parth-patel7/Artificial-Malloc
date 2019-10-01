all: memgrind.c mymalloc.o
	gcc memgrind.c mymalloc.o -o memgrind

mymalloc.o: mymalloc.c
	gcc -c mymalloc.c

clean: memgrind.c mymalloc.o
	rm memgrind; rm mymalloc.o
