SRCS = $(shell find -name '*.c')
OBJS = $(addsuffix .o,$(basename $(SRCS)))

CC = gcc
LD = ld

ASFLAGS = -m32
CFLAGS = -m32 -Wall -g -fno-stack-protector -nostdinc -Iinclude -I../../lib-mikros/include
LDFLAGS = -melf_i386 -Tlink.ld

../csh2.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ ../../lib-mikros/lib-mikrOS.a

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm $(OBJS)

.PHONY: clean

