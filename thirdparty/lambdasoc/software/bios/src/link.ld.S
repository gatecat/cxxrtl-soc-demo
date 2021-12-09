#include <arch/header.ld>
#include <config.h>

MEMORY
{
	rom  (rx) : ORIGIN = CONFIG_ROM_START, LENGTH = CONFIG_ROM_SIZE
	ram (rwx) : ORIGIN = CONFIG_RAM_START, LENGTH = CONFIG_RAM_SIZE
}

SECTIONS
{
	.text :
	{
		_ftext = .;
		*entry*.o(.text)
		*(.text .stub .text.* .gnu.linkonce.t.*)
		_etext = .;
	} > rom

	.rodata :
	{
		. = ALIGN(8);
		_frodata = .;
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		*(.rodata1)

		/* Make sure the file is aligned on disk as well
		   as in memory; CRC calculation requires that. */
		FILL(0);
		. = ALIGN(8);
		_erodata = .;
	} > rom

	.bss :
	{
		. = ALIGN(8);
		_fbss = .;
		*(.dynsbss)
		*(.sbss .sbss.* .gnu.linkonce.sb.*)
		*(.scommon)
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(8);
		_ebss = .;
		_end = .;
	} > ram

	/DISCARD/ :
	{
		*(.eh_frame)
		*(.comment)
		*(.data .data.* .gnu.linkonce.d.*)
		*(.data1)
		*(.sdata .sdata.* .gnu.linkonce.s.*)
	}
}

PROVIDE(_fstack = ORIGIN(ram) + LENGTH(ram) - 8);
