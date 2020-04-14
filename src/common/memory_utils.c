#include "memory_utils.h"
#include "log.h"

#include <stdlib.h>

static struct Memory memory = { .allocated = 0, .freed = 0 };

struct Memory_Allocation* memory_get_allocation_ptr(void* memory_block)
{
	return (struct Memory_Allocation*)((unsigned char*) memory_block - offsetof(struct Memory_Allocation, allocation));
}

void* memory_allocate(size_t size)
{
	struct Memory_Allocation* allocation = malloc(sizeof(*allocation) + size);
	if(!allocation)
	{
		log_raw("MEMORY: Failed to allocate memory of size %d", size);
	}
	else
	{
		allocation->size = size;
		//memory->allocated += size;
		memory.allocated += size + sizeof(*allocation);
	}
	return allocation->allocation;
}

void* memory_reallocate_(void** ptr, size_t size)
{
	if(ptr != NULL && *ptr == NULL) // Behave like malloc
	{
		return memory_allocate(size);
	}

	struct Memory_Allocation* current_allocation = memory_get_allocation_ptr(*ptr);
	void* reallocated_memory = realloc(current_allocation, sizeof(*current_allocation) + size);
	if(!reallocated_memory)
	{
		log_raw("MEMORY: Failed to reallocated memory of size %d", size);
	}
	else
	{
		memory.allocated += (size - current_allocation->size);
		current_allocation->size = size;
		current_allocation = reallocated_memory;
	}

	return current_allocation->allocation;

}

void* memory_allocate_and_clear(size_t count, size_t size)
{
	struct Memory_Allocation* allocation = calloc(count, sizeof(*allocation) + size);
	if(!allocation)
	{
		log_raw("MEMORY: Failed to allocate memory of size %d", size);
	}
	else
	{
		allocation->size = size;
		//memory->allocated += size;
		memory.allocated += size + sizeof(*allocation);
	}
	return allocation->allocation;
}

void memory_free(void* ptr)
{
	if(!ptr) return;
	struct Memory_Allocation* allocation = memory_get_allocation_ptr(ptr);
	//memory->allocated -= allocation->size;
	memory.allocated -= allocation->size + sizeof(*allocation);
	memory.freed += allocation->size + sizeof(*allocation);
	free(allocation);
}

struct Memory* memory_get(void)
{
	return &memory;
}
