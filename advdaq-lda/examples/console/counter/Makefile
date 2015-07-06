####### Makefile for subdirectories #################


EXAMPLES = general pci-1780 pci-1784

.PHONY: all $(EXAMPLES)

all : $(EXAMPLES)
modules : $(EXAMPLES)
$(EXAMPLES):
	$(MAKE) --directory=$@

install : 
	for n in $(EXAMPLES); do $(MAKE) -C $$n  install || exit 1; done

clean :
	for n in $(EXAMPLES); do $(MAKE) -C $$n  clean || exit 1; done


ifeq ($(KERNEL_VERSION),2.4)
include $(TOPDIR)/Rules.make
endif
