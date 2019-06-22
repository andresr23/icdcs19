#include "../include/cell.h"

extern struct cell *U[NON_INCLUSIVE_WAYS];
extern struct cell *X;

/*
 * Access 'param_d' elements of the eviction set U.
 */
void
prime_LLC_set(int set, int param_d){
	if(param_d == 0)
		return;
	void *address = (void *) &U[0][set].next;
	asm volatile(
		"loopllc:\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"loop loopllc\n\t"
		"mfence"
		:
		: "S" (address), "c" (param_d)
		: "memory"
	);
}

void
load_X(int set){
  void *address = (void *) &X[set].next;
  asm volatile(
    "mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mfence"
    :
    : "S" (address)
    : "memory"
  );
}

/*
 * Monitor the Time Stamp Counter when reloading X.
 */
unsigned int
tsc_X(int set){
	unsigned int delta_tsc;
	void *address = (void *)&X[set].next;
	asm volatile(
		"mfence\n\t"
		"rdtsc\n\t"
		"mov %%eax, %%ebx\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mfence\n\t"
		"rdtsc\n\t"
		"sub %%eax, %%ebx\n\t"
		"neg %%ebx"
		: "=b" (delta_tsc)
		: "S" (address)
		: "memory"
	);
	return ((delta_tsc > 450) ? 450 : delta_tsc);
}

/*
 * Monitor the selected PMC when reloading X.
 */
unsigned int
pmc_X(int set, int pmc){
	unsigned int delta_pmc;
	void *address = (void *)&X[set].next;
	asm volatile(
		"mfence\n\t"
		"rdpmc\n\t"
		"mov %%eax, %%ebx\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mfence\n\t"
		"rdpmc\n\t"
		"sub %%eax, %%ebx\n\t"
		"neg %%ebx"
		: "=b" (delta_pmc)
		: "S" (address), "c" (pmc)
		: "memory"
	);
	return ((delta_pmc > 450) ? 450 : delta_pmc);
}
