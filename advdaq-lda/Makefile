DAQ_VERSION := 1.10.0001
export DAQ_VERSION

#####################################################################################
UNAME_VERSION = $(shell uname -r)
KERNEL_VERSION = $(findstring 2.4,$(UNAME_VERSION))

ifeq ($(KERNEL_VERSION),)
KERNEL_VERSION = 2.6
endif

export KERNEL_VERSION
ifeq ($(KERNEL_VERSION),2.4)
#####################################################################################
# 2.4 build
# 
VERSION = 0
PATCHLEVEL = 0
SUBLEVEL = 1
EXTRAVERSION =  

PROJECT = ADVDAS
project = advdaq

SUBDIRS := kernel
-include ./Makefile.modbuild


ADVDEVMAJOR=$(filter-out advdrv,$(shell grep advdrv /proc/devices |awk '{ print $1 }'))
dev:
	-mknod -m 666 /dev/advdaq0 c $(ADVDEVMAJOR) 0
	-mknod -m 666 /dev/advdaq1 c $(ADVDEVMAJOR) 1
	-mknod -m 666 /dev/advdaq2 c $(ADVDEVMAJOR) 2
	-mknod -m 666 /dev/advdaq3 c $(ADVDEVMAJOR) 3
	-mknod -m 666 /dev/advdaq254 c $(ADVDEVMAJOR) 254
dev-remove:
	-rm -rf /dev/advdaq[0-3]
	-rm -rf /dev/advdaq254
else
#####################################################################################
# 2.6 build
#
-include ./.config

SUBDIRS = kernel lib tools
device = advdaq
ADVDEVMAJOR = $(filter-out advdrv, $(shell grep advdrv /proc/devices | awk '{print $1}'))
PWD := $(shell pwd)


obj-$(CONFIG_ADVDRV_USB4711) += USB-4711/
obj-$(CONFIG_ADVDRV_USB4716) += USB-4716/
obj-$(CONFIG_ADVDRV_USB4718) += USB-4718/
obj-$(CONFIG_ADVDRV_USB4750) += USB-4750/
obj-$(CONFIG_ADVDRV_USB4751) += USB-4751/
obj-$(CONFIG_ADVDRV_USB4761) += USB-4761/

obj-$(CONFIG_ADVDRV_PCI1710) += PCI-1710/
obj-$(CONFIG_ADVDRV_PCI1712) += PCI-1712/
obj-$(CONFIG_ADVDRV_PCI1714) += PCI-1714/
obj-$(CONFIG_ADVDRV_PCI1716) += PCI-1716/
obj-$(CONFIG_ADVDRV_PCI1718) += PCI-1718/

obj-$(CONFIG_ADVDRV_PCI1720) += PCI-1720/
obj-$(CONFIG_ADVDRV_PCI1721) += PCI-1721/
 
obj-$(CONFIG_ADVDRV_PCI1730) += PCI-1730/
obj-$(CONFIG_ADVDRV_PCI1735) += PCI-1735/
obj-$(CONFIG_ADVDRV_PCI1737) += PCI-1737/

obj-$(CONFIG_ADVDRV_PCI1741U) += PCI-1741U/
obj-$(CONFIG_ADVDRV_PCI1742) += PCI-1742/
obj-$(CONFIG_ADVDRV_PCI1747) += PCI-1747/

obj-$(CONFIG_ADVDRV_PCI1750) += PCI-1750/
obj-$(CONFIG_ADVDRV_PCI1751) += PCI-1751/
obj-$(CONFIG_ADVDRV_PCI1752) += PCI-1752/
obj-$(CONFIG_ADVDRV_PCI1753) += PCI-1753/
obj-$(CONFIG_ADVDRV_PCI1758U)+= PCI-1758U/
obj-$(CONFIG_ADVDRV_PCI1757UP) += PCI-1757UP/
obj-$(CONFIG_ADVDRV_PCI1758U) += PCI-1758U/

obj-$(CONFIG_ADVDRV_PCI1760) += PCI-1760/
obj-$(CONFIG_ADVDRV_PCI1761) += PCI-1761/

obj-$(CONFIG_ADVDRV_PCI1780) += PCI-1780/
obj-$(CONFIG_ADVDRV_PCI1784) += PCI-1784/
 
obj-$(CONFIG_ADVDRV_PCM3718) += PCM-3718/
obj-$(CONFIG_ADVDRV_PCM3724) += PCM-3724/
obj-$(CONFIG_ADVDRV_PCM3725) += PCM-3725/
obj-$(CONFIG_ADVDRV_PCM3730) += PCM-3730/
obj-$(CONFIG_ADVDRV_PCM3730I) += PCM-3730I/
obj-$(CONFIG_ADVDRV_PCM3753I) += PCM-3753I/
obj-$(CONFIG_ADVDRV_PCM3761I) += PCM-3761I/

obj-$(CONFIG_ADVDRV_CORE) += core/

export obj-m

all: subdirs
	
subdirs:
	
	for n in $(SUBDIRS); do $(MAKE) -C $$n || exit 1; done
	

install:
	for n in $(SUBDIRS); do $(MAKE) -C $$n install; done
	-mkdir -p /usr/local/include
	-mkdir -p /usr/local/include/Advantech
	-cp include/*.h /usr/local/include/Advantech -f
	-ldconfig /usr/lib
	-depmod 

config:	dummy
	$(CONFIG_SHELL) scripts/Configure

uninstall:
	-rm -fr /usr/local/include/Advantech
	-rm -fr modules
	for n in $(SUBDIRS); do $(MAKE) -C $$n uninstall; done

dev:
	-mknod -m 666 /dev/${device}0 c ${ADVDEVMAJOR} 0
	-mknod -m 666 /dev/${device}1 c ${ADVDEVMAJOR} 1
	-mknod -m 666 /dev/${device}2 c ${ADVDEVMAJOR} 2
	-mknod -m 666 /dev/${device}3 c ${ADVDEVMAJOR} 3
	-mknod -m 666 /dev/${device}254 c ${ADVDEVMAJOR} 254

dev-remove:
	-rm -rf /dev/${device}[0-3]
	-rm -rf /dev/${device}254
clean:
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean; done
	
distclean:
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean; done
	-rm -rf modules
	-rm -f .config

dummy:
scripts/mkdep: scripts/mkdep.c
	$(HOSTCC) $(HOSTCFLAGS) -o scripts/mkdep scripts/mkdep.c

scripts/split-include: scripts/split-include.c
	$(HOSTCC) $(HOSTCFLAGS) -o scripts/split-include scripts/split-include.c



endif	# end of ifeq ($(KERNEL_VERSION),2.4)
