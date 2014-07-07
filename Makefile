MFILES = $(dir $(shell find -mindepth 2 -name 'Makefile'))

.PHONY: os
os: $(MFILES)
	$(foreach m,$^,make -C $(m) -B;)
  
.PHONY: clean
clean: $(MFILES)
	$(foreach m,$^,make -C $(m) clean;)
  
