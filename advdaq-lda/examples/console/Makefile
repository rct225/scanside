####### Top Makefile for all examples #################


EXAMPLES = ad_int ad_soft ad_dma ad_dma_bm mad_int mad_soft\
	counter cnt_compare cnt_int\
	da_soft da_dma_bm da_sync\
	di_int di_pattn di_soft \
	do_soft\
	freq_in freq_out \
	mad_int mad_soft \
	port_rw \
	pulse pwm_out \
	thermo \
	tmr_cnt_set

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
