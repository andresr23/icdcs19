#include "../include/cell.h"

struct cell *X;

/*
 * Allocate any virtual page X and retrieve it's physical frame.
 */
int
create_X(unsigned long *frame){
  int unused_count = 0;
  /* Allocate Memory for X stored in any LLC Sector. */
  X = allocate_page();
  /* Touch X to actually allocate the virtual page. */
  X[0].delta = 0xff;
  /* Access the /proc/pid/pagemap file to retrieve the mapping of X. */
  if(pagemap(X, frame)){
    printf("[X] Pagemap failed.\n");
    return -1;
  }
  /* Make pointers for each set to point to itself. */
  for(int s = 0; s < PAGE_SETS; s++)
    X[s].next = (void *) &X[s].next;
  return 0;
}

void
destroy_X(){
  free(X);
}
