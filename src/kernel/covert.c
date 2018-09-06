#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/thread_info.h>
#include <linux/path.h>
#include <linux/mount.h>
#include <linux/ptrace.h>
#include <linux/pid.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/smp.h>
#include <linux/stop_machine.h>
#include <linux/capability.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/security.h>
#include <linux/kallsyms.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include "proc.h"

/*
 * Brief :  Covert Channel Attack by using NWD-SWD cache
*/

/* Logging */
#define COVERT_PRINT(...) printk(KERN_ERR "[Covert] " __VA_ARGS__)

/* global variables */
static char covertCMD[128] = {0,};
static CacheContext cctx;
static int testType;
static int testLines;
static struct kmem_cache *covertKmemCache = NULL; 
static char* covertArr[96] = {NULL,};
static int *afterResult = NULL;

static spinlock_t covertLock;
static unsigned long irqflags;

static unsigned int waySize;
static unsigned int ways;
static unsigned int sets;
static unsigned int lineSize;

/*
 * SMC Issuer
 * Sorry.. We can't disclose SMC-format for Samsung secure os.
 * Depending on your environment, you need to add code for calling SMC instruction.
 */

static inline void SendCovertSMC(int flag)
{
#ifdef __arm__
#pragma message("You need to add code here for calling SMC instruction in accordance with your environment.")
	return;
#else
#pragma message("You need to add code here for calling SMC instruction in accordance with your environment.")
	return;
#endif
}

static void SetCurrentCacheState(void)
{
	if(IsMultiCore(testType))
	{
		waySize = (cctx.l2Size / cctx.l2Ways);
		ways = cctx.l2Ways;
		sets = cctx.l2NumSets;
		lineSize = cctx.l2LineSize;
	}
	else
	{
		waySize = (cctx.l1Size / cctx.l1Ways);
		ways = cctx.l1Ways;
		sets = cctx.l1NumSets;
		lineSize = cctx.l1LineSize;
	}
}

static void PrintCacheContext(CacheContext *ctx)
{
	COVERT_PRINT("==== L1 cache spec =====");
	COVERT_PRINT("set : %d, ways : %d, line-size : %d, total-size : %d", ctx->l1NumSets, ctx->l1Ways, ctx->l1LineSize, ctx->l1Size);
	COVERT_PRINT("==== L2 cache spec ====");
	COVERT_PRINT("set : %d, ways : %d, line-size : %d, total-size : %d", ctx->l2NumSets, ctx->l2Ways, ctx->l2LineSize, ctx->l2Size);
}

static void PrintTestType(int type)
{
	COVERT_PRINT("=== Test Type ====");
	
	if(IsMultiCore(type))
	{
		COVERT_PRINT("- Multi Core");
	}
	else
	{
		COVERT_PRINT("- Single Core");
	}

	if(IsLineCountingMode(type))
	{
		COVERT_PRINT("- All Way Probing");
	}
	else
	{
		COVERT_PRINT("- One Way Probing");
	}

	if(IsCycles(type))
	{
		COVERT_PRINT("- Cycles");
	}
	else
	{
		COVERT_PRINT("- Cache Refill");
	}
}

/* [ToDo] CountCacheRefill() has a lot of if-statements. please refactor it!! */
static inline void CountCacheRefill(void)
{
	unsigned int i, j;
	unsigned int idx,retIdx;
	unsigned int cbc, cac, total;
	unsigned int *ptr = NULL;

	retIdx = 0;
	for(i=0; i<sets; i++)
	{
		idx = (i * lineSize / 4);

		if(IsMultiCore(testType))
		{
			for(j=0; j<cctx.l1Ways; j++)
			{
				FlushL1DataCache(i % cctx.l1NumSets, j);
			}
		}

		total = 0;
		for(j=0; j<ways; j++)
		{
			ptr = (unsigned int*)(covertArr[j]) + idx;
			
			asm volatile("isb\n");
		#ifdef _COVERT_ARM_V7
			asm volatile("dmb\n");
		#else
			asm volatile("dsb sy\n");
		#endif
			cbc = GetCacheRefill();
			asm volatile("isb\n");
			*(ptr) += 1;
			asm volatile("isb\n");
		#ifdef _COVERT_ARM_V7
			asm volatile("dmb\n");
		#else
			asm volatile("dsb sy\n");
		#endif
			cac = GetCacheRefill();
			asm volatile("isb\n");

			if(IsLineCountingMode(testType))
			{
				if((cac - cbc) > 0)
					total++;
			}
			else
			{
				afterResult[retIdx] = (cac - cbc);
				retIdx++;
				break;
			}
			
		}

		if(IsLineCountingMode(testType))
		{
			afterResult[retIdx] = (total / ways);
			retIdx++;
		}
	}
}

static inline void Init(void)
{
	SetCurrentCacheState();
}

static inline void Finalize(void)
{
	unsigned int i;

	for(i=0; i<(ways * 2); i++)
	{
		if(covertArr[i])
		{
			kfree(covertArr[i]);
			covertArr[i] = NULL;
		}
	}

	if(covertKmemCache)
	{
		kmem_cache_destroy(covertKmemCache);
		covertKmemCache = NULL;
	}
}

static inline void SetPMU(void)
{
	InitPMU();
	
	if(IsMultiCore(testType))
	{
		SelectL2DCacheRefill();
	}
	else
	{
		SelectL1DCacheRefill();
	}
}

