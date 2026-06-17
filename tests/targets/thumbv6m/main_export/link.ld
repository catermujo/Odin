MEMORY {
	FLASH(rx) : ORIGIN = 0x10000000, LENGTH = 0x00040000
	SRAM(rwx) : ORIGIN = 0x20000000, LENGTH = 0x00010000
}

ENTRY(reset_handler)

__stack_top = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS {
	.vectors : {
		KEEP(*(.vectors))
	} > FLASH

	.text : {
		*(.text*)
		*(.rodata*)
		. = ALIGN(4);
	} > FLASH

	/DISCARD/ : {
		*(.comment*)
		*(.note*)
		*(.eh_frame*)
		*(.ARM.exidx*)
		*(.ARM.extab*)
	}
}
