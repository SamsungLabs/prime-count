#include "../proc.h"

#define PERF_DEF_OPTS (1 | 16)
void InitPMU(void)
{	
	/* Enable user-mode access to counters. */
	asm volatile ("MCR p15, 0, %0, C9, C14, 0\n\t" :: "r" (1));	

	/* Program PMU and enable all counters */
	asm volatile("mcr p15, 0, %0, c9, c12, 0" :: "r"(PERF_DEF_OPTS));
	asm volatile("mcr p15, 0, %0, c9, c12, 1" :: "r"(0x8000000f));	
}

void SelectL1DCacheRefill(void)
{
	unsigned int value;

	/* Read original value */
	asm volatile ("mrc p15, 0, %0, c9, c13, 1\n\t": "=r" (value));

	/* Select Data Cache Refill */
	asm volatile ("mcr p15, 0, %0, c9, c13, 1\n\t" :: "r" (3));	

	/* Checking */
	asm volatile ("mrc p15, 0, %0, c9, c13, 1\n\t": "=r" (value));
}

void SelectL2DCacheRefill(void)
{
	unsigned int value;

	/* Read original value */
	asm volatile ("mrc p15, 0, %0, c9, c13, 1\n\t": "=r" (value));

	/* Select Data Cache Refill */
	asm volatile ("mcr p15, 0, %0, c9, c13, 1\n\t" :: "r" (0x17));	/* L2D_CACHE_REFILL */

	/* Checking */
	asm volatile ("mrc p15, 0, %0, c9, c13, 1\n\t": "=r" (value));
}

void DataCacheEnable(void)
{
	unsigned int val;
	asm volatile ("MRC 	p15, 0, %0, c1, c0, 0\n\t": "=r" (val));
	asm volatile (
		"orr %0, %0, #4\n"
		"dsb\n"
		:: "r" (val)
	);
	asm volatile ("MCR 	p15, 0, %0, c1, c0, 0\n\t" :: "r"(val));
}

void DataCacheDisable(void)
{
	unsigned int val;
	asm volatile ("MRC 	p15, 0, %0, c1, c0, 0\n\t": "=r" (val));
	asm volatile (
		"bic %0, %0, #4\n"
		"dsb\n"
		:: "r" (val)
	);
	asm volatile ("MCR 	p15, 0, %0, c1, c0, 0\n\t" :: "r"(val));
}
