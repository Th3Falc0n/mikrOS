#!/bin/bash

make kernel || exit 1
make modules || exit 1
make clean

mount -oloop bin/boot.img /mnt

rm /mnt/* > /dev/null

cp kernel/kernel /mnt
cp modules/*.elf /mnt
cp menu.lst /mnt/grub

umount /mnt

rm bin/kernel.objdump
objdump -dS kernel > bin/kernel.objdump

qemu-system-i386 -fda bin/boot.img -m 128 -d int --monitor stdio --no-reboot --no-shutdown
# 2>&1 | grep "check_exception" -A 20
