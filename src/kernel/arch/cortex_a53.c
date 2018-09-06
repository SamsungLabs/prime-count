#include "../proc.h"

/* [ToDo] 
	Need to implement SetL1DCacheContext & SetL2DCacheContext. 
*/

static void SetL1DCacheContext(CacheContext *cctx)
{
/* [ToDo] We can calculate l1Size, l1NumSets by decoding a corresponding register value */
}

static void SetL2DCacheContext(CacheContext *cctx)
{
/* [ToDo] We can calculate l2Size, l2NumSets by decoding a corresponding register value */
}

int GetCacheContext(CacheContext *cctx)
{
	// SetL1DCacheContext(cctx);
	// SetL2DCacheContext(cctx);

	/* Set essential values for A53 */
	cctx->l1Size = 128 * 4 * 64;
	cctx->l1NumSets = 128;
	cctx->l1Ways = 4;
	cctx->l1LineSize = 64;

	cctx->l2Size = 512 * 64 * 16;
	cctx->l2NumSets = 512;
	cctx->l2LineSize = 64;
	cctx->l2Ways = 16;
	return 0;
}

