SRCS = $(shell find -name '*.c')
OBJS = $(addsuffix .o,$(basename $(SRCS)))

CC = gcc -std=gnu99 -fno-leading-underscore
LD = ld

ASFLAGS = -m32
CFLAGS = -m32 -Wall -fno-builtin -Wno-unused-parameter -Wno-unused-variable -Werror -g -fno-stack-protector -nostdinc -Iinclude -I../../lib-mikros/include
LDFLAGS = -melf_i386 -Tlink.ld

../../initrfs/ibin/memstat: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ ../../lib-mikros/lib-mikrOS.a
	objdump -dS $@ > mod.objdump

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm $(OBJS)

.PHONY: clean

