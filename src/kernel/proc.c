#include "proc.h"

/* architecture version specific functions (ARMv7, ARMv8) */
void __WEAK InitPMU(void)
{
	return;
}

void __WEAK SelectL1DCacheRefill(void)
{
	return;
}

void __WEAK SelectL2DCacheRefill(void)
{
	return;
}

void __WEAK DataCacheEnable(void)
{
	return;
}

void __WEAK DataCacheDisable(void)
{
	return;
}

/* cortex version specific functions (Cortex-A17) */
int __WEAK GetCacheContext(CacheContext *cctx)
{
	return -1;
}

