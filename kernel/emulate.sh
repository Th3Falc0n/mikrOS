#!/bin/bash

#lib-mikros:
make -C ../lib-mikros -B || exit 1
make -C ../lib-mikros clean

pause

rm ../modules/*.elf > /dev/null

#modules:
find ../modules/ -maxdepth 1 -type d -exec bash -c "make -C {} -B || exit 1" \;
find ../modules/ -maxdepth 1 -type d -exec bash -c "make -C {} clean" \;

#kernel
make -B || exit 1
make clean

mount -oloop boot.img /mnt

rm /mnt/* > /dev/null

cp kernel /mnt
cp ../modules/*.elf /mnt
cp menu.lst /mnt/grub

umount /mnt

rm kernel.objdump
objdump -dS kernel > kernel.objdump

qemu-system-i386 -fda boot.img -d int --monitor stdio --no-reboot --no-shutdown
# 2>&1 | grep "check_exception" -A 20
