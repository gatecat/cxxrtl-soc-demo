libbase-$(CONFIG_WITH_SDRAM) += memtest.o
libbase-$(CONFIG_WITH_SDRAM) += system.o

liblitedram-$(CONFIG_WITH_SDRAM) += bist.o
liblitedram-$(CONFIG_WITH_SDRAM) += sdram.o

# memtest.o requires an implementation of __udivdi3 and __muldi3
crt-$(CONFIG_WITH_SDRAM) += udivdi3.o
crt-$(CONFIG_WITH_SDRAM) += muldi3.o
crt-$(CONFIG_WITH_SDRAM) += clzdi2.o
crt-$(CONFIG_WITH_SDRAM) += clzsi2.o
crt-$(CONFIG_WITH_SDRAM) += lshrdi3.o
crt-$(CONFIG_WITH_SDRAM) += ashldi3.o
