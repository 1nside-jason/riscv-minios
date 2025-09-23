OUTPUT_ARCH(riscv)
ENTRY(_entry)

SECTIONS
{
    . = 0x80000000;

    .text : {
        *(.text.entry)
        *(.text .text.*)
        etext = .;
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        _data_start = .;
        *(.data .data.*)
        _data_end = .;
    }

    .bss : {
        _bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        _bss_end = .;
        end = .;
    }

    . = ALIGN(16);
    stack_bottom = .;
    . += 4096;
    stack_top = .;

    _end = .;
}
