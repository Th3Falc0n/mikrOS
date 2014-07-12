MODS = $(dir $(shell find ./modules/ -name 'Makefile'))
ALL  = $(dir $(shell find -mindepth 2 -name 'Makefile'))

.PHONY: kernel
kernel:
	make -C ./kernel/ -B || exit 1
	make -C ./lib-mikros/ -B || exit 1
	
.PHONY: modules
modules: $(MODS)
	$(foreach m,$^,make -C $(m) -B || exit 1;)

.PHONY: clean
clean: $(ALL)
	$(foreach m,$^,make -C $(m) clean;)