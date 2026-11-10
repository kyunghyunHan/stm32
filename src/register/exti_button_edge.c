
#include <stdint.h>

/*
 * STM32F429ZI - EXTI Rising / Falling Edge
 *
 * B1  : PC13, Pressed = HIGH
 * LD2 : PB7,  HIGH = LED ON
 *
 * Rising Edge  : LED ON
 * Falling Edge : LED OFF
 *
 * HAL 사용하지 않음
 */

// 1. Base Address

#define RCC_BASE      0x40023800UL
#define GPIOB_BASE    0x40020400UL
#define GPIOC_BASE    0x40020800UL
#define SYSCFG_BASE   0x40013800UL
#define EXTI_BASE     0x40013C00UL

// 2. RCC

#define RCC_AHB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB2ENR   (*(volatile uint32_t *)(RCC_BASE + 0x44UL))

// 3. GPIOB (LED)

#define GPIOB_MODER   (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_OTYPER  (*(volatile uint32_t *)(GPIOB_BASE + 0x04UL))
#define GPIOB_BSRR    (*(volatile uint32_t *)(GPIOB_BASE + 0x18UL))

// 4. GPIOC (Button)

#define GPIOC_MODER   (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))
#define GPIOC_PUPDR   (*(volatile uint32_t *)(GPIOC_BASE + 0x0CUL))
#define GPIOC_IDR     (*(volatile uint32_t *)(GPIOC_BASE + 0x10UL))

// 5. SYSCFG: 어떤 GPIO 핀을 EXTI에 연결할지 선택

#define SYSCFG_EXTICR4 (*(volatile uint32_t *)(SYSCFG_BASE + 0x14UL))

// 6. EXTI:GPIO 신호의 상승·하강 에지를 감지하고 인터럽트 요청 생성

#define EXTI_IMR      (*(volatile uint32_t *)(EXTI_BASE + 0x00UL))
#define EXTI_RTSR     (*(volatile uint32_t *)(EXTI_BASE + 0x08UL))
#define EXTI_FTSR     (*(volatile uint32_t *)(EXTI_BASE + 0x0CUL))
#define EXTI_PR       (*(volatile uint32_t *)(EXTI_BASE + 0x14UL))

// 7. NVIC:발생한 인터럽트의 우선순위를 관리하고 CPU가 처리하도록 제어

#define NVIC_ISER1    (*(volatile uint32_t *)0xE000E104UL)
#define NVIC_ICPR1    (*(volatile uint32_t *)0xE000E284UL)

#define BUTTON_PIN    13U
#define LED_PIN       7U

// 8. GPIO 초기화

void GPIO_Init(void)
{
    // GPIOB, GPIOC Clock Enable
    RCC_AHB1ENR |= (1U << 1) | (1U << 2);

    // PB7 Output Mode = 01
    GPIOB_MODER &= ~(3U << (LED_PIN * 2U));
    GPIOB_MODER |=  (1U << (LED_PIN * 2U));

    // PB7 Push-Pull
    GPIOB_OTYPER &= ~(1U << LED_PIN);

    // LED 초기 상태 OFF
    GPIOB_BSRR = (1U << (LED_PIN + 16U));

    // PC13 Input Mode = 00
    GPIOC_MODER &= ~(3U << (BUTTON_PIN * 2U));

    // PC13 No Pull
    // B1 보드에 외부 Pull-down 존재
    GPIOC_PUPDR &= ~(3U << (BUTTON_PIN * 2U));
}

// 9. EXTI 초기화

void EXTI13_Init(void)
{
    // SYSCFG Clock Enable (APB2 Bit 14)
    RCC_APB2ENR |= (1U << 14);

    // EXTI13에 GPIOC 연결
    // EXTICR4 [7:4] = 0010 (Port C)
    SYSCFG_EXTICR4 &= ~(0xFU << 4);
    SYSCFG_EXTICR4 |=  (0x2U << 4);

    // Rising Edge (LOW -> HIGH)
    //상승·하강 에지 감지 활성화
    EXTI_RTSR |= (1U << BUTTON_PIN);

    // Falling Edge (HIGH -> LOW)
    EXTI_FTSR |= (1U << BUTTON_PIN);

    // EXTI13 Pending Clear (Write 1 to Clear)
    //EXTI에 남은 인터럽트 발생 표시 제거
    EXTI_PR = (1U << BUTTON_PIN);

    // EXTI13 Interrupt Unmask
    //EXTI 인터럽트 요청 허용
    EXTI_IMR |= (1U << BUTTON_PIN);

    // EXTI15_10 IRQn = 40
    // NVIC ISER1 Bit 8
    //NVIC에 남은 인터럽트 대기 상태 제거
    NVIC_ICPR1 = (1U << 8);
    //CPU가 해당 IRQ를 처리하도록 활성화
    NVIC_ISER1 = (1U << 8);
}

// 10. 인터럽트 핸들러
/*
인터럽트	핸들러 이름
EXTI0	EXTI0_IRQHandler
EXTI1	EXTI1_IRQHandler
EXTI5~9	EXTI9_5_IRQHandler
EXTI10~15	EXTI15_10_IRQHandler
USART2	USART2_IRQHandler
USART3	USART3_IRQHandler
TIM2	TIM2_IRQHandler
SysTick	SysTick_Handler
DMA1 Stream 0	DMA1_Stream0_IRQHandler
*/
//인터럽트 발생 시 CPU가 실행하는 함수
void EXTI15_10_IRQHandler(void)
{
    // EXTI13 Pending 확인
    if (EXTI_PR & (1U << BUTTON_PIN))
    {
        // Pending 플래그 해제
        EXTI_PR = (1U << BUTTON_PIN);

        // PC13 현재 입력 확인
        if (GPIOC_IDR & (1U << BUTTON_PIN))
        {
            // 버튼 눌림: HIGH
            GPIOB_BSRR = (1U << LED_PIN);
        }
        else
        {
            // 버튼 해제: LOW
            GPIOB_BSRR = (1U << (LED_PIN + 16U));
        }
    }
}
/*
① B1 버튼 누름
       ↓
② PC13 상승 에지 발생 (0 → 1)
       ↓
③ EXTI_PR의 Bit 13 = 1 (Pending 발생)
       ↓
④ NVIC를 통해 CPU에 인터럽트 요청 전달
       ↓
⑤ CPU가 EXTI15_10_IRQHandler() 진입
       ↓
⑥ if (EXTI_PR & (1U << 13))
   → EXTI13 Pending이 1인지 확인
       ↓
⑦ EXTI_PR = (1U << 13)
   → Pending을 0으로 클리어
       ↓
⑧ GPIOC_IDR 읽기
   → 버튼이 현재 HIGH인지 LOW인지 확인
       ↓
⑨ GPIOB_BSRR 제어
   → LED ON 또는 OFF
       ↓
⑩ ISR 종료 → 이전 코드로 복귀
*/
// 11. Main

int main(void)
{
    GPIO_Init();
    EXTI13_Init();

    // 시작할 때 실제 버튼 상태 반영
    if (GPIOC_IDR & (1U << BUTTON_PIN))
    {
        GPIOB_BSRR = (1U << LED_PIN);
    }

    while (1)
    {
        // 인터럽트 발생까지 CPU Sleep
        __asm volatile ("wfi");
    }
}
