#include "../include/cell.h"

#define _W (L1_WAYS - 1)

struct cell *U[L1_WAYS];

/*
 * Allocate a single memory page with size of 4-kB.
 */
struct cell *
allocate_page(){
  struct cell *ret;
  posix_memalign(&ret, PAGE_SIZE, sizeof(struct cell) * L1_SETS);
  return ret;
}

/*
 * Allocate enough memory to create the eviction set U
 * and configure forward and backward pointers.
 */
void
create_U(){
  /* Allocate Memory for U */
  for(int w = 0; w < L1_WAYS; w++)
    U[w] = allocate_page();
  /* Create Pointers for all 64 L1D Sets */
  for(int s = 0; s < L1_SETS; s++){
    /* Forward Pointers */
    for(int w = 0; w < _W; w++)
      U[w][s].next = (void *) &U[w + 1][s].next;
    U[_W][s].next = (void *) &U[0][s].next;
    /* Backward Pointers */
    for(int w = _W; w > 0; w--)
      U[w][s].prev = (void *) &U[w - 1][s].prev;
    U[0][s].prev = (void *) &U[_W][s].prev;
  }
}

/*
 * Free Allocated Memory.
 */
void
destroy_U(){
  for(int w = 0; w < _W; w++){
    if(U[w] != NULL)
      free(U[w]);
  }
}
