SRCS = $(shell find -name '*.c')
OBJS = $(addsuffix .o,$(basename $(SRCS)))

CC = gcc -std=gnu99 -fno-leading-underscore
AR = ar

ASFLAGS = -m32
CFLAGS = -m32 -Wall -Wno-unused -Werror -g -fno-builtin -fno-stack-protector -nostdinc -I include -Os
ARFLAGS = rcs

lib-mikrOS.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm $(OBJS)

.PHONY: clean

