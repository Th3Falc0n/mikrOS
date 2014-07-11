MODS = $(dir $(shell find ./modules/ -name 'Makefile'))
ALL  = $(dir $(shell find -mindepth 2 -name 'Makefile'))

.PHONY: kernel
kernel:
	make -C ./kernel/ -B
	make -C ./lib-mikros/ -B
	
.PHONY: modules
modules: $(MODS)
	$(foreach m,$^,make -C $(m) -B;)

.PHONY: clean
clean: $(ALL)
	$(foreach m,$^,make -C $(m) clean;)