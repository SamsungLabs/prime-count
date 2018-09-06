#!/bin/sh

rm -rf build/

# clean app
cd src/user/
rm -f main
cd ../../

# clean kernel module
cd src/kernel/
make clean
cd ../../
