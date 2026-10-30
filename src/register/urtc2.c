#include <stdint.h>

/*
조건 - HSI = 16MHz
     - APB1 = 16MHz
     USART2 = 9600bps, 8N1
     PD5 = TX, PD6 = RX
*/

#define RCC_BASE 0x40023800UL
#define GPIOD_BASE 0x40020C00UL
#define USART2_BASE 0x40004400UL

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))