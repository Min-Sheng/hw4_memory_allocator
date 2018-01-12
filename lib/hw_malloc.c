#include "hw_malloc.h"

int first_alloc = 1;

void bin_init(bin_t *bin_)
{
	bin_->next = (chunk_ptr_t)bin_;
	bin_->prev = (chunk_ptr_t)bin_;
}

int bin_is_empty(bin_t *bin_)
{
	if(bin_->next==(chunk_ptr_t)bin_)
		return 1;
	else
		return 0;
}

void bin_add_chunk(bin_t *bin_,chunk_ptr_t chunk_ptr)
{
	chunk_ptr_t last=bin_->prev;
	//printf("d1\n");
	if (bin_ == &bin[6]) {
		chunk_ptr_t tmp = bin_->next;
		while (tmp != (chunk_ptr_t)&bin[6]) {
			if(chunk_ptr->chunk_size > tmp->chunk_size) {
				last = tmp->prev;
				bin_ =(bin_t*)tmp;
				//printf("d2\n");
				break;
			}
			tmp = tmp->next;
		}
	}
	//printf("d3\n");
	last->next = chunk_ptr;
	bin_->prev = chunk_ptr;
	chunk_ptr->next = (chunk_ptr_t)bin_;
	chunk_ptr->prev = last;
}

