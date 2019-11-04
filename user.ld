ENTRY(_start)

PHDRS
{
	headers PT_LOAD FILEHDR PHDRS;
	text PT_LOAD;
	rodata PT_LOAD;
	data PT_LOAD;
}

SECTIONS {
	. = 0x40000000;
	.text : {
		*(.text .text.*)
		*(.gnu.linkonce.t.*)
		. = ALIGN(4096);
	} :text
	.rodata : {
		*(.rodata .rodata.* .rdata .rdata.*)
		*(.gnu.linkonce.r.*)
		__CTORS_BEGIN__ = .;
		*(.ctor*)
		__CTORS_END__ = .;
		__DTORS_BEGIN__ = .;
		*(.dtor*)
		__DTORS_END__ = .;
		. = ALIGN(4096);
	} :rodata
	.data : {
		*(.data .data.*)
		*(.gnu.linkonce.d.*)
		*(.got.plt)
		. = ALIGN(4096);
	} :data
	.bss : {
		*(COMMON)
		*(.bss .bss.*)
		. = ALIGN(4096);
	} :data
	/DISCARD/ : {
		*(.note .note.* .eh_frame)
	}
	.stab 0 : {
		*(.stab)
	}
	.stabstr 0 : {
		*(.stabstr)
	}
	.stab.excl 0 : {
		*(.stab.excl)
	}
	.stab.exclstr 0 : {
		*(.stab.exclstr)
	}
	.stab.index 0 : {
		*(.stab.index)
	}
	.stab.indexstr 0 : {
		*(.stab.indexstr)
	}
	.comment 0 : {
		*(.comment)
	}
}
