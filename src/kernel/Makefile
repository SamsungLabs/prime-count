# USAGE for arm
# make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- KDIR=/home/your/kernel CORTEX_VERSION=a17
#
# USAGE for arm64
# make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- KDIR=/home/your/kernel CORTEX_VERSION=a53
#

obj-m := covert_lkm.o

covert_lkm-y += covert.o
covert_lkm-y += proc.o

PWD    := $(shell pwd)

ifeq ($(CORTEX_VERSION), a17)
covert_lkm-y += arch/arm_v7.o
covert_lkm-y += arch/cortex_a17.o
endif

ifeq ($(CORTEX_VERSION), a53)
covert_lkm-y += arch/arm_v8.o
covert_lkm-y += arch/cortex_a53.o
endif

ccflags-y += -Wall
ccflags-y += -I../common

default:
	make -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod *.symvers *.order *.mod.c .*cmd .tmp*
	rm -rf arch/*.o arch/*.ko arch/*.mod arch/*.symvers arch/*.order arch/*.mod.c arch/.*cmd arch/.tmp*

