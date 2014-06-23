#!/bin/bash
cd ../../kernel
make -B || exit 1

mount -oloop boot.img /mnt

rm /mnt/* > /dev/null

cp kernel /mnt
cp ../modules/csh/csh.elf /mnt
cp menu.lst /mnt/grub

umount /mnt

rm kernel.objdump
objdump -dS kernel > kernel.objdump

qemu -fda boot.img -d int --monitor stdio --no-reboot --no-shutdown
# 2>&1 | grep "check_exception" -A 20
