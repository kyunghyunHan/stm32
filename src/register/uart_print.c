#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

/*
 * STM32F429ZI USART3
 * HSI / APB1 = 16MHz
 * Baudrate = 9600, 8N1
 *
 * PD8 = TX (AF7)
 * PD9 = RX (AF7)
 *
 * ST-LINK Virtual COM Port
 */

#define RCC_BASE     0x40023800UL
#define GPIOD_BASE   0x40020C00UL
#define USART3_BASE  0x40004800UL

// RCC
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIOD
#define GPIOD_MODER (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_AFRH  (*(volatile uint32_t *)(GPIOD_BASE + 0x24UL))

// USART3
#define USART3_SR  (*(volatile uint32_t *)(USART3_BASE + 0x00UL))
#define USART3_DR  (*(volatile uint32_t *)(USART3_BASE + 0x04UL))
#define USART3_BRR (*(volatile uint32_t *)(USART3_BASE + 0x08UL))
#define USART3_CR1 (*(volatile uint32_t *)(USART3_BASE + 0x0CUL))
#define USART3_CR2 (*(volatile uint32_t *)(USART3_BASE + 0x10UL))
#define USART3_CR3 (*(volatile uint32_t *)(USART3_BASE + 0x14UL))

void uart3_init(void)
{
    // 1. GPIOD Clock Enable
    RCC_AHB1ENR |= (1U << 3);

    // 2. USART3 Clock Enable
    RCC_APB1ENR |= (1U << 18);

    // 3. PD8, PD9 Alternate Function Mode
    GPIOD_MODER &= ~((3U << 16) | (3U << 18));
    GPIOD_MODER |=  ((2U << 16) | (2U << 18));

    // 4. AF7 = USART3
    GPIOD_AFRH &= ~((0xFU << 0) | (0xFU << 4));
    GPIOD_AFRH |=  ((7U << 0) | (7U << 4));

    // 5. USART 초기화
    USART3_CR1 = 0;
    USART3_CR2 = 0;
    USART3_CR3 = 0;

    // 6. 9600bps (PCLK1 = 16MHz)
    USART3_BRR = 1667U;

    // 7. USART Enable, TX Enable, RX Enable
    USART3_CR1 = (1U << 13) | (1U << 3) | (1U << 2);
}

// 문자 1개 송신
void uart3_send_char(char c)
{
    while (!(USART3_SR & (1U << 7)))
    {
    }

    USART3_DR = (uint8_t)c;
}

// 문자열 송신
void uart3_send_string(const char *str)
{
    while (*str)
    {
        uart3_send_char(*str++);
    }
}

// printf 형식으로 UART 출력
void Uart3_Printf(const char *fmt, ...)
{
    char buffer[128];

    va_list args;

    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len < 0)
    {
        return;
    }

    uart3_send_string(buffer);
}

int main(void)
{
    uart3_init();

    Uart3_Printf("\r\n===== HEAP TEST =====\r\n");

    int *p = NULL;
    void *blocks[4] = {0};

    // Heap 메모리 4번 할당
    for (int i = 0; i < 4; i++)
    {
        p = (int *)calloc(2, 4);

        if (p == NULL)
        {
            Uart3_Printf("calloc failed!\r\n");
            break;
        }

        blocks[i] = p;

        Uart3_Printf(
            "HEAP[%d]: %p\r\n",
            i,
            (void *)p
        );
    }

    Uart3_Printf("\r\n===== MEMORY DUMP =====\r\n");

    /*
     * 원본 코드의 0x20000940 주소를 그대로 사용한다.
     *
     * 주의:
     * 이 주소가 실제 할당된 Heap 영역이라는 보장은 없다.
     * 아래 루프는 주소 주변의 RAM을 직접 읽는 실험이다.
     */
    uintptr_t base = 0x20000940UL;

    for (int i = -2; i < 14; i++)
    {
        uintptr_t address = base + (intptr_t)(i * 4);
        volatile uint32_t *q = (volatile uint32_t *)address;

        Uart3_Printf(
            "0x%08" PRIXPTR ": 0x%08" PRIX32 "\r\n",
            address,
            *q
        );
    }

    // 할당한 Heap 메모리 해제
    for (int i = 0; i < 4; i++)
    {
        free(blocks[i]);
    }

    while (1)
    {
    }
}