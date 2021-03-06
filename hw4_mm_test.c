#include "hw4_mm_test.h"

int main()
{
	/*
	printf("chunk_header size: %ld\n", sizeof(struct chunk_header));
	printf("%p\n", hw_malloc(8));
	printf("%p\n", hw_malloc(16));
	printf("%p\n", hw_malloc(24));
	printf("%p\n", hw_malloc(32));
	printf("%p\n", hw_malloc(40));
	printf("%p\n", hw_malloc(48));
	printf("%p\n", hw_malloc(56));
	printf("%p\n", hw_malloc(64));
	printf("%s\n", hw_free(NULL) == 1 ? "success" : "fail");
	printf("start_brk: %p\n", get_start_sbrk());
	int i = 0;
	for (i = 0; i < 8; i++) {
		printf("bin[%d]",i);
		bin_show(&bin[i]);
		printf("\n");
	}
	*/
	/*Read the testfile*/
	char str[32];
	char *cmd, *param;
	while(fgets(str, sizeof(str), stdin) !=NULL) {
		if (strcmp(str, "\n") == 0)
			continue;
		cmd = strtok(str, "\n");
		cmd = strtok(str, " ");
		param = strtok(NULL, " ");
		if(param==NULL) {
			continue;
		}
		if (!strcmp(cmd, "alloc")) {
			unsigned long long to_alloc_size;
			sscanf(param, "%llu", &to_alloc_size);
			printf("%010p\n", (void *)hw_malloc(to_alloc_size)-(unsigned long long)
			       get_start_sbrk());
		} else if (!strcmp(cmd, "free")) {
			void *to_free_addr;
			sscanf(param, "%p", &to_free_addr);
			printf("%s\n", hw_free((void *)get_start_sbrk() + (unsigned long long)
			                       to_free_addr) == 1 ? "success" : "fail");
		} else if(!strcmp(cmd,"print")) {
			int to_print_bin;
			char *bin, *num;
			bin=strtok(param, "[");
			if(!strcmp(bin, "bin")) {
				num = strtok(NULL, "]");
				if(num==NULL)
					continue;
				sscanf(num, "%d", &to_print_bin);
				hw_print_bin(to_print_bin);
			}
		}
	}
	/*Shrink the heap*/
	brk(get_start_sbrk());
	return 0;
}
