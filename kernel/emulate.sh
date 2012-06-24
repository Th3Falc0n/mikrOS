cd ../modules/csh
make -B

cd ../../kernel
make -B

mount -oloop boot.img /mnt

rm /mnt/*

cp kernel /mnt
cp ../modules/csh/csh.elf /mnt
cp menu.lst /mnt/grub

umount /mnt

qemu -fda boot.img -monitor stdio
