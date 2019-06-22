#include <stdlib.h>
#include <stdio.h>

#include "../include/primeprobe.h"
#include "../include/pagemap.h"

#define PAGES 8

/* Global */
int p, s, w;
int i;

/*
 *Spy Table Array
 * L2 can hold up to 16 pages in 8 ways.
 */
struct cell *_pages[16][PAGES];
int _pcount[16] = {};

void
smart_prime(unsigned int page, unsigned int set, unsigned int rep){
	asm volatile(
		"l1:\n\t"
		"	mov (%%rsi), %%rsi\n\t"
		" loop l1\n\t"
		"mfence\n\t"
		:
		: "c" (rep), "S" (&_pages[page][0][set].next)
		: "memory"
	);
}

/* Naive method to check if we have all the memory we need */
int
notfilled(){
	for(i = 0; i < 16; i++){
		if(_pcount[i] < PAGES)
			return -1;
	}
	return 0;
}

int
create_spy_table(){
	/* Allocate Memory Until we can map L2 */
	unsigned long physical;
	unsigned long _sl2;
	int _check = 0;
	int status;
	struct cell *dummy;
	int _c;
	while(notfilled()){
		posix_memalign(&dummy, L1D_OFFSET, sizeof(struct cell) * 64);
		_check++;
		/* Cancel the test to avoid the system from running out of memory */
		if(_check > 2500){
			printf("[PrimeProbe] Memory Protection\n");
		 	return -1;
		}
		dummy[0].time = 0xFF; /* In Linux, we need to touch the page */
		status = -1;
		physical = 0UL;
		pagemap((void *)dummy, &physical, &status);
		if(status == -1){
			printf("[PrimeProbe] Pagemap Failed\n");
			return -1;
		}
		_sl2 = physical & 0x0FUL;
		_c = _pcount[_sl2];
		if(_c == PAGES)
			continue;
		_pages[_sl2][_c] = dummy;
		_pcount[_sl2]++;
	}
	printf("[PrimeProbe] Allocated %d Pages\n", _check);
	/* We have all the memory we need, no we create the pointers */
	int _p = PAGES - 1;
	for(p = 0; p < 16; p++){
		for(s = 0; s < 64; s++){
			for(w = 0; w < _p; w++)
				_pages[p][w][s].next = &_pages[p][w + 1][s].next;
			_pages[p][_p][s].next = &_pages[p][0][s].next;
		}
	}
	printf("[PrimeProbe] Created Pointers\n");
	return 0;
}

/* Free the used memory accordingly */
void
destroy_spy_table(void){
	for(p = 0; p < 16; p++){
		for(w = 0; w < PAGES; w++){
			if(_pages[p][w] != NULL)
				free(_pages[p][w]);
		}
	}
}
