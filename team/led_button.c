/* STM32F429ZITx, HSI 16 MHz (CMSIS SystemInit default).
 * PA10 -> resistor -> LED -> GND: HIGH turns the LED on.
 * PC13 -> button -> GND: internal pull-up, pressed = LOW.
 * Toggle the LED once per debounced press.
 */
#include "stm32f429xx.h"
#include <stdint.h>

#define LED_PIN 10U
#define BUTTON_PIN 13U
#define DEBOUNCE_MS 20U

static void delay_ms(uint32_t ms)
{
    while (ms-- > 0U)
    {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U)
        {
        }
    }
}

static uint32_t button_read(void)
{
    return (GPIOC->IDR >> BUTTON_PIN) & 1U;
}

int main(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;
    (void)RCC->AHB1ENR;

    // PA10: LED initially off, push-pull output without pull resistors.
    GPIOA->BSRR = 1U << (LED_PIN + 16U);
    GPIOA->OTYPER &= ~(1U << LED_PIN);
    GPIOA->OSPEEDR &= ~(3U << (LED_PIN * 2U));
    GPIOA->PUPDR &= ~(3U << (LED_PIN * 2U));
    GPIOA->MODER = (GPIOA->MODER & ~(3U << (LED_PIN * 2U)))
                   | (1U << (LED_PIN * 2U));

    // PC13: input with pull-up; the external switch connects it to GND.
    GPIOC->MODER &= ~(3U << (BUTTON_PIN * 2U));
    GPIOC->PUPDR = (GPIOC->PUPDR & ~(3U << (BUTTON_PIN * 2U)))
                   | (1U << (BUTTON_PIN * 2U));

    SystemCoreClockUpdate();
    SysTick->LOAD = (SystemCoreClock / 1000U) - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    delay_ms(DEBOUNCE_MS);
    uint32_t stable_button = button_read();
    uint32_t candidate = stable_button;
    uint32_t stable_ms = 0U;
    uint32_t led_on = 0U;

    while (1)
    {
        delay_ms(1U);
        uint32_t button = button_read();
        if (button != candidate)
        {
            candidate = button;
            stable_ms = 0U;
        }
        else if (stable_ms < DEBOUNCE_MS)
        {
            ++stable_ms;
        }

        if (stable_ms == DEBOUNCE_MS && stable_button != candidate)
        {
            stable_button = candidate;
            if (stable_button == 0U)
            {
                led_on ^= 1U;
                GPIOA->BSRR = 1U << (LED_PIN + (led_on ? 0U : 16U));
            }
        }
    }
}
