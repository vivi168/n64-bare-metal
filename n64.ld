OUTPUT_ARCH(mips)
ENTRY(_start)

SECTIONS
{
	.n64_header : AT(0)
	{
		KEEP(*(.n64_header))
	}
	ASSERT(SIZEOF(.n64_header) == 0x40, "N64 header must be 64 bytes")

	.n64_boot : AT(0x40)
	{
		KEEP(*(.n64_boot))
	}
	ASSERT(SIZEOF(.n64_boot) == 0xFC0, "IPL3 boot code must be 4032 bytes")

	__ivt_ram_start = 0x80000000;

	.text 0x80000400 : AT(0x1000)
	{
		KEEP(*(.text.entry))
		*(.text .text.*)

		. = ALIGN(16);
		__ivt_start = .;

		KEEP(*(.ivt))

		__ivt_end = .;
	}

	.rodata ALIGN(16) : AT(ALIGN(LOADADDR(.text) + SIZEOF(.text), 16))
	{
		*(.rodata .rodata.*)
	}

	.data ALIGN(16) : AT(ALIGN(LOADADDR(.rodata) + SIZEOF(.rodata), 16))
	{
		__data_start = .;

		*(.data .data.*)
	}

	.sdata ALIGN(16) : AT(ALIGN(LOADADDR(.data) + SIZEOF(.data), 16))
	{
		__gp = . + 0x8000;

		*(.sdata .sdata.*)

		__data_end = .;
	}

	.sbss ALIGN(16) (NOLOAD) :
	{
		__bss_start = .;

		*(.sbss .sbss.*)
		*(.scommon)
	}

	.bss ALIGN(16) (NOLOAD) :
	{
		*(.bss .bss.*)
		*(COMMON)

		. = ALIGN(16);
		__bss_end = .;
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
