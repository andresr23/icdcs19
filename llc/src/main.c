#include <stdio.h>
#include <time.h>

#include "../include/cell.h"

/*
 * Parameter D (PARAM_D) defines the number of lines to be accessed
 * per LLC set.
 */
#define PARAM_D_MIN 0
#define PARAM_D_MAX 32
#define PARAM_D_SPACE (PARAM_D_MAX - PARAM_D_MIN + 1)

unsigned int buffer[PARAM_D_SPACE][2] = {};

/*
 * When configuring the PMCs by writing to their respective
 * MSRs, they can be accessed in the order in which they were
 * assigned in the MSRs, the first MSR can be accessed by 0x00
 * the second one by 0x01 and so on...
 */
#define PMC_0 0x00 // Requests to L2 Group1, Mask: 0x80
#define PMC_1 0x01 // Requests to L2 Group1, Mask: 0x02
#define PMC_2 0x02 // Core to L2 Cacheable Request Access Status
#define PMC_3 0x03 // Cycles not in Halt

void
save_results(){
	FILE *f = fopen("llc.dat", "w");
	for(int w = 0; w < PARAM_D_SPACE; w++)
		fprintf(f, "%u %u\n", buffer[w][0], buffer[w][1]);
	fclose(f);
}

void
LLC(){
	srand(time(0));
	int set = rand() % PAGE_SETS;
	for(int w = 0; w < PARAM_D_SPACE; w++){
		load_X(set);
		prime_LLC_set(set, w);
		buffer[w][0] = tsc_X(set);
		load_X(set);
		prime_LLC_set(set, w);
		buffer[w][1] = pmc_X(set, PMC_3);
	}
}

int main(){
	unsigned long frame;
	/* Create an X page an retrieve the corresponding frame. */
	if(create_X(&frame)){
		printf("[LLC] X could not be allocated.\n");
		return;
	}
	/* Create an eviction set U congruent to X. */
	if(create_LLC_U(frame)){
		printf("[LLC] LLC eviction set could not be allocated.\n");
		return;
	}
	/* Experiment. */
	LLC();
	/* Free memory. */
	destroy_X();
	destroy_LLC_U();
	/* Save results. */
	save_results();
	return 0;
}
