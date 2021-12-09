ifdef CONFIG_CPU_MINERVA
LITEX_CPU_DIR  := $(top)/3rdparty/litex/litex/soc/cores/cpu/minerva
CPPFLAGS_litex := -D__minerva__
else
$(error Unsupported CPU)
endif

LITEX_SW_DIR   := $(top)/3rdparty/litex/litex/soc/software
LITEX_INC_DIR  := $(top)/3rdparty/litex/litex/soc/software/include
LITEX_GEN_DIR  := $(top)/src/drivers/sdram/include

CPPFLAGS_litex += \
	-nostdinc \
	-I$(LITEX_CPU_DIR) \
	-I$(LITEX_SW_DIR) \
	-I$(LITEX_INC_DIR) \
	-I$(LITEX_INC_DIR)/base \
	-I$(LITEX_GEN_DIR) \
	-I$(litedram_dir) \
	-I$(build) \


litex-obj := $(obj)/3rdparty/litex

ifdef crt-y
liblitex-objs += $(crt-objs)
endif

ifdef libbase-y
libbase-src   := $(LITEX_SW_DIR)/libbase
libbase-obj   := $(litex-obj)/libbase
liblitex-objs += $(addprefix $(libbase-obj)/,$(libbase-y))

$(libbase-obj)/%.o: CPPFLAGS = $(CPPFLAGS_litex)
$(libbase-obj)/%.o: $(libbase-src)/%.c
	$(COMPILE.c) -o $@ $<
endif

ifdef liblitedram-y
liblitedram-src  := $(LITEX_SW_DIR)/liblitedram
liblitedram-obj  := $(litex-obj)/liblitedram
liblitex-objs    += $(addprefix $(liblitedram-obj)/,$(liblitedram-y))

$(liblitedram-obj)/%.o: CPPFLAGS = $(CPPFLAGS_litex)
$(liblitedram-obj)/%.o: $(liblitedram-src)/%.c
	$(COMPILE.c) -o $@ $<
endif


$(foreach obj,$(liblitex-objs), \
	$(eval dirs += $(dir $(obj))))

LDFLAGS += -L$(litex-obj)
LDLIBS  += -llitex
deps    += $(liblitex-objs:.o=.d)

$(litex-obj)/liblitex.a: $(liblitex-objs)
	$(AR) crs $@ $^

$(obj)/bios.elf: $(litex-obj)/liblitex.a
