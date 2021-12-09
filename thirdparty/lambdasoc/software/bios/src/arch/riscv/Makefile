#FIXME
TRIPLE := riscv64-unknown-elf

CC      := $(TRIPLE)-gcc
OBJCOPY := $(TRIPLE)-objcopy

riscv-march := rv32i
ifeq ($(CONFIG_RISCV_MULDIV_SOFT), n)
riscv-march := $(riscv-march)m
endif

CPPFLAGS := -I$(src)/arch/riscv/include
CFLAGS   := -march=$(riscv-march) -mabi=ilp32
ASFLAGS  := -march=$(riscv-march) -mabi=ilp32

crt-$(CONFIG_RISCV_MULDIV_SOFT) += riscv/mulsi3.o
crt-$(CONFIG_RISCV_MULDIV_SOFT) += umodsi3.o
crt-$(CONFIG_RISCV_MULDIV_SOFT) += udivsi3.o
crt-$(CONFIG_RISCV_MULDIV_SOFT) += clzsi2.o

obj-y += boot.o
obj-y += entry.o
obj-y += trap_handler.o
