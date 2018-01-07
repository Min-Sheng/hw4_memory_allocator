#include "hw_malloc.h"
int first_alloc = 1;
void bin_init(bin_t *bin_)
{
	bin_->next = (chunk_ptr_t)bin_;
	bin_->prev=(chunk_ptr_t)bin_;
}

int bin_is_empty(bin_t *bin_)
{
	if(bin_->next==(chunk_ptr_t)bin)
		return 1;
	else
		return 0;
}

void bin_add_chunk(bin_t *bin_,chunk_ptr_t chunk_ptr)
{
	chunk_ptr_t last=bin_->prev;
	if(bin_==&bin[6]) {
		chunk_ptr_t tmp = bin_->next;
		while (tmp != (chunk_ptr_t)&bin[6]) {
			if(chunk_ptr->chunk_size > tmp->chunk_size) {
				last = tmp->prev;
				bin_ =(bin_t*)tmp;
				break;
			}
			tmp = tmp->next;
		}
	}
	last->next = chunk_ptr;
	bin_->prev = chunk_ptr;
	chunk_ptr->next = (chunk_ptr_t)bin_;
	chunk_ptr->prev = last;
}

void bin_show(bin_t *bin_)
{
	chunk_ptr_t current = bin_->next;
	while (current != (chunk_ptr_t)bin_) {
		printf("-> free chunk: %p", current);
		current = current->next;
	}
}
void *hw_malloc(size_t bytes)
{

	void* alloc_start_addr;
	void* free_start_addr;
	if (first_alloc) {
		heap_start_addr = sbrk(65536);
		if(*(int*)heap_start_addr==-1)
			return NULL;
		alloc_start_addr = heap_start_addr + 40;
		free_start_addr = alloc_start_addr + bytes;
		chunk_ptr_t new_chunk_ptr = (chunk_ptr_t)free_start_addr;
		new_chunk_ptr->chunk_size = 65536 - (40*2 + bytes);
		new_chunk_ptr->pre_chunk_size = bytes;
		new_chunk_ptr->prev_free_flag = 0;
		int i = 0;
		for (i = 0; i < 7; i++)
			bin_init(&bin[i]);
		bin_add_chunk(&bin[6],new_chunk_ptr);
		first_alloc = 0;
	} else {
		return NULL;
	}
	return alloc_start_addr;
}

int hw_free(void *mem)
{
	return 0;
}

void *get_start_sbrk(void)
{
	int i = 0;
	for (i = 0; i < 7; i++) {
		printf("bin[%d]",i);
		bin_show(&bin[i]);
		printf("\n");
	}
	brk(heap_start_addr);
	return heap_start_addr;
}