static inline void Allocation(void)
{
	unsigned int i;
	int addr;

	if(covertKmemCache && covertArr[0])
	{
		return;
	}

	/* 1. create kmem cache */
	covertKmemCache = kmem_cache_create("covertKmemCache", waySize, waySize, 0, NULL);
	if(!covertKmemCache)
	{
		COVERT_PRINT("kmem_cache_create error");
		return;
	}

	/* 2. alloc memories */
	for(i=0; i<(ways * 2); i++)
	{
		/* 0 ~ ways-1 :  memory used for normal world */
		/* ways ~ ways * 2 - 1 :  memory used for secure world */
		covertArr[i] = (char*)kmem_cache_alloc(covertKmemCache, GFP_ATOMIC);
		if(covertArr[i] == NULL)
		{
			COVERT_PRINT("kmem_cache_alloc fail!!");
			return;
		}
	}

	for(i=ways; i<(ways * 2); i++)
	{
		addr = (int)virt_to_phys(covertArr[i]);
		SendCovertSMC(addr);
	}
}

static inline void Prime(void)
{
	unsigned int idx;
	unsigned int *ptr = NULL;
	unsigned int i, j;
	
	for(i=0; i<sets; i++)
	{
		idx = (i * lineSize / 4);

		/* Flush L1 cache */
		for(j=0; j<cctx.l1Ways; j++)
		{
			FlushL1DataCache(i % cctx.l1NumSets, j);
		}

		if(IsMultiCore(testType))
		{
			/* Flush L2 cache if multi core test */
			for(j=0; j<cctx.l2Ways; j++)
			{
				FlushL2DataCache(i, j);
			}
		}

		/* Access right after flushing */
		for(j=0; j<ways; j++)
		{
			ptr = (unsigned int*)(covertArr[j]) + idx;

			asm volatile("isb\n");
		#ifdef _COVERT_ARM_V7
			asm volatile("dmb\n");
		#else
			asm volatile("dsb sy\n");
		#endif
			*ptr = 8;
			asm volatile("isb\n");
		#ifdef _COVERT_ARM_V7
			asm volatile("dmb\n");
		#else
			asm volatile("dsb sy\n");
		#endif
		}
	}
}

static inline void Attack(unsigned int attackLine)
{
	/* [ToDo] support attack code for multi core. (using kthread) */
	SendCovertSMC(attackLine);
}

static inline void Count(void)
{
	/* [ToDo] Support ProbeCycle() if testType is cycle */
	CountCacheRefill();
}

static int CovertProcess(void *data)
{
	unsigned int attackLine = (unsigned int)data;

	/* 1. Set PMU */
	SetPMU();
	
	/* 2. Prime */
	Prime();

	/* 3. Attack */
	Attack(attackLine);

	/* 4. Count */
	Count();

	return 0;
}

/* proc file to communication with User Space */
static int CovertProcOpen(struct inode *inode,struct file *filp)
{
	return 0;
}
static int CovertProcClose(struct inode *inode,struct file *filp)
{
	return 0;
}

/* [ToDo]
	Allocation() and Finalize() shoud be ran on each testcase.
	To do that, kerninfo.c must be modifed too.
*/

/* [ToDo] CovertProcRead is too long. We can split it to several functions. */
static ssize_t CovertProcRead(struct file *file, char *buf, size_t count, loff_t *off)
{
	int ret;
	char tmp[128] = {0,};
	unsigned int attackLine;

	/* read cache context to share context with user */
	if(strncmp(covertCMD, CMD_READ_CACHE_CONTEXT, strlen(CMD_READ_CACHE_CONTEXT) + 1) == 0)
	{
		ret = GetCacheContext(&cctx);
		if(ret)
		{
			COVERT_PRINT("do not supported processor");
			return 0;
		}

		ret = copy_to_user(buf, &cctx, sizeof(cctx));
		if(ret)
		{
			COVERT_PRINT("copy_to_user error");
			return 0;
		}
	}
	/* test!! */
	/* [ToDo] If it is not CMD_DO_TEST, It can be handled as an error */
	else
	{
		/* 1. parse test number */
		ParseTestCmd(covertCMD, tmp, &testType, &attackLine);
		testType = ConvertTestType(testType);

		/* 2. alloc afterResult */
		testLines = GetTestLines(testType, &cctx);
		afterResult = (int*)kmalloc(testLines * sizeof(int), GFP_KERNEL);
		memset(afterResult, 0, testLines * sizeof(int));

		/* 3. Init */
		Init();
		
		/* 4. Allocation */
		Allocation();

		/* 5. prime & attack & probe */
		if(IsMultiCore(testType))
		{
			CovertProcess((void*)attackLine);
		}
		else
		{
			stop_machine(CovertProcess, (void*)attackLine, NULL);
		}

		/* 6. write result to user */
		ret = copy_to_user(buf, afterResult, testLines * sizeof(int));
		if(ret)
		{
			COVERT_PRINT("copy_to_user error");
			return 0;
		}

		/* 7. Finalize */
		//Finalize();
		kfree(afterResult);
	}
	
	return 0;
}

static ssize_t CovertProcWrite(struct file *file, const char *buf, size_t count, loff_t *data)
{
	int ret = 0;

	/* copy command only */
	memset(covertCMD, 0, sizeof(covertCMD));
	
	ret = copy_from_user(covertCMD, buf, count);
	if(ret)
	{
		COVERT_PRINT("copy_from_user error");
		return ret;
	}

	return count;
}

static struct file_operations covertOps =
{
	.owner = THIS_MODULE,
	.read = CovertProcRead,
	.write = CovertProcWrite,
	.open = CovertProcOpen,
	.release = CovertProcClose,
};

static int __init CovertInit(void)
{
	spin_lock_init(&covertLock);
	proc_create(COVERT_PROC_NAME, 0, NULL, &covertOps);

	return 0;
}

static void __exit CovertExit(void)
{
	return;
}

module_init(CovertInit);
module_exit(CovertExit);

