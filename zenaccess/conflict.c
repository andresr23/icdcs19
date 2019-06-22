#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include "../include/primeprobe.h"
#include "../include/pagemap.h"
#include "../include/conflict.h"

struct cell *_x_table = NULL;

int s, w;

/* Counter Thread */
static pthread_t cthread;
int cthread_enable = 1;
int cthread_ready = 0;
unsigned long ccount = 0;

void *
counter_thread(void *info){
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(3, &cpuset);
	pthread_setaffinity_np(cthread, sizeof(cpu_set_t), &cpuset);
	cthread_ready = 1;
	while(cthread_enable)
		ccount++;
}

/* Bring x to the L1D cache */
void
cache_x(int set){
	asm volatile(
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mfence\n\t"
		:
		: "S" (&_x_table[set].next)
		: "memory"
	);
}

int
conflict_check_pmc(int set, unsigned long pmc){
	unsigned int delta;
	asm volatile(
		"mfence\n\t"
		"rdpmc\n\t"
		"mov %%eax, %%ebx\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mfence\n\t"
		"rdpmc\n\t"
		"sub %%eax, %%ebx\n\t"
		"neg %%ebx\n\t"
		: "=b" (delta)
		: "c" (pmc), "S" (&_x_table[set].next)
		: "memory"
	);
	return delta;
}

int
conflict_check_rdtsc(int set){
	unsigned int time;
	asm volatile(
		"mfence\n\t"
		"rdtsc\n\t"
		"mov %%eax, %%ebx\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mfence\n\t"
		"rdtsc\n\t"
		"sub %%eax, %%ebx\n\t"
		"neg %%ebx\n\t"
		: "=b" (time)
		: "S" (&_x_table[set].next)
		: "memory"
	);
	return time;
}

int
conflict_check_count(int set){
	unsigned long start, stop;
	asm volatile("mfence\n\t":::);
	start = ccount;
	asm volatile(
		"mfence\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mfence\n\t"
		:
		: "S" (&_x_table[set].next)
		: "memory"
	);
	stop = ccount;
	return (int)(stop - start);
}

void
start_cthread(){
	pthread_create(&cthread, NULL, counter_thread, NULL);
	while(cthread_ready == 0){}
}

void
stop_cthread(){
	cthread_enable = 0;
	pthread_join(cthread, NULL);
}

int
conflict_prepare(){
	posix_memalign(&_x_table, L1D_OFFSET, sizeof(struct cell) * 64);
	if(_x_table == NULL)
		return -1;
	for(s = 0; s < L1D_SETS; s++){
		_x_table[s].next = &_x_table[s].prev;
		_x_table[s].prev = &_x_table[s].next;
	}
	unsigned long physical;
	int status;
	pagemap((void *)_x_table, &physical, &status);
	physical &= 0xF;
	printf("[Conflict] L2 location for X is %lu\n", physical);
	return 0;
}

void
conflict_destroy(){
	free(_x_table);
}
