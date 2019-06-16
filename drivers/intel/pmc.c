#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

//MODULE_LICENSE("GPL");

/* 
 * Intel's Skylake only supports the configuration of 4 PMCs per
 * physical thread, this feature can be checked by 
 * executing the 'cpuid' instruction with the
 * register %eax = 0x0000_000A and then reading
 * NumberOfPMCs = (eax & 0xFF00) >> 8;
 * To configure the PMCs it is necessary to configure the
 * respective MSRs described in Table 2-2 of:
 * Intel® 64 and IA-32 architectures software developer's manual volume 4: Model-specific registers,
 * Section 2.1 ARCHITECTURAL MSRS, Page 2-3
 */
unsigned long PMC_CTRL_0 = 0x186;
unsigned long PMC_CTRL_1 = 0x187;
unsigned long PMC_CTRL_2 = 0x188;
unsigned long PMC_CTRL_3 = 0x189;

/*
 * Memory Load Retired PMC counts Load operations in accordance to the
 * configured event mask.
 * More information can be found in Table 18-36 of:
 * Intel® 64 and IA-32 architectures software developer's manual combined volumes 3A, 3B, 3C, and 3D: System programming guide,
 * Vol 3B, Chapter 18 Performance Monitoring Events, Page 18-61
 */
unsigned long PMC_EVENT_0 = 0xD1; /* Memory Load Retired */

/* 
 * Mask Collection for the 'Memory Load Retired' PMC
 */
unsigned long PMC_MEMRET_L1HIT = 0x01;  /* L1 Hit  */
unsigned long PMC_MEMRET_L2HIT = 0x02;  /* L2 Hit  */
unsigned long PMC_MEMRET_L3HIT = 0x04;  /* L3 Hit  */
unsigned long PMC_MEMRET_L1MIS = 0x08;  /* L1 Miss */
unsigned long PMC_MEMRET_L2MIS = 0x10;  /* L2 Miss */
unsigned long PMC_MEMRET_L3MIS = 0x20;  /* L3 MIss */
unsigned long PMC_MEMRET_HILFB = 0x40;  /* Hit LFB */

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
 */
static void
enable_rdpmc(void * info){
	unsigned long cr4;
	unsigned long _m = (unsigned long) 0x100;
	cr4 = read_cr4();
	cr4 |= _m;
	write_cr4(cr4);
}

/* To enable the rdpmc instruction we need to reset the 9th
 * bit of the cr4 register, more information can be found in
 * Architecture Programmer's Manual, Volume 2: System Programming
 * section 3.1.3
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
 * Configuration Layout for Intel's IA32 Architectural MSRs in accordance to Table 2-2 of:
 * Intel® 64 and IA-32 architectures software developer's manual volume 4: Model-specific registers,
 * Section 2.1 ARCHITECTURAL MSRS, Page 2-3
 * [32:24] CounterMask = 0x00 -> Enables a user to detect multiple events on each clock cycle
 * [23] CounterMask Invert
 * [22] Enable
 * [21] Reserved
 * [20] APIC interrupt Enable = 0
 * [19] Pin Control = 0
 * [18] Edge detection = 0
 * [17] OS Mode (Ring 0) = 1
 * [16] User Mode (1,2,3)= 1
 * [15:08] Event Mask (UMask)
 * [07:00] Event Select
 */
static void
config_pmc(unsigned long pmc, unsigned long event, unsigned long mask){
	unsigned long _v0 = (event & 0xFF);       /* Event                      */
	unsigned long _m0 = (mask  & 0xFF) << 8;  /* UMask                      */
	unsigned long _ou = 0x3 << 16;            /* Count OS/User events - 11b */
	unsigned long _e0 = 0x1 << 22;            /* Enable                     */
	_e0 |= (_v0 | _m0 | _ou);
	write_msr(pmc, _e0);
}

/*
 * Reset the configuration to all zeros.
 */
static void
reset_pmc(unsigned long pmc){
	write_msr(pmc, 0x0);
}

/* 
 * Installation function, here the module configures
 * each Configuration MSR with the corresponding values
 * of a PMC event and mask.
 * Addtionally, the function enables the 'rdpmc'
 * instruction.
 */
static int __init
msr_init(void){
	config_pmc(PMC_CTRL_0, PMC_EVENT_0, PMC_MEMRET_L1HIT);
	config_pmc(PMC_CTRL_1, PMC_EVENT_0, PMC_MEMRET_L1MIS);
	config_pmc(PMC_CTRL_2, PMC_EVENT_0, PMC_MEMRET_L2HIT);
	config_pmc(PMC_CTRL_3, PMC_EVENT_0, PMC_MEMRET_L2MIS);
	enable_rdpmc(NULL);
	return 0;
}

/* 
 * Removal function, here the module reverts all the 
 * changes made to the Configuration MSRs and disables
 * the 'rdpmc' instruction.
 */
static void __exit
msr_exit(void){
	reset_pmc(PMC_CTRL_0);
	reset_pmc(PMC_CTRL_1);
	reset_pmc(PMC_CTRL_2);
	reset_pmc(PMC_CTRL_3);
	disable_rdpmc(NULL);
}

module_init(msr_init);
module_exit(msr_exit);
