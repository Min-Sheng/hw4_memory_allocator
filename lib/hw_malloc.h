#ifndef HW_MALLOC_H
#define HW_MALLOC_H

#include <stddef.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>

typedef struct chunk_header* chunk_ptr_t ;
typedef unsigned long long chunk_size_t, chunk_flag_t ;

struct chunk_header {
	chunk_ptr_t prev;
	chunk_ptr_t next;
	chunk_size_t chunk_size;
	chunk_size_t pre_chunk_size;
	chunk_flag_t prev_free_flag;
};

typedef struct {
	chunk_ptr_t prev;
	chunk_ptr_t next;
}bin_t;

bin_t bin[7];

void* heap_start_addr;

void bin_init(bin_t *bin_);
int bin_is_empty(bin_t *bin_);
void bin_add_chunk(bin_t *bin_, chunk_ptr_t chunk_ptr);
void bin_show(bin_t *bin_);

extern void *hw_malloc(size_t bytes);
extern int hw_free(void *mem);
extern void *get_start_sbrk(void);

#endif
