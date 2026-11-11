#include <stdint.h>
// 주소정의
// 시작주소
// GPIO D 레지스터들이 시작되는 주
#define GPIOD_BASE 0x40020c00UL

// RCC 레지스터 시작 주소
#define RCC_BASE 0x40023800UL

// GPIO 레지스터 offset

// MODE 레지스터 :핀을 입력/출력/특수기능/아날로그 중 뭘로 쓸지
/*
 * moder은 핀당 2bit
 * 01로 만들면 outpuut
 * 00은input
 */
#define GPIO_MODER_OFFSET 0x00UL // 2bit
// 출력 방식을 Push-Pull / Open-Drain 중 뭘로 할지
#define GPIO_OTYPER_OFFSET 0x04UL
// 출력 신호의 속도(slew rate) 설정
// 2bit
#define GPIO_OSPEEDR_OFFSET 0x08UL
// 내부 Pull-up / Pull-down 저항 설정
// 2bit
#define GPIO_PUPDR_OFFSET 0x0CUL
// 핀으로 들어오는 0/1을 읽음 1bit
#define GPIO_IDR_OFFSET 0x10UL
// 핀으로 내보낼 0/1을 설정 1bit
#define GPIO_ODR_OFFSET 0x14UL

// RCC 레지스터 오프셋
// 레지스터주소정의
#define RCC_AHB1ENR (*((volatile uint32_t *)(RCC_BASE + RCC_AHB1ENR_OFFSET)))
// GPIOD_MODER |= (1<<6);
#define GPIOD_MODER (*((volatile uint32_t *)(GPIOD_BASE + GPIO_MODER_OFFSET)))

#define GPIOD_OTYPER (*((volatile uint32_t *)(GPIOD_BASE + GPIO_OTYPER_OFFSET)))

#define GPIO_OSPEEDR (*((volatile uint32_t *)(GPIOD_BASE + GPIO_OSPEEDR_OFFSET)))

#define GPIO_PUPDR (*((volatile uint32_t *)(GPIOD_BASE + GPIO_PUPDR_OFFSET)))

#define GPIO_IDR (*((volatile uint32_t *)(GPIOD_BASE + GPIO_IDR_OFFSET)))

#define GPIO_ODR (*((volatile uint32_t *)(GPIOD_BASE + GPIO_ODR_OFFSET)))

typedef struct
{
    volatile uint32_t MODER;   // 0x00
    volatile uint32_t OTYPER;  // 0x04
    volatile uint32_t OSPEEDR; // 0x08
    volatile uint32_t PUPDR;   // 0x0C
    volatile uint32_t IDR;     // 0x10
    volatile uint32_t ODR;     // 0x14
    volatile uint32_t BSRR;    // 0x18
    volatile uint32_t LCKR;    // 0x1C
    // volatile uint32_t AFRL;     // 0x20
    // volatile uint32_t AFRH;     // 0x24
    volatile uint32_t AFR[2];
} GPIO_TypeDef;

#define GPIOD ((GPIO_TypeDef *)GPIOD_BASE)



// *(volatile uint32_t *)0x1000 = 1;

