UNAME_VERSION = $(shell uname -r)
KERNEL_VERSION = $(findstring 2.4,$(UNAME_VERSION))

CFLAGS =-I . -L../lib -O2 -Wall -Wstrict-prototypes -g

TOPDIR=..
BINS=
MBINS=advdevice_bind advdevice_list advdevice_locate advdevice_setboardid advdevice_calibrate
MBINDIR=/usr/bin/
all:	$(BINS) $(MBINS)
modules:$(BINS) $(MBINS)

install:dummy
	install -d $(MBINDIR)
	install $(MBINS) $(MBINDIR)
dummy:

uninstall:
	rm -f $(MBINDIR)advdevice_bind
	rm -f $(MBINDIR)advdevice_list
	rm -f $(MBINDIR)advdevice_locate
	rm -f $(MBINDIR)advdevice_setboardid
	rm -f $(MBINDIR)advdevice_calibrate
clean:

ifeq ($(KERNEL_VERSION),2.4)
include $(TOPDIR)/Rules.make
endif
