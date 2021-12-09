# SPDX-License-Identifier: GPL-2.0-or-later

# Early configuration of coreboot specific changes
Kconfig ?= $(src)/Kconfig

# Include verbatim Makefile
include $(dir $(lastword $(MAKEFILE_LIST)))Makefile

# Extend Linux kconfig build rules

# Support mingw by shipping our own regex implementation
_OS=$(shell uname -s |cut -c-7)
regex-objs=
ifeq ($(_OS),MINGW32)
	regex-objs=regex.o
endif
$(kconfig-obj)/regex.o: $(kconfig-src)/regex.c
	$(HOSTCC) $(HOSTCFLAGS) $(HOST_EXTRACFLAGS) -DHAVE_STRING_H -c -o $@ $<

conf-objs += $(regex-objs)
mconf-objs += $(regex-objs)
