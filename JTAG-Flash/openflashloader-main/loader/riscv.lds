OUTPUT_ARCH( "riscv" )

SECTIONS
{
	. = 0x20000000;

	.text :
	{
		*(.text.entry)
		*(.text)
	}

	.data :
	{
		*(.data)
	}
}
