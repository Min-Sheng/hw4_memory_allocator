#include "hw_malloc.h"

void* heap_start_addr;
int first_alloc = 0;

void *hw_malloc(size_t bytes)
{

	void* alloc_start_addr;
	void* free_start_addr;
	if (!first_alloc){
		heap_start_addr = sbrk(64 * 1024);
		if(*(int*)heap_start_addr==-1)
			return NULL;
		alloc_start_addr = heap_start_addr + 40;
		free_start_addr = alloc_start_addr + bytes;

	}else{

	}
	return alloc_start_addr;
}

int hw_free(void *mem)
{
	return 0;
}

void *get_start_sbrk(void)
{
	brk(heap_start_addr);
	return heap_start_addr;
}
