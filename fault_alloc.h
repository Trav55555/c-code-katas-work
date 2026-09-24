#ifndef FAULT_ALLOC_H
#define FAULT_ALLOC_H

#include <stddef.h>

/* Test-only malloc replacement for allocation-failure injection.
 * Compiled over the modules with -Dmalloc=fault_malloc.
 *
 * Project decision: defining a macro named after a standard library
 * function while including its header is undefined behavior (C99 7.1.3,
 * 7.1.4). It is accepted here because it is confined to the `make fault`
 * test build, works on the glibc/gcc/clang toolchains this repository
 * targets, and keeps the production code free of allocator hooks. The
 * shipped builds (debug, release, asan) never define it.
 * countdown < 0: never fail. countdown == 0: this call returns NULL
 *   (sticky: keeps failing until set elsewhere). countdown > 0: pass
 *   through and decrement. */
extern int fault_alloc_countdown;

void *fault_malloc(size_t size);

#endif
