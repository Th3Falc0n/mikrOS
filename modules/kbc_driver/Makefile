SRCS = $(shell find -name '*.c')
OBJS = $(addsuffix .o,$(basename $(SRCS)))

CC = i586-elf-gcc -std=gnu99 -fno-leading-underscore
LD = i586-elf-ld

ASFLAGS = -m32
CFLAGS = -m32 -Wall -g -fno-stack-protector -fno-builtin -nostdinc -Iinclude -I../../lib-mikros/include
LDFLAGS = -melf_i386 -Tlink.ld -u _start --gc-sections

../../initrfs/ibin/drivers/kbc: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ ../../lib-mikros/lib-mikrOS.a
	objdump -dSx $@ > mod.objdump

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm $(OBJS)

.PHONY: clean

