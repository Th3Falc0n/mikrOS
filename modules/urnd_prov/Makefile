SRCS = $(shell find -name '*.c')
OBJS = $(addsuffix .o,$(basename $(SRCS)))

CC = clang
LD = ld

ASFLAGS = -m32
CFLAGS = -m32 -Wall -Wno-builtin-requires-header  -Wno-unused-parameter -Wno-unused-variable -Werror -g -fno-stack-protector -nostdinc -Iinclude -I../../lib-mikros/include
LDFLAGS = -melf_i386 -Tlink.ld

../../initrfs/ibin/urnd_prov: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ ../../lib-mikros/lib-mikrOS.a

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm $(OBJS)

.PHONY: clean

