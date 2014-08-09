#!/bin/bash

make kernel || exit 1
make modules || exit 1
make clean

rm initrfs.tar > /dev/null
cd initrfs
tar -cWf ../initrfs.tar * > /dev/null
cd ..

rm bin/kernel.objdump
objdump -dS kernel/kernel > bin/kernel.objdump

qemu-system-i386 -kernel kernel/kernel -initrd initrfs.tar -m 1024 --monitor stdio --no-reboot --no-shutdown
# 2>&1 | grep "check_exception" -A 20
