
#include <stdint.h>

/*
 * STM32F429ZI - LED Morse Code SOS
 *
 * LED   : LD2 (PB7, Active-HIGH)
 * Clock : HSI 16MHz
 * Timer : SysTick 1ms
 * Mode  : Register / Bare-metal
 *
 * SOS = ... --- ...
 *
 * Dot  (.) : 200ms
 * Dash (-) : 600ms
 */

// ===============================
// 1. Register Address
// ===============================

#define RCC_BASE       0x40023800UL
#define GPIOB_BASE     0x40020400UL
#define SYSTICK_BASE   0xE000E010UL

// RCC
#define RCC_AHB1ENR    (*(volatile uint32_t *)(RCC_BASE + 0x30UL))

// GPIOB
#define GPIOB_MODER    (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_OTYPER   (*(volatile uint32_t *)(GPIOB_BASE + 0x04UL))
#define GPIOB_BSRR     (*(volatile uint32_t *)(GPIOB_BASE + 0x18UL))

// SysTick
#define SYST_CSR       (*(volatile uint32_t *)(SYSTICK_BASE + 0x00UL))
#define SYST_RVR       (*(volatile uint32_t *)(SYSTICK_BASE + 0x04UL))
#define SYST_CVR       (*(volatile uint32_t *)(SYSTICK_BASE + 0x08UL))

#define LED_PIN        7U
#define UNIT_MS        200U

// ===============================
// 2. GPIO Initialization
// ===============================

void GPIO_Init(void)
{
    // GPIOB Clock Enable
    RCC_AHB1ENR |= (1U << 1);

    // PB7 Output Mode (01)
    GPIOB_MODER &= ~(3U << 14);
    GPIOB_MODER |=  (1U << 14);

    // PB7 Push-Pull
    GPIOB_OTYPER &= ~(1U << 7);

    // Initial LED OFF
    GPIOB_BSRR = (1U << 23);
}

// ===============================
// 3. SysTick Initialization
// ===============================

void SysTick_Init(void)
{
    // HSI 16MHz / 1000 = 16000
    // 1ms Period
    SYST_CSR = 0U;
    SYST_RVR = 16000U - 1U;
    SYST_CVR = 0U;

    // ENABLE = 1, TICKINT = 0, CLKSOURCE = 1
    SYST_CSR = (1U << 2) | (1U << 0);
}

// ===============================
// 4. Delay (ms)
// ===============================

void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        // Wait until COUNTFLAG = 1
        while ((SYST_CSR & (1U << 16)) == 0U)
        {
        }
    }
}

// ===============================
// 5. LED Control
// ===============================

void LED_ON(void)
{
    GPIOB_BSRR = (1U << LED_PIN);
}

void LED_OFF(void)
{
    GPIOB_BSRR = (1U << (LED_PIN + 16U));
}

// ===============================
// 6. Morse Code
// ===============================

void dot(void)
{
    LED_ON();
    delay_ms(UNIT_MS);

    LED_OFF();
    delay_ms(UNIT_MS);
}

void dash(void)
{
    LED_ON();
    delay_ms(UNIT_MS * 3U);

    LED_OFF();
    delay_ms(UNIT_MS);
}

// ===============================
// 7. SOS Pattern
// ===============================

void SOS(void)
{
    // S = ...
    dot();
    dot();
    dot();

    // Letter Gap: total 3 units
    delay_ms(UNIT_MS * 2U);

    // O = ---
    dash();
    dash();
    dash();

    // Letter Gap: total 3 units
    delay_ms(UNIT_MS * 2U);

    // S = ...
    dot();
    dot();
    dot();

    // Repeat Gap: total 7 units
    delay_ms(UNIT_MS * 6U);
}

// ===============================
// 8. Main
// ===============================

int main(void)
{
    GPIO_Init();
    SysTick_Init();

    while (1)
    {
        SOS();
    }
}
