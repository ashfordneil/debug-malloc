Debug Malloc
============

Disclaimer: does not support windows, untested on anything but one particular Linux box

A custom crafted malloc implementation, designed for fast failing. By default,
if you access memory that is outside the allocated region (but not too far
outside the allocated region) you often get non-deterministic behaviour,
including but not limited to:

- segmentation faults
- corruption of memory in other allocated regions
- corruption of metadata used by the malloc implementation, and calls to free
  raising SIGABRT
- no detectable side effects, making debugging harder

This malloc implementation can be used in debugging by adding it to the
[LD_PRELOAD](https://jvns.ca/blog/2014/11/27/ld-preload-is-super-fun-and-easy/)
environment variable. I expect it will increase the total memory usage
(untested) of programs using it, and slow them down (definitely untested). In
exchange, any access to memory 0-4096 (insert your page size here) bytes after
the end of the allocated region will deterministically cause a segmentation
fault.

## How it works

Rather than using sbrk(2) to build a global arena and manually managing the
allocation and freeing of chunks of memory within it, this library uses
mmap(2). Each allocated region is a separately mapped region of memory.

The segmentation faults work as follows. Consider a request to allocate N
bytes (allocated bytes represented by `*`):

First, metadata (`@`) relating to the size of the allocation is prepended to the
data.

```
@@@@************
```

Secondly, the number of pages required to store this data is calculated.

```
|-----------|-----------|
|@@@@*******|*****      |
|-----------|-----------|
```

The data is then aligned so that the end of the allocated region lines up with
a page boundary.

```
|-----------|-----------|
|      @@@@*|***********|
|-----------|-----------|
```

Another page is allocated at the end of this region, which is explicitly marked
as not able to be accessed.

```
|-----------|-----------|-----------|
|      @@@@*|***********|XXXXXXXXXXX|
|-----------|-----------|-----------|
```

Finally, a pointer to the beginning of the allocated region is returned.

```
|----------v|-----------|-----------|
|      @@@@*|***********|XXXXXXXXXXX|
|----------^|-----------|-----------|
```

## Caveats

This library cannot do anything if you go off the beginning of the allocated
region. The same method could be applied backwards, but I'm guessing that more
issues will be caused by running off the end of an allocated region than
running off the beginning. In fact, if you modify memory that is off the
beginning of the allocated region there is a chance that the free or realloc
functions will have issues caused by corruption of metadata.

Also, there are definitely memory profilers and tooling that are more powerful
and more able to assist in debugging memory issues.
