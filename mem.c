//
// Created by hackeris on 15-6-20.
//

#include <stdlib.h>
#include "mem.h"

void *mem_alloc(size_t size) {

    return malloc(size);
}

void mem_free(void *mem){

    free(mem);
}
