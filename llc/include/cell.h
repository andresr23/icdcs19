#include <stddef.h>

/* Default Page Size for Linux */
#define PAGE_SIZE 4096
#define PAGE_SETS 64

/* L1D Parameters for the Ryzen 1600x */
#define L1_SETS 64
#define L1_WAYS 8

/* L2 Parameters for the Ryzen 1600x */
#define L2_SECTORS 16
#define L2_WAYS 8

/* LLC Parameters for the Ryzen 1600x */
#define LLC_WAYS 16
#define NON_INCLUSIVE_WAYS (L2_WAYS + LLC_WAYS)

/*
 * Basic Cache Line representation,
 * There are two flavors for the struct cell configuration
 * -One option consits of creating a single array of 16 void pointers,
 * -The second option uses distinct data types (this one), and adding an array of
 *  pointers will cause the structure to be 64 bytes larger.
 */
struct cell{
	void *next;
	void *prev;
	long delta;
	long fill[5];
};
