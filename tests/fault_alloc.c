#include "fault_alloc.h"

#include <stdlib.h>

int fault_alloc_countdown = -1;

void *fault_malloc(size_t size)
{
    if (fault_alloc_countdown >= 0) {
        if (fault_alloc_countdown == 0)
            return NULL;
        fault_alloc_countdown--;
    }
    return malloc(size); /* this TU is compiled WITHOUT -Dmalloc */
}
