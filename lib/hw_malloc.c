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
		printf("(chunk_size=%llu; prev_chunk_size=%llu; prev_free_flag=%llu)",(unsigned long long)current->chunk_size, (unsigned long long)current->pre_chunk_size, (unsigned long long)current->prev_free_flag);
		current = current->next;
	}
}

int bin_choose(size_t bytes)
{
	if(bytes<=48)
		return 0;
	else if(bytes>48&&bytes<=56)
		return 1;
	else if(bytes>56&&bytes<=64)
		return 2;
	else if(bytes>64&&bytes<=72)
		return 3;
	else if(bytes>72&&bytes<=80)
		return 4;
	else if(bytes>80&&bytes<=88)
		return 5;
	else
		return 6;
}

void *chunk_split(size_t bytes)
{

	/*Split free chunk function for bin 6*/
	chunk_ptr_t chunk_min_fit = bin[6].prev;
	int found = 0, split = 0;
	while (chunk_min_fit != (chunk_ptr_t)&bin[6])
	{
		if(bin_is_empty(&bin[6])){
			break;
		}else if(chunk_min_fit->chunk_size > bytes){
			if(chunk_min_fit->chunk_size - bytes< 48){
				bytes = chunk_min_fit->chunk_size;
				found = 1;
				break;
			}else{
				found = 1;
				split = 1;
				break;
			}
		}else if(chunk_min_fit->chunk_size < bytes){
			chunk_min_fit = chunk_min_fit->prev;
		}
	}
	if(!found)
		return NULL;
	if(split){
		chunk_ptr_t chunk_rest;
		int rest = bin_choose(chunk_min_fit->chunk_size - bytes);
		chunk_rest=(void*)chunk_min_fit + 40 + bytes;
		chunk_rest->chunk_size=chunk_min_fit->chunk_size - bytes;
		chunk_rest->pre_chunk_size = bytes;
		chunk_rest->prev_free_flag = 0;
		bin_add_chunk(&bin[rest], chunk_rest);
	}
	return chunk_min_fit;
}

void *chunk_merge(chunk_ptr_t chunk_ptr)
{
	return NULL;
}

void *chunk_del(chunk_ptr_t chunk_ptr)
{
	/*Delete the split-out free chunk*/
	chunk_ptr->prev->next=chunk_ptr->next;
	chunk_ptr->next->prev=chunk_ptr->prev;
	chunk_ptr->next=NULL;
	chunk_ptr->prev=NULL;
	return chunk_ptr;
}

void *hw_malloc(size_t bytes)
{
	if(bytes < 0)
		return NULL;
	else if(bytes % 8 != 0)
		bytes = (bytes / 8 + 1) * 8;
	void *alloc_start_addr;
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
		int which_bin = bin_choose(bytes);
		chunk_ptr_t new_alloc_ptr;
		if(which_bin==6){
			/*Allocate from bin 6*/
			new_alloc_ptr=chunk_split(bytes);
			chunk_del(new_alloc_ptr);
		}
		else if(bin_is_empty(&bin[which_bin])){
			/*No free chunk in the choosed bin, so allocate from bin 6*/
			new_alloc_ptr=chunk_split(bytes);
			chunk_del(new_alloc_ptr);
		}
		else{
			/*Allocate from the bin other than bin 6*/
			new_alloc_ptr=chunk_del(bin[which_bin].prev);
		}
		alloc_start_addr = (void *)new_alloc_ptr + 40;
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
