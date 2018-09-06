#ifndef _COVERT_DEFINES_H
#define _COVERT_DEFINES_H

/* smp option */
enum
{
	SINGLE_CORE = 0x0000,
	MULTI_CORE = 0x0010,
};

/* probe option */
enum
{
	SET_COUNTING_MODE = 0x0000,
	LINE_COUNTING_MODE = 0x0100,
};

/* event option */
enum
{
	CACHE_REFILL = 0x0000,
	CYCLES = 0x1000,
};

/* test numbers */
enum
{
	TEST_01 = (SINGLE_CORE | SET_COUNTING_MODE | CACHE_REFILL),
	TEST_02 = (SINGLE_CORE | LINE_COUNTING_MODE | CACHE_REFILL),
	TEST_03 = (MULTI_CORE | SET_COUNTING_MODE | CACHE_REFILL),
};

static inline int IsMultiCore(int test)
{
	return ((test & MULTI_CORE) == MULTI_CORE);
}

static inline int IsLineCountingMode(int test)
{
	return ((test & LINE_COUNTING_MODE) == LINE_COUNTING_MODE);
}

static inline int IsCycles(int test)
{
	return ((test & CYCLES) == CYCLES);
}

/* cache context */
typedef struct _CacheContext
{
	/* L1 data cache spec */
	unsigned int l1WT;	/* write through if it is 1 */
	unsigned int l1WB;	/* write back */
	unsigned int l1RA;	/* read allocate */
	unsigned int l1WA;	/* write allocate */
	unsigned int l1Size;
	unsigned int l1NumSets;
	unsigned int l1Ways;
	unsigned int l1LineSize;

	/* L2 cache spec */
	unsigned int l2WT;
	unsigned int l2WB;
	unsigned int l2RA;
	unsigned int l2WA;
	unsigned int l2Size;
	unsigned int l2NumSets;
	unsigned int l2Ways;
	unsigned int l2LineSize;
}CacheContext;

/* proc file name */
#define COVERT_PROC_NAME "covert"
#define COVERT_PROC_FULL_NAME "/proc/covert"

/* proc commands */
#define CMD_READ_CACHE_CONTEXT "read_cctx"
#define CMD_DO_TEST "test"

/* weak symbol defines */
#define __WEAK __attribute__ ((weak))

/* inline & macro functions */
static inline int GetTestLines(int testType, CacheContext *cctx)
{
	int lines;
	
	if(IsMultiCore(testType))
	{
		if(IsLineCountingMode(testType))
		{
			lines = (cctx->l2NumSets * cctx->l2Ways);
		}
		else
		{
			lines = (cctx->l2NumSets);
		}
	}
	else
	{
		if(IsLineCountingMode(testType))
		{
			lines = (cctx->l1NumSets * cctx->l1Ways);
		}
		else
		{
			lines = (cctx->l1NumSets);
		}
	}

	return lines;
}

static inline int ConvertTestType(int testType)
{
	switch(testType)
	{
		case 1:
			return TEST_01;
		case 2:
			return TEST_02;
		case 3:
			return TEST_03;
		case TEST_01:
			return 1;
		case TEST_02:
			return 2;
		case TEST_03:
			return 3;
		default:
			break;
	}

	return 0;
}

//static inline void ConvertTestCmd(int testType, unsigned int attackLine, char *cmd)
#define ConvertTestCmd(testType, attackLine, cmd)\
do\
{\
	sprintf(cmd, "%s %d %d", CMD_DO_TEST, testType, attackLine);\
}while(0)

//static inline void ParseTestCmd(char *cmd, char *out, int *testType, unsigned int *attackLine)
#define ParseTestCmd(cmd, out, testType, attackLine)\
do\
{\
	sscanf(cmd, "%s%d%d", out, testType, attackLine);\
}while(0)

#endif
