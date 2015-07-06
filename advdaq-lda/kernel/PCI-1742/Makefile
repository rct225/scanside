#####################################################################################
UNAME_VERSION = $(shell uname -r)
KERNEL_VERSION = $(findstring 2.4,$(UNAME_VERSION))

CFLAGS += -Wall -Werror

ifeq ($(KERNEL_VERSION),2.4)
#####################################################################################
# 2.4 build
# 
ALL_SUB_DIRS :=
MOD_SUB_DIRS :=
SUB_DIRS     :=
MOD_LIST_NAME := MISC_MODULES

EXTRA_CFLAGS := -I ../../include -I ../include


ifeq ($(CONFIG_ADVDRV_PCI1742),m)
	M_OBJS := pci1742.o
	MI_OBJS := advinit.o advai.o advdio.o advport.o advevt.o advisr.o advproperty.o

endif
export-objs := 

include $(TOPDIR)/Rules.make
pci1742.o:$(MI_OBJS)
	$(LD) -r -o $@ $(MI_OBJS)
	-rm -f $(MI_OBJS)

else
#######################################################################
# 2.6 build
# 
# To build modules outside of the kernel tree, we run "make"
# in the kernel source tree; the Makefile these then includes this
# Makefile once again.
# This conditional selects whether we are being included from the
# kernel Makefile or not.
ifeq ($(KERNELRELEASE),)

    # Assume the source tree is where the running kernel was built
    # You should set KERNELDIR in the environment if it's elsewhere
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    # The current directory is passed to sub-makes as argument
    PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

.PHONY: modules modules_install clean

else
#     cur_path := $(shell pwd)
    # called from kernel build system: just declare what our modules are
#     CFLAGS += -I$(cur_path)../../include -I $(cur_path)../include	
    obj-m := pci1742.o
    pci1742-objs = advinit.o advai.o advisr.o advevt.o advdio.o advport.o advproperty.o
endif
endif	# end of ifeq ($(KERNEL_VERSION),2.4)

