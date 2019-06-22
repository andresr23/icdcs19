#include <stdio.h>
#include <time.h>

#include "../include/cell.h"

/*
 * When configuring the PMCs by writing to their respective
 * MSRs, they can be accessed in the order in which they were
 * assigned in the MSRs, the first MSR can be accessed by 0x00
 * the second one by 0x01 and so on...
 */
#define PMC_0 0x0 /* Requests to L2 Group1 		                  (0x60) */
#define PMC_1 0x1 /* Core to L2 Cacheable Request Access Status (0x64) */
#define PMC_2 0x2 /* Cycles not in Halt   		                  (0x76) */
#define PMC_3 0x3 /* Fill Pending from L2                              */

unsigned int buffer[L2_SECTORS][2] = {};

void
save_results(){
	FILE *f = fopen("zenaccess.dat", "w");
	for(int u = 0; u < L2_SECTORS; u++)
		fprintf(f, "%u %u\n", buffer[u][0], buffer[u][1]);
	fclose(f);
}

void
zenAccess(){
	srand(time(0));
	int set = rand() % PAGE_SETS;
	for(int u = 0; u < L2_SECTORS; u++){
		load_X(set);
		prime_L2_set(u, set);
		buffer[u][0] = pmc_X(set, PMC_1);
		load_X(set);
		prime_L2_set(u, set);
		buffer[u][1] = pmc_X(set, PMC_3);
	}
}

int
main(int argc, char *argv[]){
	unsigned long l2_s = 0;
	/* Take user input to select the sector where X should be stored in L2. */
	if(argc > 1){
		l2_s = atoi(argv[1]);
		l2_s = (l2_s > L2_SECTORS) ? L2_SECTORS : l2_s;
	}
	/* Allocate X in the L2 sector selected. */
	if(create_X(l2_s)){
		printf("[ZenAccess] X could not be allocated.\n");
		return;
	}
	printf("[ZenAccess] Created X.\n");
	/* Allocate an L2 AMES. */
	if(create_L2_AMES()){
		printf("[ZenAccess] L2 AMES could not be allocated.\n");
		return;
	}
	printf("[ZenAccess] Created AMES.\n");
	/* Experiment. */
	zenAccess();
	/* Free memory. */
	destroy_X();
	destroy_L2_AMES();
	/* Save results. */
	save_results();
}
