SRCS = $(shell find -name '*.[cS]')
OBJS = $(addsuffix .o,$(basename $(SRCS)))

CC = clang
LD = ld

ASFLAGS = -m32
CFLAGS = -m32 -Wall -Wextra -Wno-unused-parameter -Werror -g -fno-stack-protector -nostdinc
LDFLAGS = -melf_i386 -T kernel.ld

kernel: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $^

clean:
	rm $(OBJS)

.PHONY: clean
