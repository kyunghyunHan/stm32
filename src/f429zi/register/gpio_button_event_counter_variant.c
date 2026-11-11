/*
 * NUCLEO-F429ZI / single-file register-level GPIO example
 * No HAL, main.h, key.h, led.h, or stm32f429xx.h required.
 *
 * Original three-file behavior preserved:
 *  1) Onboard B1 (PC13, active HIGH) -> debounce/event queue ->
 *     LD1 PB0, LD2 PB7, LD3 PB14 display count 000...111.
 *  2) External button D2 (PF15, external 10k pull-up, active LOW) ->
 *     external LED D7 (PF13, push-pull, active HIGH):
 *     released => LED ON, pressed => LED OFF.
 *
 * External wiring as described in original main.c:
 *   3.3V --- 10k ---+--- D2/PF15
 *                   |
 *                 button
 *                   |
 *                  GND
 *   D7/PF13 --- 330 ohm --- LED anode (+) / LED cathode (-) --- GND
 *
 * Assumes SYSCLK/HCLK = 16 MHz HSI (default after reset); SysTick polls
 * at 1ms. If clock is configured differently, adjust SYST_RVR value.
 * This file replaces application main.c, not the startup/linker files.
 */
#include <stdint.h>
#include <stdbool.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))
#define RCC_AHB1ENR REG32(0x40023830UL)

#define GPIOB_BASE 0x40020400UL
#define GPIOC_BASE 0x40020800UL
#define GPIOF_BASE 0x40021400UL
#define GPIO_MODER(base)  REG32((base) + 0x00UL)
#define GPIO_OTYPER(base) REG32((base) + 0x04UL)
#define GPIO_OSPEEDR(base) REG32((base) + 0x08UL)
#define GPIO_PUPDR(base)  REG32((base) + 0x0CUL)
#define GPIO_IDR(base)    REG32((base) + 0x10UL)
#define GPIO_BSRR(base)   REG32((base) + 0x18UL)

#define SYST_CSR REG32(0xE000E010UL)
#define SYST_RVR REG32(0xE000E014UL)
#define SYST_CVR REG32(0xE000E018UL)

#define KEY_DEBOUNCE_MS 20U
#define KEY_LONGPRESS_MS 1000U
#define KEY_EVT_QUEUE_SIZE 8U

typedef enum {
    KEY_EVT_NONE = 0,
    KEY_EVT_PRESSED,
    KEY_EVT_RELEASED,
    KEY_EVT_LONG_PRESS
} KeyEvent_t;

static uint32_t tick_ms;

/* 16MHz system clock / 16000 = 1kHz; processor clock, no interrupt */
static void Tick_Init(void)
{
    SYST_CSR = 0U;
    SYST_RVR = 16000U - 1U;
    SYST_CVR = 0U;
    SYST_CSR = (1U << 2) | 1U;
}

static void Tick_Wait1ms(void)
{
    while ((SYST_CSR & (1U << 16)) == 0U) { }
    ++tick_ms;
}

/* ------------------------- led.c merged ------------------------- */
static void LED_Init(void)
{
    RCC_AHB1ENR |= (1U << 1); /* GPIOB */
    (void)RCC_AHB1ENR;
    GPIO_BSRR(GPIOB_BASE) = (1U << 16) | (1U << 23) | (1U << 30);
    GPIO_OTYPER(GPIOB_BASE) &= ~((1U << 0) | (1U << 7) | (1U << 14));
    GPIO_OSPEEDR(GPIOB_BASE) &= ~((3U << 0) | (3U << 14) | (3U << 28));
    GPIO_PUPDR(GPIOB_BASE) &= ~((3U << 0) | (3U << 14) | (3U << 28));
    GPIO_MODER(GPIOB_BASE) =
        (GPIO_MODER(GPIOB_BASE) & ~((3U << 0) | (3U << 14) | (3U << 28))) |
        ((1U << 0) | (1U << 14) | (1U << 28));
}

static void LED_On(uint32_t pin)
{
    GPIO_BSRR(GPIOB_BASE) = 1U << pin;
}

static void LED_Off(uint32_t pin)
{
    GPIO_BSRR(GPIOB_BASE) = 1U << (pin + 16U);
}

static void LED_ShowCount(uint32_t count)
{
    if (count & 1U) LED_On(0U); else LED_Off(0U);
    if (count & 2U) LED_On(7U); else LED_Off(7U);
    if (count & 4U) LED_On(14U); else LED_Off(14U);
}

