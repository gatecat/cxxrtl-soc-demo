crt-src  := $(top)/3rdparty/compiler-rt/lib/builtins
crt-obj  := $(obj)/3rdparty/compiler-rt
crt-objs := $(addprefix $(crt-obj)/,$(crt-y))

CPPFLAGS_crt := -I$(src)/include -I$(crt-src) -D'mode(x)='
ifeq ($(CONFIG_CPU_BYTEORDER), "little")
CPPFLAGS_crt += -D_YUGA_LITTLE_ENDIAN=1
else
CPPFLAGS_crt += -D_YUGA_BIG_ENDIAN=1
endif


$(crt-obj)/%.o: CPPFLAGS = $(CPPFLAGS_crt)
$(crt-obj)/%.o: $(crt-src)/%.c
	$(COMPILE.c) -o $@ $<

$(crt-obj)/%.o: $(crt-src)/%.S
	$(COMPILE.S) -o $@ $<


$(foreach obj,$(crt-objs), \
	$(eval dirs += $(dir $(obj))))

LDFLAGS += -L$(crt-obj)
LDLIBS  += -lcompiler-rt
deps    += $(crt-objs:.o=.d)

$(crt-obj)/libcompiler-rt.a: $(crt-objs)
	$(AR) crs $@ $^

$(obj)/bios.elf: $(crt-obj)/libcompiler-rt.a
