#include "../include/cell.h"

#define UNUSED_SIZE_X 128

struct cell *X;

struct cell *UNUSED_X[UNUSED_SIZE_X];

int
create_X(int l2_s){
  unsigned long frame;
  int unused_count = 0;
  int done = 0;
  /* Allocate Memory for X until obtaining a page in the requested L2 sector. */
  while(1){
    X = allocate_page();
    /* Touch X to actually allocate the virtual page. */
    X[0].delta = 0xff;
    /* Access the /proc/pid/pagemap file to retrieve the mapping of X. */
    if(pagemap(X, &frame)){
      printf("[X] Pagemap failed.\n");
      return -1;
    }
    /* The frame is cropped to only represent the L2 sector where X is located. */
    frame &= 0x0fUL;
    /* If the sector of X is equal to the one requested, the function is done. */
    if(frame == (unsigned long) l2_s)
      break;
    else{
      /* Abort if the program has allocated an excessive amount of memory */
      if(unused_count >= UNUSED_SIZE_X){
        /* Free all memory and terminate. */
        for(int i = 0; i < unused_count; i++)
          free(UNUSED_X[i]);
        free(X);
        printf("[X] Memory Protection.\n");
        return -1;
      }
      else
        UNUSED_X[unused_count++] = X;
    }
  }
  /* Make pointers for each set to point to itself. */
  for(int s = 0; s < PAGE_SETS; s++)
    X[s].next = (void *) &X[s].next;
  /* Free unused memory */
  for(int i = 0; i < unused_count; i++)
    free(UNUSED_X[i]);
  return 0;
}

void
destroy_X(){
  free(X);
}
