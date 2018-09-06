#ifndef _COVERT_PROC_H
#define _COVERT_PROC_H

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

#include "defines.h"

/* 
   inline function is used for performance-critical functions.
   inline function is implemented at header file.
   e.g) cache flush, get event count, get cycles, ...
*/
#ifdef __arm__
	#include "arch/arm_v7.h"
#else
	#include "arch/arm_v8.h"
#endif

/* architecture version specific functions (ARMv7, ARMv8) */
void InitPMU(void);
void SelectL1DCacheRefill(void);
void SelectL2DCacheRefill(void);
void DataCacheEnable(void);
void DataCacheDisable(void);

/* cortex version specific functions (Cortex-A17) */
int GetCacheContext(CacheContext *cctx);

#endif