void bin_show(bin_t *bin_)
{
	chunk_ptr_t current = bin_->next;
	while (current != (chunk_ptr_t)bin_) {
		if((void *)current == heap_start_addr)
			printf("-> chunk: 0x00000000");
		else
			printf("-> chunk: %010p", (void *)current-(unsigned long long)heap_start_addr);
		printf("(prev_free_flag=%llu)\n",
		       (unsigned long long)current->prev_free_flag);
		/*
		printf("(next:%p; prev:%p; chunk_size=%llu; prev_chunk_size=%llu; prev_free_flag=%llu)",
		       (void *)current->next, (void *)current->prev,
		       (unsigned long long)current->chunk_size,
		       (unsigned long long)current->pre_chunk_size,
		       (unsigned long long)current->prev_free_flag);
		*/
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
	//printf("c1\n");
	int found = 0, split = 0;
	while (chunk_min_fit != (chunk_ptr_t)&bin[6]) {
		if(bin_is_empty(&bin[6])) {
			break;
		} else if(chunk_min_fit->chunk_size >= bytes) {
			while(chunk_min_fit->chunk_size==chunk_min_fit->prev->chunk_size)
				chunk_min_fit = chunk_min_fit->prev;
			if(chunk_min_fit->chunk_size - bytes < 48) {
				bytes = chunk_min_fit->chunk_size;
				found = 1;
				break;
			} else {
				found = 1;
				split = 1;
				break;
			}
		}else {
			chunk_min_fit = chunk_min_fit->prev;
		}
	}
	//printf("c2\n");
	if(!found)
		return NULL;
	if(split) {
		//printf("s\n");
		//printf("Chunk_min_fit: %p\n", (void *)chunk_min_fit);
		//printf("Rest Size: %llu\n", chunk_min_fit->chunk_size - bytes);
		int rest = bin_choose(chunk_min_fit->chunk_size - bytes);
		//printf("Rest: %d\n", rest);
		chunk_ptr_t chunk_rest = (void *)chunk_min_fit + bytes;
		//printf("heap_end: %p\n", heap_start_addr + 65536);
		//printf("Chunk_rest: %p\n", (void *)chunk_rest);
		//printf("Chunk_min_fit_size: %llu\n", chunk_min_fit->chunk_size);
		chunk_rest->chunk_size = chunk_min_fit->chunk_size - bytes;
		//printf("Chunk_rest_size: %llu\n", chunk_rest->chunk_size);
		chunk_rest->pre_chunk_size = bytes;
		chunk_rest->prev_free_flag = 0;
		chunk_min_fit->chunk_size = bytes;
		//printf("c3\n");
		bin_add_chunk(&bin[rest], chunk_rest);
		//printf("c4\n");
	} else {
		chunk_min_fit->chunk_size = bytes;
	}
	return chunk_min_fit;
}

void *chunk_merge(chunk_ptr_t chunk_ptr)
{
	chunk_ptr_t prev_chunk = NULL, next_chunk = NULL, next_next_chunk = NULL;
	if((void *)chunk_ptr + chunk_ptr->chunk_size < heap_start_addr + 65536) {
		next_chunk = (chunk_ptr_t)((void *)chunk_ptr + chunk_ptr->chunk_size);
		/*
		printf("next_chunk: %p(next:%p; prev:%p; chunk_size=%llu; prev_chunk_size=%llu; prev_free_flag=%llu)\n",
			(void *)next_chunk,
			(void *)next_chunk->next,
			(void *)next_chunk->prev,
			(unsigned long long)next_chunk->chunk_size,
			(unsigned long long)next_chunk->pre_chunk_size,
			(unsigned long long)next_chunk->prev_free_flag);
		*/
		if((void *)next_chunk + next_chunk->chunk_size < heap_start_addr + 65536) {
			next_next_chunk = (chunk_ptr_t)((void *)next_chunk + next_chunk->chunk_size);
			/*
			printf("next_next_chunk: %p(next:%p; prev:%p; chunk_size=%llu; prev_chunk_size=%llu; prev_free_flag=%llu)\n",
				(void *)next_next_chunk,
				(void *)next_next_chunk->next,
				(void *)next_next_chunk->prev,
				(unsigned long long)next_next_chunk->chunk_size,
				(unsigned long long)next_next_chunk->pre_chunk_size,
				(unsigned long long)next_next_chunk->prev_free_flag);
			*/
			if(next_next_chunk->prev_free_flag == 1) {
				chunk_ptr->chunk_size += next_chunk->chunk_size;
				chunk_del(next_chunk);
				next_next_chunk->pre_chunk_size = chunk_ptr->chunk_size;
				//printf("merge next\n");
			}
		} else if((void *)next_chunk + next_chunk->chunk_size == heap_start_addr +
		          65536) {
			if(last_is_free) {
				chunk_ptr->chunk_size += next_chunk->chunk_size;
				chunk_del(next_chunk);
				//printf("merge last\n");
			}
		}
	}
	if (chunk_ptr->prev_free_flag == 1) {
		prev_chunk = (chunk_ptr_t)((void *)chunk_ptr - chunk_ptr->pre_chunk_size);
		/*
		printf("prev_chunk: %p(next:%p; prev:%p; chunk_size=%llu; prev_chunk_size=%llu; prev_free_flag=%llu)\n",
			(void *)prev_chunk,
			(void *)prev_chunk->next,
			(void *)prev_chunk->prev,
			(unsigned long long)prev_chunk->chunk_size,
			(unsigned long long)prev_chunk->pre_chunk_size,
			(unsigned long long)prev_chunk->prev_free_flag);
		*/
		prev_chunk->chunk_size += chunk_ptr->chunk_size;
		chunk_ptr = prev_chunk;
		/*
		if(prev_chunk->prev==NULL){
			printf("prev: %010p", prev_chunk);
			int i = 0;
			for (i = 0; i < 8; i++) {
				printf("bin[%d]",i);
				bin_show(&bin[i]);
				printf("\n");
			}
		}
		*/
		chunk_del(prev_chunk);
		if((void *)chunk_ptr + chunk_ptr->chunk_size < heap_start_addr +65536) {
			next_chunk = (chunk_ptr_t)((void *)chunk_ptr + chunk_ptr->chunk_size);
			next_chunk->pre_chunk_size = chunk_ptr->chunk_size;
		}
		//printf("merge prev\n");
	}
	return chunk_ptr;
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
	if (bytes < 0)
		return NULL;
	bytes += 40;
	if(bytes % 8 != 0)
		bytes = (bytes / 8 + 1) * 8;
	//printf("------\n");
	chunk_ptr_t new_alloc_chunk_ptr;
	if (first_alloc) {
		heap_start_addr = sbrk(65536);
		if(*(int*)heap_start_addr==-1)
			return NULL;
		new_alloc_chunk_ptr = heap_start_addr;
		new_alloc_chunk_ptr->chunk_size = bytes;
		new_alloc_chunk_ptr->pre_chunk_size = 0;
		new_alloc_chunk_ptr->prev_free_flag = 0;
		chunk_ptr_t new_free_chunk_ptr = (chunk_ptr_t)((void *)new_alloc_chunk_ptr +
		                                 bytes);
		new_free_chunk_ptr->chunk_size = 65536 - bytes;
		new_free_chunk_ptr->pre_chunk_size = bytes;
		new_free_chunk_ptr->prev_free_flag = 0;
		int i = 0;
		for (i = 0; i < 8; i++)
			bin_init(&bin[i]);
		bin_add_chunk(&bin[6],new_free_chunk_ptr);
		last_is_free = 1;
		first_alloc = 0;
	} else {
		int which_bin = bin_choose(bytes);
		//printf("which_bin: %d\n", which_bin);
		if (which_bin == 6) {
			/*Allocate from bin 6*/
			//printf("a1\n");
			new_alloc_chunk_ptr = chunk_split(bytes);
			if(new_alloc_chunk_ptr == NULL)
				return NULL;
			//printf("a2\n");
			chunk_del(new_alloc_chunk_ptr);
			//printf("a4\n");
		} else if (bin_is_empty(&bin[which_bin])) {
			/*No free chunk in the choosen bin, so allocate from bin 6*/
			//printf("b\n");
			new_alloc_chunk_ptr = chunk_split(bytes);
			if(new_alloc_chunk_ptr == NULL)
				return NULL;
			chunk_del(new_alloc_chunk_ptr);
		} else {
			/*Allocate from the bin other than bin 6*/
			//printf("c\n");
			new_alloc_chunk_ptr = chunk_del(bin[which_bin].next);
			if(new_alloc_chunk_ptr == NULL)
				return NULL;
			new_alloc_chunk_ptr->chunk_size = bytes;
		}
		if((void *)new_alloc_chunk_ptr + new_alloc_chunk_ptr->chunk_size <
		   heap_start_addr +65536) {
			chunk_ptr_t next_chunk = (chunk_ptr_t)((void *)new_alloc_chunk_ptr +
			                                       new_alloc_chunk_ptr->chunk_size);
			next_chunk->prev_free_flag = 0;
		} else if((void *)new_alloc_chunk_ptr + new_alloc_chunk_ptr->chunk_size ==
		          heap_start_addr + 65536) {
			last_is_free = 0;
		}
	}
	bin_add_chunk(&bin[7], new_alloc_chunk_ptr);
	return (void *)new_alloc_chunk_ptr + 40;
}

int hw_free(void *mem)
{
	if (mem == NULL || mem < heap_start_addr || mem > heap_start_addr + 65536)
		return 0;
	int found=0;
	chunk_ptr_t about_to_free = bin[7].next;
	while(about_to_free != (chunk_ptr_t)&bin[7]) {
		if(about_to_free == (chunk_ptr_t)(mem - 40)) {
			found=1;
			break;
		}
		about_to_free = about_to_free->next;
	}
	if(!found)
		return 0;
	chunk_del(about_to_free);
	about_to_free = chunk_merge(about_to_free);
	//printf("about_to_free: %010p\n", about_to_free);
	//printf("about_to_free_size: %llu\n", about_to_free->chunk_size);
	if((void *)about_to_free + about_to_free->chunk_size < heap_start_addr +
	   65536) {
		chunk_ptr_t next_chunk = (chunk_ptr_t)((void *)about_to_free +
		                                       about_to_free->chunk_size);
		//printf("next: %010p\n", next_chunk);
		//printf("prev_free_flag: %llu\n", next_chunk->prev_free_flag);
		next_chunk->prev_free_flag = 1;
		//printf("prev_free_flag: %llu\n", next_chunk->prev_free_flag);
	} else if((void *)about_to_free + about_to_free->chunk_size == heap_start_addr +
	          65536) {
		last_is_free = 1;
	}
	int which_bin = bin_choose(about_to_free->chunk_size);
	bin_add_chunk(&bin[which_bin], about_to_free);
	return 1;
}

void *get_start_sbrk(void)
{
	/*
	int i = 0;
	for (i = 0; i < 8; i++) {
		printf("bin[%d]",i);
		bin_show(&bin[i]);
		printf("\n");
	}
	hw_free(heap_start_addr + 40*2 +8);
	for (i = 0; i < 8; i++) {
		printf("bin[%d]",i);
		bin_show(&bin[i]);
		printf("\n");
	}
	brk(heap_start_addr);
	*/
	return heap_start_addr;
}
void hw_print_bin(int which_bin)
{
	chunk_ptr_t current = bin[which_bin].next;
	while (current != (chunk_ptr_t)&bin[which_bin]) {
		if((void *)current == heap_start_addr)
			printf("0x00000000--------%llu\n", current->chunk_size);
		else
			printf("%010p--------%llu\n",
			       (void *)current - (unsigned long long)heap_start_addr, current->chunk_size);
		current = current->next;
	}
}
