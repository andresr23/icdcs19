#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

/* 
 * There are six configurable PMCs per physical thread (or logical core),
 * the description of the control registers can be found in:
 * Open Source Register Reference for AMD Family 17h Processors Models 00h-2Fh,
 * Section 2.1.10.3 MSRs, page 136.
 */
unsigned long PMC_CTRL_0 = 0xC0010200;
unsigned long PMC_CTRL_1 = 0xC0010202;
unsigned long PMC_CTRL_2 = 0xC0010204;
unsigned long PMC_CTRL_3 = 0xC0010206;
unsigned long PMC_CTRL_4 = 0xC0010208;
unsigned long PMC_CTRL_5 = 0xC001020A;

/*
 * PMC identifiers and event selection masks,
 * More information about specific events can be found in:
 * Open Source Register Reference for AMD Family 17h Processors Models 00h-2Fh,
 * Section 2.1.11 PMCs, page 150.
 */
unsigned long PMC_0 = 0x060;	/* Requests to L2 Group1 			  */
unsigned long PMC_1 = 0x064;	/* Core to L2 Cacheable Request Access Status */
unsigned long PMC_2 = 0x076;	/* Cycles not in Halt   			  */
unsigned long PMC_MASK_0 = 0x80;
unsigned long PMC_MASK_1 = 0x40;
unsigned long PMC_MASK_2 = 0x00;

/* 
 * Auxiliary function to read from a MSR. 
 * The function is not required to configure the PMCs, but
 * can be used to verify the configuration.
 */
/*static unsigned long
read_msr(unsigned long msr){
	unsigned long hi, lo;
	asm volatile(
		"rdmsr\n\t"
		: "=d" (hi), "=a" (lo)
		: "c" (msr)
	);
	hi <<= 32;
	hi |= lo;
	return hi;
}*/

/*
 * Function to write to change the value of a MSR.
 */
static void
write_msr(unsigned long msr, unsigned long data){
	unsigned long hi = data >> 32;
	unsigned long lo = data & 0xFFFFFFFF;
	asm volatile(
		"wrmsr\n\t"
		:
		: "c" (msr), "d" (hi), "a" (lo)
	);
}

/*
 * Function to read the value of the x86's CR4 register.
 */
static unsigned long
read_cr4(void){
	unsigned long data;
	asm volatile(
		"mov %%cr4, %0\n\t"
		: "=r" (data)
	);
	return data;
}

/*
 * Function to write the value of x86's CR4 register.
 */
static void
write_cr4(unsigned long data){
	asm volatile(
		"mov %0, %%cr4\n\t"
		:
		: "r" (data)
	);
}

/*
 * To enable the 'rdpmc' instruction, the module needs to set the
 * 9th bit of the CR4 register.
 * More information about this can be found in:
 * AMD64 Architecture Programmer's Manual, Volume 2: System Programming,
 * Section 3.1.3, Page 47.
 *
 * Additionally, before loading the driver it is necessary to 
 * modify the configuration of the kernel via:
 * sudo -i
 * echo 2 > /sys/devices/cpu/rdpmc
 * Credit for this:
 * https://stackoverflow.com/questions/38469233/cr4-pce-for-rdpmc-is-cleared
 */
static void
enable_rdpmc(void * info){
	unsigned long cr4;
	unsigned long _m = (unsigned long) 0x100;
	cr4 = read_cr4();
	cr4 |= _m;
	write_cr4(cr4);
}

/*
 * To disable the 'rdpmc' instruction, the module resets the 
 * 9th bit of the CR4 register upon removal.
 * More information about this can be found in:
 * AMD64 Architecture Programmer's Manual, Volume 2: System Programming,
 * Section 3.1.3, Page 47.
 */
static void
disable_rdpmc(void * info){
	unsigned long cr4;
	unsigned long _m = ~((unsigned long) 0x100);
	cr4 = read_cr4();
	cr4 &= _m;
	write_cr4(cr4);
}

/*
 * This function configures the value of the PERF_CTL MSR in
 * accordance to the layout depicted in:
 * Open Source Register Reference for AMD Family 17h Processors Models 00h-2Fh,
 * Section 2.1.10.3 MSRs, page 136.
 * [63:42] - Reserved
 * [41:40] - Host/Guest, 0h for all events
 * [39:36] - Reserved
 * [35:32] - Event Select[11:08]
 * [31:24] - Count Mask, 00h for all events in a cc
 * [23]		 - Invert Counter mask
 * [22]		 - Enable Performance Counter
 * [21]		 - Reserved
 * [20]		 - APIC interrupt when overflow
 * [19]		 - Reserved
 * [18]		 - Edge detect
 * [17:16] - OS/Usermode
 * [16:08] - UnitMask
 * [07:00] - Event Select [07:00]
 */
static void
config_pmc(unsigned long pmc, unsigned long event, unsigned long mask){
	unsigned long _v0 = (event & 0xF00) << 32; /* Upper bits of event select */
	unsigned long _v1 = (event & 0x0FF);	   /* Lower bits of event select */
	unsigned long _m0 = (mask  &  0xFF) <<  8; /* Mask of the event          */
	unsigned long _ou = 0x3 << 16;		   /* Count OS/User events - 11b */
	unsigned long _e0 = 0x1 << 22;		   /* Enable                     */
	_e0 |= (_v0 | _v1 | _m0 | _ou);
	write_msr(pmc, _e0);
}

/* 
 * Reset the configuration of a PERF_CTL MSR
 * to all zeros.
 */
static void
reset_pmc(unsigned long pmc){
	write_msr(pmc, 0x0);
}

/* 
 * Installation function, here the module configures
 * each PERF_CTL MSR with the corresponding values
 * of a PMC event and mask.
 * Addtionally, the function enables the 'rdpmc'
 * instruction.
 */
static int __init
msr_init(void){
	config_pmc(PMC_CTRL_0, PMC_0, PMC_MASK_0);
	config_pmc(PMC_CTRL_1, PMC_1, PMC_MASK_1);
	config_pmc(PMC_CTRL_2, PMC_2, PMC_MASK_2);
	enable_rdpmc(NULL);
	return 0;
}

/* 
 * Removal function, here the module reverts all the 
 * changes made to the PERF_CTL MSRs and disables
 * the 'rdpmc' instruction.
 */
static void __exit
msr_exit(void){
	reset_pmc(PMC_CTRL_0);
	reset_pmc(PMC_CTRL_1);
	reset_pmc(PMC_CTRL_2);
	disable_rdpmc(NULL);
}

module_init(msr_init);
module_exit(msr_exit);
