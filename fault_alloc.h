#ifndef FAULT_ALLOC_H
#define FAULT_ALLOC_H

#include <stddef.h>

/* Test-only malloc replacement for allocation-failure injection.
 * Compiled over the modules with -Dmalloc=fault_malloc.
 * countdown < 0: never fail. countdown == 0: this call returns NULL
 *   (sticky: keeps failing until set elsewhere). countdown > 0: pass
 *   through and decrement. */
extern int fault_alloc_countdown;

void *fault_malloc(size_t size);

#endif
