ENTRY(start)
SECTIONS
{
    . = 0x100000;
    .boot : { *(.multiboot_header) }
    .text : { *(.text) }
    .data : { *(.data) }
    .bss  : { *(.bss) }
}