/* ------------------------- key.c merged ------------------------- */
static uint32_t raw_prev;
static bool stable_pressed;
static uint32_t changed_at;
static uint32_t pressed_at;
static bool long_sent;
static KeyEvent_t evt_queue[KEY_EVT_QUEUE_SIZE];
static uint8_t evt_head;
static uint8_t evt_tail;

static void Key_PushEvent(KeyEvent_t evt)
{
    uint8_t next = (uint8_t)((evt_head + 1U) & (KEY_EVT_QUEUE_SIZE - 1U));
    if (next == evt_tail) return;
    evt_queue[evt_head] = evt;
    evt_head = next;
}

static void Key_Init(void)
{
    RCC_AHB1ENR |= (1U << 2); /* GPIOC */
    (void)RCC_AHB1ENR;
    GPIO_MODER(GPIOC_BASE) &= ~(3U << 26); /* PC13 input */
    GPIO_PUPDR(GPIOC_BASE) &= ~(3U << 26); /* external board pull-down */

    raw_prev = (GPIO_IDR(GPIOC_BASE) >> 13) & 1U;
    stable_pressed = (raw_prev != 0U); /* B1: press = HIGH */
    changed_at = tick_ms;
    pressed_at = tick_ms;
    long_sent = stable_pressed;
    evt_head = evt_tail = 0U;
}

static void Key_Scan(void)
{
    uint32_t now = tick_ms;
    uint32_t raw = (GPIO_IDR(GPIOC_BASE) >> 13) & 1U;
    if (raw != raw_prev) {
        raw_prev = raw;
        changed_at = now;
    }

    bool raw_pressed = (raw != 0U);
    if ((now - changed_at) >= KEY_DEBOUNCE_MS &&
        raw_pressed != stable_pressed) {
        stable_pressed = raw_pressed;
        if (stable_pressed) {
            pressed_at = now;
            long_sent = false;
            Key_PushEvent(KEY_EVT_PRESSED);
        } else {
            Key_PushEvent(KEY_EVT_RELEASED);
        }
    }

    if (stable_pressed && !long_sent &&
        (now - pressed_at) >= KEY_LONGPRESS_MS) {
        long_sent = true;
        Key_PushEvent(KEY_EVT_LONG_PRESS);
    }
}

static KeyEvent_t Key_GetEvent(void)
{
    if (evt_tail == evt_head) return KEY_EVT_NONE;
    KeyEvent_t evt = evt_queue[evt_tail];
    evt_tail = (uint8_t)((evt_tail + 1U) & (KEY_EVT_QUEUE_SIZE - 1U));
    return evt;
}

/* ---------------------- main.c external GPIO -------------------- */
static void EXT_GPIO_Init(void)
{
    RCC_AHB1ENR |= (1U << 5); /* GPIOF */
    (void)RCC_AHB1ENR;

    /* PF13 (D7) push-pull LED output, initially LOW/off */
    GPIO_BSRR(GPIOF_BASE) = (1U << (13U + 16U));
    GPIO_OTYPER(GPIOF_BASE) &= ~(1U << 13);
    GPIO_PUPDR(GPIOF_BASE) &= ~(3U << 26);
    GPIO_MODER(GPIOF_BASE) =
        (GPIO_MODER(GPIOF_BASE) & ~(3U << 26)) | (1U << 26);

    /* PF15 (D2) external button input, external 10k pull-up */
    GPIO_MODER(GPIOF_BASE) &= ~(3U << 30);
    GPIO_PUPDR(GPIOF_BASE) &= ~(3U << 30); /* no internal pull */
}

int main(void)
{
    uint32_t count = 0U;
    Tick_Init();
    LED_Init();
    Key_Init();
    EXT_GPIO_Init();
    LED_ShowCount(count); /* 000 at power-on */

    while (1) {
        Tick_Wait1ms();
        Key_Scan();
        KeyEvent_t event;
        while ((event = Key_GetEvent()) != KEY_EVT_NONE) {
            if (event == KEY_EVT_PRESSED) {
                count = (count + 1U) & 7U;
                LED_ShowCount(count);
            }
            /* KEY_EVT_RELEASED, KEY_EVT_LONG_PRESS are still generated */
        }

        /* Original external LED logic: released (PF15=1) => LED ON,
           pressed (PF15=0) => LED OFF. */
        if ((GPIO_IDR(GPIOF_BASE) & (1U << 15)) != 0U) {
            GPIO_BSRR(GPIOF_BASE) = 1U << 13;
        } else {
            GPIO_BSRR(GPIOF_BASE) = 1U << (13U + 16U);
        }
    }
}
