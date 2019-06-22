#include "../include/cell.h"

#define _W_LLC (NON_INCLUSIVE_WAYS - 1)

#define UNUSED_SIZE_U 4096

struct cell *U[NON_INCLUSIVE_WAYS];

struct cell *UNUSED_U[UNUSED_SIZE_U];

/*
 * Allocate pages to fill all the ways of the L2 and LLC caches.
 */
int
create_LLC_U(unsigned long reference_frame){
  unsigned long LLC_reference_frame = reference_frame & 0x7fUL;
  unsigned long dummy_frame;
  int unused_count = 0;
  int U_count = 0;
  struct cell *_DUMMY;
  while(U_count < NON_INCLUSIVE_WAYS){
    /*
     * If the program allocates an excessive amount of memory to create U then
     * terminate the program.
     */
    if(unused_count >= UNUSED_SIZE_U){
      printf("[U] Memory protection reached.\n");
      for(int i = 0; i < UNUSED_SIZE_U; i++)
        free(UNUSED_U[i]);
      return -1;
    }
    /* Allocate a new page */
    _DUMMY = allocate_page();
    _DUMMY[0].delta = 0xff;
    /* Check the frame of the new page */
    if(pagemap(_DUMMY, &dummy_frame)){
      printf("[U] Pagemap failed.\n");
      return -1;
    }
    /* Crop the dummy frame in accordance to the LLC. */
    dummy_frame &= 0x7fUL;
    /* The '_DUMMY' page is congruent to the reference_frame w.r.t. the LLC. */
    if(dummy_frame == LLC_reference_frame)
      U[U_count++] = _DUMMY;
    else{
      UNUSED_U[unused_count++] = _DUMMY;
    }
  }
  /* Create the pointers for U. */
  for(int s = 0; s < PAGE_SETS; s++){
    for(int w = 0; w < _W_LLC; w++)
      U[w][s].next = (void *) &U[w + 1][s].next;
    U[_W_LLC][s].next = (void *) &U[0][s].next;
  }
  /* Free all unused memory. */
  for(int i = 0; i < unused_count; i++)
    free(UNUSED_U[i]);
  return 0;
}

/* Free the memory used by U. */
void
destroy_LLC_U(){
  for(int i = 0; i < NON_INCLUSIVE_WAYS; i++)
    free(U[i]);
}
