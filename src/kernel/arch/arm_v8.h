#ifndef _COVERT_ARM_V8
#define _COVERT_ARM_V8

/* ARMV8 PMU related */                                                                        
#define ARMV8_PMCR_E            (1 << 0)
#define ARMV8_PMCR_P            (1 << 1)
#define ARMV8_PMCR_C            (1 << 2) 
#define ARMV8_PMCNTENSET_EL0_EN (1 << 31) 


inline uint64_t GetCycles(void)
{
	uint64_t result = 0;
	asm volatile("MRS %0, PMCCNTR_EL0" : "=r" (result));
	return result;
}

inline uint32_t GetCacheRefill(void)
{
	uint32_t ret;
	uint32_t counter = 0 & 0x1F;
	asm volatile("MSR PMSELR_EL0, %0" : : "r" (counter));
	asm volatile("ISB");
	asm volatile("MRS %0, PMXEVCNTR_EL0" : "=r"(ret));
	return ret;
}

inline void FlushL1DataCache(uint32_t set, uint32_t way)
{
	volatile uint64_t value;
	value = 0x0000000000000000;
	value = (value | (set << 6));
	value = (value | (way << 30));
	asm volatile("DC CISW, %0\n"
		:
		: "r" (value)
		);
	asm volatile("ISB\n");
	asm volatile("DSB SY\n");
}

inline void FlushL2DataCache(uint32_t set, uint32_t way)
{
	volatile uint64_t value;
	value = 0x0000000000000000;
	value = (value | (1 << 1));
	value = (value | (set << 6));
	value = (value | (way << 28));
	asm volatile("DC CISW, %0\n"
		:
		: "r" (value)
		);
	asm volatile("ISB\n");
	asm volatile("DSB SY\n");
}

inline void armv8_ldr(void* pointer)
{
	volatile uint64_t value;
	asm volatile ("LDR %0, [%1]\n\t"
	  : "=r" (value)
	  : "r" (pointer)
	  );
}

inline void armv8_cycle_init(void)
{
	uint32_t value = 0;
	asm volatile("MRS %0, PMCR_EL0" : "=r" (value));
	value |= ARMV8_PMCR_E; 
	value |= ARMV8_PMCR_C; 
	value |= ARMV8_PMCR_P;
	asm volatile("MSR PMCR_EL0, %0" : : "r" (value));
	asm volatile("MRS %0, PMCNTENSET_EL0" : "=r" (value));
	value |= ARMV8_PMCNTENSET_EL0_EN;
	asm volatile("MSR PMCNTENSET_EL0, %0" : : "r" (value));
}

inline void armv8_pmu_reset_all(void)
{
	uint32_t value = 0;
	asm volatile("MRS %0, PMCR_EL0" : "=r" (value));
	value |= ARMV8_PMCR_C;
	value |= ARMV8_PMCR_P;
	asm volatile("MSR PMCR_EL0, %0" : : "r" (value));
}

#endif
