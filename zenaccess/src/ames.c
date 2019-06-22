#include "../include/cell.h"

#define UNUSED_SIZE_AMES 2048

#define _W_L2 (L2_WAYS - 1)

struct cell *AMES[L2_SECTORS][L2_WAYS];
int ames_count[L2_SECTORS] = {};

struct cell *UNUSED_AMES[UNUSED_SIZE_AMES];

/* Check if there are elements pending to construct the AMES... */
int
incompleteAMES(){
	for(int i = 0; i < L2_SECTORS; i++)
		if(ames_count[i] < L2_WAYS)
			return -1;
	return 0;
}

/*
 * Allocate enough memory to create the eviction set U
 * and configure forward and backward pointers.
 */
void
create_L2_AMES(){
  unsigned long frame;
  int unused_count = 0;
  struct cell *_DUMMY;
  do{
    /*
     * If the program allocates an excessive amount of memory to create the AMES
     * terminate the program.
     */
    if(unused_count >= UNUSED_SIZE_AMES){
      printf("[AMES] Memory protection reached.\n");
      for(int i = 0; i < UNUSED_SIZE_AMES; i++)
        free(UNUSED_AMES[i]);
      return -1;
    }
    /* Allocate a new page */
    _DUMMY = allocate_page();
    _DUMMY[0].delta = 0xff;
    /* Check the frame of the new page */
    if(pagemap(_DUMMY, &frame)){
      printf("[AMES] Pagemap failed.\n");
      return -1;
    }
    frame &= 0x0fUL;
    /*
     * If the sector where this page is already complete, then the program needs
     * to continue.
     */
    if(ames_count[frame] == L2_WAYS){
      UNUSED_AMES[unused_count++] = _DUMMY;
      continue;
    }
    /* Save the dummy in the AMES array of pointers */
    AMES[frame][ames_count[frame]++] = _DUMMY;
  }while(incompleteAMES());
  /* Create the pointers for each U in the AMES */
	for(int u = 0; u < L2_SECTORS; u++){
		for(int s = 0; s < PAGE_SETS; s++){
			for(int w = 0; w < _W_L2; w++)
				AMES[u][w][s].next = (void *) &AMES[u][w + 1][s].next;
			AMES[u][_W_L2][s].next = (void *) &AMES[u][0][s].next;
			for(int w = _W_L2; w > 0; w--)
				AMES[u][w][s].prev = (void *) &AMES[u][w - 1][s].prev;
			AMES[u][0][s].prev = (void *) &AMES[u][_W_L2][s].prev;
	  }
	}
	/* Free all unused memory. */
	for(int i = 0; i < unused_count; i++)
		free(UNUSED_AMES[i]);
  return 0;
}

/* Free the memory used by the AMES */
void
destroy_L2_AMES(){
  for(int i = 0; i < L2_SECTORS; i++)
    for(int j = 0; j < L2_WAYS; j++)
      free(AMES[i][j]);
}
