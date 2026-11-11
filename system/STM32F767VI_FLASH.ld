/* STM32F767VIT6: DS11532. Boot from AXI Flash, data/stack in DTCM.
 * SRAM1/SRAM2 are declared separately; not implicitly merged with DTCM.
 * Compatible with ST GCC startup_stm32f767xx.s copy/zero loops.
 */
ENTRY(Reset_Handler)
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K
  DTCM (xrw)  : ORIGIN = 0x20000000, LENGTH = 128K
  SRAM1 (xrw) : ORIGIN = 0x20020000, LENGTH = 368K
  SRAM2 (xrw) : ORIGIN = 0x2007C000, LENGTH = 16K
}
_estack = ORIGIN(DTCM) + LENGTH(DTCM);
_Min_Heap_Size = 0x200;
_Min_Stack_Size = 0x800;
SECTIONS
{
  .isr_vector : { . = ALIGN(4); KEEP(*(.isr_vector)) . = ALIGN(4); } >FLASH
  .text : {
    . = ALIGN(4);
    *(.text .text.*) *(.rodata .rodata.*)
    KEEP(*(.init)) KEEP(*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >FLASH
  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM.exidx : {
    __exidx_start = .; *(.ARM.exidx*) __exidx_end = .;
  } >FLASH
  .preinit_array : {
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } >FLASH
  .init_array : {
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } >FLASH
  .fini_array : {
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*))) KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } >FLASH
  _sidata = LOADADDR(.data);
  .data : {
    . = ALIGN(4); _sdata = .;
    *(.data .data.*) . = ALIGN(4); _edata = .;
  } >DTCM AT> FLASH
  .bss (NOLOAD) : {
    . = ALIGN(4); _sbss = .; __bss_start__ = .;
    *(.bss .bss.*) *(COMMON)
    . = ALIGN(4); _ebss = .; __bss_end__ = .;
  } >DTCM
  ._user_heap_stack (NOLOAD) : {
    . = ALIGN(8); PROVIDE(end = .); PROVIDE(_end = .);
    . += _Min_Heap_Size; . += _Min_Stack_Size; . = ALIGN(8);
  } >DTCM
  .ARM.attributes 0 : { *(.ARM.attributes) }
}
ASSERT(_estack % 8 == 0, "Stack must be 8-byte aligned")
ASSERT(SIZEOF(._user_heap_stack) + ADDR(._user_heap_stack) <= _estack,
       "DTCM data, heap and stack exceed 128 KiB")
