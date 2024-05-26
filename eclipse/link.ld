OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(5);
    bss  PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = 1M;
    
    .text ALIGN(0x1000) : AT(ADDR(.text))
    {
        *(.text)
        *(.rodata)
    } : text
    
    .data ALIGN(0x1000) : AT(ADDR(.data))
    {
        *(.data)
    } : data
    
    .bss ALIGN(0x1000) (NOLOAD) :
    {
        *(.bss)
    } : bss

    /DISCARD/ :
    {
        *(.comment)
        *(.note)
    }
}