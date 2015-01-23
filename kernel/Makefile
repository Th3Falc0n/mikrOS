SRCS  = $(shell find -name '*.S')
SRCS += $(shell find -name '*.c')
SRCS += $(shell find -name '*.cpp')
OBJS = $(addsuffix .o,$(basename $(SRCS)))

CC = i586-elf-gcc -std=gnu99 -fno-leading-underscore
LD = i586-elf-ld

ASFLAGS = -m32
CFLAGS = -m32 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Werror -g -fno-stack-protector -nostdinc -I ./os/include
LDFLAGS = -melf_i386 -T kernel.ld

kernel: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $^
	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm $(OBJS)

