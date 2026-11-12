OUTPUT_ARCH(mips)
ENTRY(_start)

SECTIONS
{
	. = 0;
	.n64_header : AT(0)
	{
		KEEP(*(.n64_header))
	}
	ASSERT(SIZEOF(.n64_header) == 0x40, "N64 header must be 64 bytes")

	.n64_boot 0x40 : AT(0x40)
	{
		KEEP(*(.n64_boot))
	}
	ASSERT(SIZEOF(.n64_boot) == 0xFC0, "IPL3 boot code must be 4032 bytes")

	. = 0x80000400;
	.text : AT(0x1000)
	{
		KEEP(*(.text.entry))
		*(.text .text.*)
	}

	.rodata ALIGN(16) : AT(ALIGN(LOADADDR(.text) + SIZEOF(.text), 16))
	{
		*(.rodata .rodata.*)
	}

	.data ALIGN(16) : AT(ALIGN(LOADADDR(.rodata) + SIZEOF(.rodata), 16))
	{
		*(.data .data.*)
		*(.sdata .sdata.*)
	}

	.bss ALIGN(16) (NOLOAD) :
	{
		__bss_start = .;
		*(.bss .bss.*)
		*(.sbss .sbss.*)
		*(COMMON)
		__bss_end = ALIGN(., 4);
	}

	/DISCARD/ :
	{
		*(.MIPS.abiflags)
		*(.reginfo)
		*(.comment)
		*(.note*)
		*(.eh_frame*)
		*(.got .got.*)
		*(.mdebug*)
		*(.pdr)
	}
}
