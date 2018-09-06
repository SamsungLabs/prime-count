#!/bin/sh
#
# USAGE
# ./build.sh <ARCH> <CROSS_COMPILE> <KDIR> <CORTEX_VERSION>
#
# USAGE for arm
# ./build.sh arm arm-linux-gnueabi- /home/your/kernel a17
#
# USAGE for arm64
# ./build.sh arm64 arm-linux-gnueabi- /home/your/kernel a53
#

mkdir build/

# build app
cd src/user
$2gcc -o main main.c -I../common
cp -f main ../../build/
cd ../../

# build kernel module
cd src/kernel
cp -f ../common/defines.h .
make ARCH=$1 CROSS_COMPILE=$2 KDIR=$3 CORTEX_VERSION=$4
cp -f covert_lkm.ko ../../build/
rm -f defines.h
cd ../../
