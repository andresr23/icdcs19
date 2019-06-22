#include <stdio.h>
#include <stdlib.h>

/*
 * /proc/pid/pagempap is a virtual file that provides
 * information about the mappings between the virtual and
 * physical memory utilized by each process.
 * The pagemap() function takes a memory address and
 * accesses the file to retrieve the corresponding mapping of
 * the virtual page that stores the provided address.
 */
int
pagemap(void *address, unsigned long *physical){
	char path[32];
	/*
	 * The offset is calculated by dividing the value of the provided
	 * address by the size of virtual pages (4-kB in linux), then multiplying
	 * by 8, which is the size of each page table entry (in terms of bytes)
	 */
	unsigned long offset = (((unsigned long) address) / 4096UL) * 8UL;
 	/* Get the process ID of this process */
	pid_t pid = getpid();
	sprintf(path, "/proc/%u/pagemap", pid);
	FILE *file = fopen(path, "rb");
	/* Check if the file could be opened */
	if(file == NULL){
		printf("[Pagemap] Can't open /proc/pid/pagemap, Are you sudo?\n");
		return -1;
	}
	/* The provided page could not be found */
	if(fseek(file, offset, SEEK_SET) < 0){
		printf("[Pagemap] The provided virtual address is in a page that has not been allocated.\n");
		return -1;
	}
	/* Retrieve physical address */
	fread(physical, 8, 1, file);
	fclose(file);
	return 0;
}
