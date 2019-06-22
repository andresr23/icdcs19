#include "../include/cell.h"

extern struct cell *AMES[L2_SECTORS][L2_WAYS];
extern struct cell *X;

void
prime_L2_set(int u, int set){
	void *address = (void *) &AMES[u][0][set].next;
	asm volatile(
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mfence"
		:
		: "S" (address)
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

unsigned int
pmc_X(int set, int pmc){
	unsigned long delta_pmc;
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
	return ((delta_pmc > 105) ? 105 : delta_pmc);
}
