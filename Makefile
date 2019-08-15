CC=gcc
CFLAGS=-Wall -Wextra -std=gnu99 -fPIC -shared

.PHONY: clean

libdebug_malloc.so: debug_malloc.c
	$(CC) $(CFLAGS) debug_malloc.c -o libdebug_malloc.so

clean:
	rm -f libdebug_malloc.so
