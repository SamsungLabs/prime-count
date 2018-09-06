#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include "defines.h"

/* [ToDo] make stress test as a separate executable */

/* global variables */
CacheContext cctx;
int testType;
int imageTransfer;
int testCount;
int testLines;
char *inputFile;
char *outputFile;
int *retArr = NULL;
int *afterResult = NULL;

static void BindCPU(int cpuid)
{
	unsigned long mask = 0;

	if(cpuid == 0) mask = 1;
	else if(cpuid == 1) mask = 2;
	else if(cpuid == 2) mask = 4;
	else if(cpuid == 3) mask = 8;
	
	if(sched_setaffinity(0, sizeof(mask), (cpu_set_t*)&mask) < 0)
	{
		printf("sched_setaffinity error\n");
		return;
	}
}

static void PrintUsage(void)
{
	printf("Test Types:\n");
	printf("	test 1: single core, set counting mode, cache refill\n");
	printf("	test 2: single core, line counting mode, cache refill\n");
	printf("	test 3: multi core, set counting mode, cache refill\n");
	printf("\n");
	printf("USAGE:\n");
	printf("	ex1) ./main [test type] [image transfer option] [test count] [output file]\n");
	printf("		./main 1 0 100 /tmp/covert.csv\n");
	printf("	ex2) ./main [test type] [image transfer option] [image transfer input file] [image transfer output file]\n");
	printf("		./main 1 1 /opt/targetfiles/01/data /tmp/01.csv\n");
}

static int RetCompare(const void *a1, const void *a2)
{
	int *ptr1 = (int*)a1;
	int *ptr2 = (int*)a2;

	if(*ptr1 > *ptr2)
		return 1;
	else if(*ptr1 < *ptr2)
		return -1;
	else
		return 0;
}

static void DoCovert(char *fname, char *cmd, unsigned int cmdLen, char *out, unsigned int outSize)
{
	int ret = 0;
	int fd = 0;
	int syscall_ret = 0;

	fd = open(fname, O_RDWR, S_IXUSR | S_IROTH);
	if(fd < 0)
	{
		printf( "fail to open : %s\n", fname);
		return;
	}

	/* Write !!! */
	syscall_ret = write(fd, cmd, cmdLen);
	if (syscall_ret < 0)
	{
		printf("fail to write : %s\n", fname); 
		return; 
	}
	/* Read Result!! */
	syscall_ret = read(fd, out, outSize);
	if (syscall_ret < 0)
	{
		printf("fail to read : %s\n", fname); 
		return; 
	}

	if(fd)
	{
		close(fd);
	}
}

static void GetCacheContext(void)
{
	DoCovert(COVERT_PROC_FULL_NAME, CMD_READ_CACHE_CONTEXT, strlen(CMD_READ_CACHE_CONTEXT) + 1, &cctx, sizeof(cctx));
}

static int GetEstimateLines(void)
{
	unsigned int i;
	int lines = 0;

	for(i=0; i<testLines; i++)
	{
		lines += afterResult[i];
	}

	return lines;
}

static void DoTest(void)
{
	unsigned int i, j, k;
	int min, max, type;
	double mean;
	char str[128] = {0,};
	FILE *fop = NULL;
	FILE *fop1 = NULL;
	char filename[10];

	fop = fopen(outputFile, "w");
	retArr = (int*)malloc(testCount * sizeof(int));
	memset(retArr, 0, testCount * sizeof(int));

	afterResult = (int*)malloc(testLines * sizeof(int));
	memset(afterResult, 0, testLines * sizeof(int));

	/* [ToDo] write raw result (afterResult) to separate direcotry */
	/* [ToDo] support image transfer */
	for(i=0; i <= testLines; i++)
	{
		snprintf(filename, sizeof(filename), "%d.csv", i);
		fop1 = fopen(filename, "w");

		for(j=0; j < testCount; j++)
		{
			type = ConvertTestType(testType);	/* convert it to simple integer */
			ConvertTestCmd(type, i, str);
			DoCovert(COVERT_PROC_FULL_NAME, str, strlen(str) + 1, afterResult, sizeof(int) * testLines);
			retArr[j] = GetEstimateLines();
		
			for(k=0; k < testLines; k++)
			{
				if(k != testLines - 1) 
				{
					snprintf(str, sizeof(str), "%d,", afterResult[k]);
				}
				else
				{
					snprintf(str, sizeof(str), "%d\n", afterResult[k]);
				}
				fwrite(str, 1, strlen(str), fop1);
			}
		}
		fclose(fop1);
		
		/* sorting to get min, max, mean */
		qsort(retArr, testCount, sizeof(int), RetCompare);
		min = retArr[0];
		max = retArr[testCount-1];
		mean = (double)(retArr[testCount/2 - 1] + retArr[testCount/2]) / (double)2;
		
		/* write result to file */
		snprintf(str, 128, "%d.csv,%d,%d,%.2f\n", i, min, max, mean);
		fwrite(str, 1, strlen(str), fop);
		printf("[%d] end, min : %d, max : %d, mean : %.2f\n", i, min, max, mean);
	}

	printf("end\n");
	free(afterResult);
	free(retArr);
	fclose(fop);
}

/* [ToDo] If input is wrong, call PrintUsage() and exit */
static int CheckInput(int argc, char **argv)
{
	if(argc != 5)
	{
		printf("incorrect number of arguments\n");
		PrintUsage();
		return -1;
	}

	testType = atoi(argv[1]);
	if(testType > 3 || testType < 1)
	{
		printf("incorrect testType\n");
		PrintUsage();
		return -1;
	}
	testType = ConvertTestType(testType);
	
	imageTransfer = atoi(argv[2]);
	if(imageTransfer == 0)
	{
		testCount = atoi(argv[3]);
		if(testCount == 0)
		{
			printf("incorrect testCount\n");
			PrintUsage();
			return -1;
		}
		outputFile = argv[4];
	}
	else if(imageTransfer == 1)
	{
		inputFile = argv[3];
		outputFile = argv[4];
	}
	else
	{
		printf("incorrect imageTransfer option\n");
		PrintUsage();
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	/* 1. Check input parameters */
	if (CheckInput(argc, argv))
	{
		return 0;
	}

	/* 2. Get cache context */
	GetCacheContext();
	testLines = GetTestLines(testType, &cctx);

	/* 3. Bind CPU */
	BindCPU(2);

	/* 4. Test!! */
	DoTest();
	
	return 0;
}
