#include "../proc.h"

/* [ToDo] 
	SetL1DCacheContext, SetL2DCacheContext can be moved to arm_v7.c.
	In other words, these are architeture-version specific function, not cortex-version specific.
*/

static void SetL1DCacheContext(CacheContext *cctx)
{
	/* L1 data cache has one option only */
	cctx->l1WT = 0;
	cctx->l1WB = 1;
	cctx->l1RA = 1;
	cctx->l1WA = 1;
	cctx->l1Size = (32 * 1024);
	cctx->l1NumSets = 0x7F + 1;
	cctx->l1Ways = 4;
	cctx->l1LineSize = 64;
}

static void SetL2DCacheContext(CacheContext *cctx)
{
	unsigned int orig = 0;
	unsigned int selectL2 = 0;
	unsigned int val = 0;
	
	/* Read original cache size selection register */
	asm volatile ("MRC p15, 2, %0, c0, c0, 0\n\t": "=r" (orig));

	/* Select L2 */
	selectL2 = (1<<1) | (0);	/* Data or unified cache && Cache Level is 2 */
	asm volatile("MCR p15, 2, %0, c0, c0, 0\n" :: "r" (selectL2));

	/* Read cache size ID */
	asm volatile("MRC p15, 1, %0, c0, c0, 0\n": "=r" (val));

	/* Restore original cache size selection register */
	asm volatile("MCR p15, 2, %0, c0, c0, 0\n" :: "r" (orig));

	/* Set common options */
	cctx->l2WT = 0;
	cctx->l2WB = 1;
	cctx->l2RA = 1;
	cctx->l2WA = 1;
	cctx->l2Ways = 16;
	cctx->l2LineSize = 64;

	/* Implementation defined options */
	/* [ToDo] We can calculate l2Size, l2NumSets by decoding register value */
	if(val == 0x701FE07A)
	{
		cctx->l2Size = (256 * 1024);
		cctx->l2NumSets = 0xFF + 1;
	}
	else if(val == 0x703FE07A)
	{
		cctx->l2Size = (512 * 1024);
		cctx->l2NumSets = 0x1FF + 1;
	}
	else if(val == 0x707FE07A)
	{
		cctx->l2Size = (1024 * 1024);
		cctx->l2NumSets = 0x3FF + 1;
	}
	else if(val == 0x70FFE07A)
	{
		cctx->l2Size = (2048 * 1024);
		cctx->l2NumSets = 0x7FF + 1;
	}
	else if(val == 0x71FFE07A)
	{
		cctx->l2Size = (4096 * 1024);
		cctx->l2NumSets = 0xFFF + 1;
	}
	else if(val == 0x73FFE07A)
	{
		cctx->l2Size = (8192 * 1024);
		cctx->l2NumSets = 0x1FFF + 1;
	}
}

int GetCacheContext(CacheContext *cctx)
{
	SetL1DCacheContext(cctx);
	SetL2DCacheContext(cctx);
	return 0;
}

