MODS = $(dir $(shell find modules/ -name 'Makefile'))
ALL  = $(dir $(shell find -mindepth 2 -name 'Makefile'))

.PHONY: all
all: 
	make kernel || exit 1
	make modules || exit 1
	make clean

	-rm initrfs.tar
	cd initrfs; tar -cWf ../initrfs.tar *

	-rm bin/kernel.objdump
	objdump -dS kernel/kernel > bin/kernel.objdump
	
.PHONY: kernel
kernel:
	make -C kernel/ -B || exit 1
	make -C lib-mikros/ -B || exit 1
	
.PHONY: modules
modules: $(MODS)
	$(foreach m,$^,make -C $(m) -B || exit 1;)

.PHONY: clean
clean: $(ALL)
	$(foreach m,$^,make -C $(m) clean;)