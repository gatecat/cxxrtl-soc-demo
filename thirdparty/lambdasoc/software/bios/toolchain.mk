CFLAGS   += -Os -Wall
CPPFLAGS += -I$(build)/include -I$(src)/include -MMD
LDFLAGS  := -nostdlib -T$(obj)/link.ld

PYTHON   := python3
MSCIMG   := $(PYTHON) util/mkmscimg.py
ifeq ($(CONFIG_CPU_BYTEORDER), "little")
MSCIMG   += --little
endif

objs.o   := $(filter %.o,$(objs))
objs.ld  := $(filter %.ld,$(objs))
deps     := $(objs.o:.o=.d) $(objs.ld:.ld=.d)

ifdef crt-y
include compiler_rt.mk
endif

liblitex-y := $(libbase-y)
liblitex-y += $(liblitedram-y)

ifdef liblitex-y
include litex.mk
endif

-include deps

$(obj)/%.ld: $(src)/%.ld.S
	$(CPP) $(CPPFLAGS) -P -o $@ $<

$(obj)/%.o: $(src)/%.c
	$(COMPILE.c) -o $@ $<

$(obj)/%.o: $(src)/%.S
	$(COMPILE.S) -o $@ $<

$(obj)/bios.elf: $(objs)
	$(LINK.o) -o $@ $(objs.o) $(LDLIBS)

$(obj)/bios.bin: $(obj)/bios.elf
	$(OBJCOPY) -O binary $< $@
	$(MSCIMG) $@
