#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include <stddef.h>

struct Memory
{
	size_t allocated;
	size_t freed;
};

struct Memory_Allocation
{
	size_t size;
	unsigned char allocation[];
};

void*          memory_allocate(size_t size);
void*          memory_reallocate_(void** ptr, size_t size);
void*          memory_allocate_and_clear(size_t count, size_t size);
void           memory_free(void* ptr);
struct Memory* memory_get(void);

#define memory_reallocate(ptr, size) memory_reallocate_(&ptr, size);

#endif
