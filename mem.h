//
// Created by hackeris on 15-6-20.
//

#ifndef UHTTP_MEM_H
#define UHTTP_MEM_H

#include <sys/types.h>

void *mem_alloc(size_t size);
void mem_free(void * mem);

#endif //UHTTP_MEM_H
