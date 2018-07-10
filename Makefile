all: compile
	LD_PRELOAD=`pwd`/lib64/libruntime.so LD_LIBRARY_PATH=`pwd`/lib64 ./a.out -p 4  --mutexl

compile: *.c
	clang -std=c99 -pedantic *.c -Wall -L./lib64 -lruntime -o a.out

clean:
	rm a.out

