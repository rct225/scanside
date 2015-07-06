#####################################################################################

ifeq ($(KERNEL_VERSION),2.4)
#####################################################################################
# 2.4 build
# 
-include ../.config
SUB_DIRS     := core  
ALL_SUB_DIRS := core  

MOD_SUB_DIRS := PCI-1718 PCI-1730 PCI-1735 PCI-1737 PCI-1750 PCI-1751 PCI-1752 PCI-1753 PCI-1757UP PCI-1760 core
 
MOD_IN_SUB_DIRS :=


include $(TOPDIR)/Rules.make

uninstall:
	for n in $(MOD_SUB_DIRS); do \
	$(MAKE) -C $$n modules_uninstall; done
else
##################################################################################### 
# 2.6 build
# 	
# 
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
modules: 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules 
install:
	mkdir -p ../modules
	install -d ../modules
	for n in $(obj-m);\
	do cp $$n*.ko ../modules;  \
	done
		
uninstall:
	for n in $(obj-m); do $(MAKE) -C $$n modules_uninstall; done

clean:
	for n in $(obj-m); do $(MAKE) -C $$n clean; done
	rm -fr .tmp_versions
endif	# end of ifeq ($(KERNEL_VERSION),2.4)
