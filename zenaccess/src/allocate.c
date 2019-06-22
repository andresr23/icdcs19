#include "../include/cell.h"

/*
 * Allocate a single memory page with size of 4-kB.
 */
struct cell *
allocate_page(){
  struct cell *ret;
  posix_memalign(&ret, PAGE_SIZE, sizeof(struct cell) * L1_SETS);
  return ret;
}
