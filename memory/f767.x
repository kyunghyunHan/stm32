/* STM32F767VI: 2 MiB Flash. Use DTCM for data/stack;
 * SRAM1/SRAM2 are not allocated by this minimal firmware. */
MEMORY
{
  FLASH : ORIGIN = 0x08000000, LENGTH = 2048K
  RAM   : ORIGIN = 0x20000000, LENGTH = 128K
}
