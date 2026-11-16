// UART와 DMA로 수신한 문자를 그대로 돌려보내는 echo 실습
#include <stdint.h>

#define RCC_BASE       0x40023800UL
#define GPIOD_BASE     0x40020C00UL
#define USART3_BASE    0x40004800UL
#define DMA1_BASE      0x40026000UL

#define REG32(addr) (*(volatile uint32_t *)(addr))

#define RCC_AHB1ENR REG32(RCC_BASE + 0x30UL)
#define RCC_APB1ENR REG32(RCC_BASE + 0x40UL)

#define GPIOD_MODER REG32(GPIOD_BASE + 0x00UL)
#define GPIOD_AFRH  REG32(GPIOD_BASE + 0x24UL)

#define USART3_SR  REG32(USART3_BASE + 0x00UL)
#define USART3_DR  REG32(USART3_BASE + 0x04UL)
#define USART3_BRR REG32(USART3_BASE + 0x08UL)
#define USART3_CR1 REG32(USART3_BASE + 0x0CUL)
#define USART3_CR2 REG32(USART3_BASE + 0x10UL)
#define USART3_CR3 REG32(USART3_BASE + 0x14UL)

// DMA 상태 및 플래그 클리어
#define DMA1_LISR   REG32(DMA1_BASE + 0x00UL)
#define DMA1_LIFCR  REG32(DMA1_BASE + 0x08UL)

// Stream1 = USART3 RX
#define DMA1_S1CR    REG32(DMA1_BASE + 0x28UL)
#define DMA1_S1NDTR  REG32(DMA1_BASE + 0x2CUL)
#define DMA1_S1PAR   REG32(DMA1_BASE + 0x30UL)
#define DMA1_S1M0AR  REG32(DMA1_BASE + 0x34UL)
#define DMA1_S1FCR   REG32(DMA1_BASE + 0x3CUL)

// Stream3 = USART3 TX
#define DMA1_S3CR    REG32(DMA1_BASE + 0x58UL)
#define DMA1_S3NDTR  REG32(DMA1_BASE + 0x5CUL)
#define DMA1_S3PAR   REG32(DMA1_BASE + 0x60UL)
#define DMA1_S3M0AR  REG32(DMA1_BASE + 0x64UL)
#define DMA1_S3FCR   REG32(DMA1_BASE + 0x6CUL)

// DMA 비트
#define DMA_EN       (1UL << 0)
#define DMA_DIR_M2P  (1UL << 6)
#define DMA_CH4      (4UL << 25)

#define DMA_RX_TCIF  (1UL << 11)
#define DMA_TX_TCIF  (1UL << 27)

// Stream1, Stream3 전체 플래그 클리어 마스크
#define DMA_S1_CLEAR 0x00000F40UL
#define DMA_S3_CLEAR 0x0F400000UL

// DMA 접근이 가능한 일반 SRAM에 배치
static volatile uint8_t rx_data = 0;

void uart3_init(void)
{
    // GPIOD 클럭 활성화
    RCC_AHB1ENR |= (1UL << 3);

    // USART3 클럭 활성화
    RCC_APB1ENR |= (1UL << 18);

    // PD8 / PD9 Alternate Function
    GPIOD_MODER &= ~((3UL << 16) | (3UL << 18));
    GPIOD_MODER |=  ((2UL << 16) | (2UL << 18));

    // AF7 = USART3
    GPIOD_AFRH &= ~((0xFUL << 0) | (0xFUL << 4));
    GPIOD_AFRH |=  ((7UL << 0) | (7UL << 4));

    // USART 초기화
    USART3_CR1 = 0;
    USART3_CR2 = 0;
    USART3_CR3 = 0;

    // 16MHz / 9600bps, OVER8 = 0
    USART3_BRR = 1667UL;

    // USART 활성화, TX/RX 허용
    USART3_CR1 = (1UL << 13) |
                 (1UL << 3)  |
                 (1UL << 2);
}

void dma1_init(void)
{
    // 1. DMA1 클럭 활성화 (AHB1 bit21)
    RCC_AHB1ENR |= (1UL << 21);

    // 2. DMA Stream 비활성화
    DMA1_S1CR &= ~DMA_EN;
    DMA1_S3CR &= ~DMA_EN;

    // 실제 비활성화 완료 대기
    while (DMA1_S1CR & DMA_EN);
    while (DMA1_S3CR & DMA_EN);

    // 3. DMA 플래그 초기화
    DMA1_LIFCR = DMA_S1_CLEAR | DMA_S3_CLEAR;

    // 4. RX: USART3_DR -> RAM
    DMA1_S1PAR  = (uint32_t)(uintptr_t)&USART3_DR;
    DMA1_S1M0AR = (uint32_t)(uintptr_t)&rx_data;
    DMA1_S1NDTR = 1;

    // Channel4, Peripheral-to-Memory
    // 8bit 전송, Normal Mode, 주소 증가 없음
    DMA1_S1CR = DMA_CH4;
    DMA1_S1FCR = 0;

    // 5. TX: RAM -> USART3_DR
    DMA1_S3PAR  = (uint32_t)(uintptr_t)&USART3_DR;
    DMA1_S3M0AR = (uint32_t)(uintptr_t)&rx_data;
    DMA1_S3NDTR = 1;

    // Channel4, Memory-to-Peripheral
    // 8bit 전송, Normal Mode, 주소 증가 없음
    DMA1_S3CR = DMA_CH4 | DMA_DIR_M2P;
    DMA1_S3FCR = 0;

    // 6. USART3 DMA 요청 활성화
    // DMAR(bit6) = RX DMA
    // DMAT(bit7) = TX DMA
    USART3_CR3 |= (1UL << 6) | (1UL << 7);
}

void dma_uart3_receive(void)
{
    // RX Stream 비활성화
    DMA1_S1CR &= ~DMA_EN;
    while (DMA1_S1CR & DMA_EN);

    // 이전 완료 플래그 제거
    DMA1_LIFCR = DMA_S1_CLEAR;

    // 전송 개수 = 1바이트
    DMA1_S1NDTR = 1;

    // RX DMA 시작
    DMA1_S1CR |= DMA_EN;

    // DMA가 1바이트를 수신할 때까지 대기
    while ((DMA1_LISR & DMA_RX_TCIF) == 0UL);

    // RX DMA 종료 대기
    while (DMA1_S1CR & DMA_EN);

    // 완료 플래그 클리어
    DMA1_LIFCR = DMA_S1_CLEAR;
}

void dma_uart3_send(void)
{
    // TX Stream 비활성화
    DMA1_S3CR &= ~DMA_EN;
    while (DMA1_S3CR & DMA_EN);

    // 이전 완료 플래그 제거
    DMA1_LIFCR = DMA_S3_CLEAR;

    // 전송 개수 = 1바이트
    DMA1_S3NDTR = 1;

    // TX DMA 시작
    DMA1_S3CR |= DMA_EN;

    // DMA가 USART3_DR에 데이터를 옮길 때까지 대기
    while ((DMA1_LISR & DMA_TX_TCIF) == 0UL);

    // TX DMA 종료 대기
    while (DMA1_S3CR & DMA_EN);

    // 완료 플래그 클리어
    DMA1_LIFCR = DMA_S3_CLEAR;

    // UART 핀으로 실제 송신 완료될 때까지 대기
    while ((USART3_SR & (1UL << 6)) == 0UL);
}

int main(void)
{
    uart3_init();
    dma1_init();

    while (1)
    {
        // 1. USART3 -> DMA -> RAM
        dma_uart3_receive();

        // 2. RAM -> DMA -> USART3
        dma_uart3_send();
    }
}
