#ifndef _COVERT_ARM_V7
#define _COVERT_ARM_V7

static inline unsigned int GetCacheRefill(void)
{
	unsigned int count;
	
	/* Read PMNx Register */
	asm volatile ("MRC p15, 0, %0, C9, C13, 2\t\n": "=r" (count));
	return count;
}

static inline unsigned int GetCycles(void)
{
	unsigned int cycles;
	asm volatile ("MRC p15, 0, %0, C9, C13, 0\t\n": "=r" (cycles));
	return cycles;
}

static inline void FlushL1DataCache(int set, int way)
{
	unsigned int val;

	/* Clean & Invalidate L1 */
	val = 0x00000000;	// L1
	val = (val | (set << 6));	// set
	val = (val | (way << 30)); // way
	__asm__ __volatile__ ("MCR p15, 0, %0, c7, c14, 2\n" :: "r" (val));
	__asm__ __volatile__ ("isb\n");
	__asm__ __volatile__ ("dsb\n");
}

static inline void FlushL2DataCache(int set, int way)
{
	unsigned int val;

	/* Clean & Invalidate L2 */
	val = 0x00000002;	// L2
	val = (val | (set << 6));	// set
	val = (val | (way << 28)); // way
	__asm__ __volatile__ ("MCR p15, 0, %0, c7, c14, 2\n" :: "r" (val));
	__asm__ __volatile__ ("isb\n");
	__asm__ __volatile__ ("dsb\n");
}

#endif
