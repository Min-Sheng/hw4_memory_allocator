#include "hw4_mm_test.h"

int main()
{
	printf("chunk_header size: %ld\n", sizeof(struct chunk_header));
	printf("%p\n", hw_malloc(8));
	printf("%p\n", hw_malloc(8));
	printf("%p\n", hw_malloc(16));
	printf("%s\n", hw_free(NULL) == 1 ? "success" : "fail");
	printf("start_brk: %p\n", get_start_sbrk());
	return 0;
}
