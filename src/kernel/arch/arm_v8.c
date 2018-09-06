#include "../proc.h"

void InitPMU(void)
{
	uint32_t value = 0;
	asm volatile("MRS %0, PMCR_EL0" : "=r" (value));
	value |= ARMV8_PMCR_E;
	value |= ARMV8_PMCR_C;
	value |= ARMV8_PMCR_P;
	asm volatile("MSR PMCR_EL0, %0" : : "r" (value));
}

void SelectL1DCacheRefill(void)
{
	uint32_t value = 0;
	value = 1 << 0;
	asm volatile("MSR PMCNTENSET_EL0, %0" : : "r" (value));
	value = 0 & 0x1F;
	asm volatile("MSR PMSELR_EL0, %0" : : "r" (value));
	value = L1D_CACHE_REFILL;
	asm volatile("MSR PMXEVTYPER_EL0, %0" : : "r" (value));
}

void SelectL2DCacheRefill(void)
{
	uint32_t value = 0;
	value = 1 << 0;
	asm volatile("MSR PMCNTENSET_EL0, %0" : : "r" (value));
	value = 0 & 0x1F;
	asm volatile("MSR PMSELR_EL0, %0" : : "r" (value));
	value = L2D_CACHE_REFILL;
	asm volatile("MSR PMXEVTYPER_EL0, %0" : : "r" (value));
}

void DataCacheEnable(void)
{
	uint64_t value = 0;
	asm volatile("MRS %0, S3_1_C15_C2_0" : "=r" (value));
	value |= 0xE0;
	asm volatile("MSR S3_1_C15_C2_0, %0" : : "r" (value));
}

void DataCacheDisable(void)
{
	uint64_t value = 0;
	asm volatile("MRS %0, S3_1_C15_C2_0" : "=r" (value));
	value &= 0xffffffffffff1fff;
	asm volatile("MSR S3_1_C15_C2_0, %0" : : "r" (value));
}