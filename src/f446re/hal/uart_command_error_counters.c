
#include "stm32f4xx_hal.h"
#include <stdint.h>

/*
 * STM32 NUCLEO-F446RE
 *
 * USART2 : PA2 TX / PA3 RX (AF7)
 * LED1   : PB0 (External)
 * LED2   : PA5 (Onboard LD2)
 * LED3   : PB1 (External)
 * KEY1   : PC13 (Onboard B1)
 *
 * HSI = 16MHz
 * APB1 = 16MHz
 * UART = 9600, 8N1
 *
 * KEY1:
 * Pressed  = HIGH
 * Released = LOW
 */

#define LED1_PIN GPIO_PIN_0
#define LED2_PIN GPIO_PIN_5
#define LED3_PIN GPIO_PIN_1
#define KEY1_PIN GPIO_PIN_13

#define DEBOUNCE_MS 20U
#define BUFFER_SIZE 32U

UART_HandleTypeDef huart2;

/* LED states */
static uint8_t led_state[3] = {0, 0, 0};

/* UART error counters */
static uint32_t ore_count = 0;
static uint32_t fe_count = 0;
static uint32_t ne_count = 0;
static uint32_t pe_count = 0;
static uint32_t buffer_error_count = 0;

/* Button debounce */
static uint8_t key_stable = 0;
static uint8_t key_candidate = 0;
static uint32_t key_count = 0;

/* Function declarations */
void SystemClock_Config(void);
void gpio_init(void);
void uart2_init(void);
void key_init(void);
void key_scan_1ms(void);
void led_control(uint8_t led, uint8_t on);
void all_led_control(uint8_t on);
void print_status(void);
void print_errors(void);
void process_command(const char *cmd);
void Error_Handler(void);

/* ========================================
 * 1. System Clock
 * ======================================== */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
        Error_Handler();

    clk.ClockType = RCC_CLOCKTYPE_SYSCLK |
                    RCC_CLOCKTYPE_HCLK |
                    RCC_CLOCKTYPE_PCLK1 |
                    RCC_CLOCKTYPE_PCLK2;

    clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_0)
        != HAL_OK)
        Error_Handler();
}

/* ========================================
 * 2. GPIO Initialization
 * ======================================== */

void gpio_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* LED initial OFF */
    HAL_GPIO_WritePin(GPIOA, LED2_PIN, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(
        GPIOB,
        LED1_PIN | LED3_PIN,
        GPIO_PIN_RESET
    );

    /* PA5 : LED2 */
    gpio.Pin = LED2_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &gpio);

    /* PB0, PB1 : LED1, LED3 */
    gpio.Pin = LED1_PIN | LED3_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOB, &gpio);

    /* PC13 : KEY1 */
    gpio.Pin = KEY1_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(GPIOC, &gpio);
}

/* ========================================
 * 3. USART2 Initialization
 * ======================================== */

void uart2_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    /* PA2 TX, PA3 RX */
    gpio.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF7_USART2;

    HAL_GPIO_Init(GPIOA, &gpio);

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 9600;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK)
        Error_Handler();
}

/* ========================================
 * 4. UART Send
 * ======================================== */

void uart2_send_char(char c)
{
    uint8_t data = (uint8_t)c;

    HAL_UART_Transmit(
        &huart2,
        &data,
        1,
        HAL_MAX_DELAY
    );
}

void uart2_send_string(const char *s)
{
    while (*s)
    {
        uart2_send_char(*s++);
    }
}

void uart2_send_number(uint32_t n)
{
    char buf[11];
    uint32_t i = 0;

    if (n == 0U)
    {
        uart2_send_char('0');
        return;
    }

    while (n)
    {
        buf[i++] = (char)('0' + (n % 10U));
        n /= 10U;
    }

    while (i)
    {
        uart2_send_char(buf[--i]);
    }
}

/* ========================================
 * 5. UART Receive (Non-blocking Polling)
 *
 * Return:
 *  1 = valid byte
 *  0 = no byte
 * -1 = UART error
 * ======================================== */

int uart2_receive_checked(char *out)
{
    uint32_t sr = huart2.Instance->SR;

    uint32_t errors = sr &
        (UART_FLAG_ORE |
         UART_FLAG_FE  |
         UART_FLAG_NE  |
         UART_FLAG_PE);

    if (errors)
    {
        if (errors & UART_FLAG_ORE) ore_count++;
        if (errors & UART_FLAG_FE)  fe_count++;
        if (errors & UART_FLAG_NE)  ne_count++;
        if (errors & UART_FLAG_PE)  pe_count++;

        /* SR -> DR read clears these errors */
        __HAL_UART_CLEAR_PEFLAG(&huart2);

        return -1;
    }

    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE)
        == RESET)
    {
        return 0;
    }

    uint8_t data = 0;

    if (HAL_UART_Receive(
            &huart2,
            &data,
            1,
            0
        ) == HAL_OK)
    {
        *out = (char)data;
        return 1;
    }

    return 0;
}

/* ========================================
 * 6. LED Control
 * ======================================== */

void led_control(uint8_t led, uint8_t on)
{
    GPIO_PinState state =
        on ? GPIO_PIN_SET : GPIO_PIN_RESET;

    if (led == 1)
    {
        HAL_GPIO_WritePin(GPIOB, LED1_PIN, state);
        led_state[0] = on ? 1U : 0U;
    }
    else if (led == 2)
    {
        HAL_GPIO_WritePin(GPIOA, LED2_PIN, state);
        led_state[1] = on ? 1U : 0U;
    }
    else if (led == 3)
    {
        HAL_GPIO_WritePin(GPIOB, LED3_PIN, state);
        led_state[2] = on ? 1U : 0U;
    }
}

void all_led_control(uint8_t on)
{
    led_control(1, on);
    led_control(2, on);
    led_control(3, on);
}

/* ========================================
 * 7. Button
 * ======================================== */

uint8_t key_read(void)
{
    return HAL_GPIO_ReadPin(GPIOC, KEY1_PIN)
        == GPIO_PIN_SET ? 1U : 0U;
}

void key_init(void)
{
    key_stable = key_read();
    key_candidate = key_stable;
    key_count = 0;
}

void key_scan_1ms(void)
{
    uint8_t current = key_read();

    if (current != key_candidate)
    {
        key_candidate = current;
        key_count = 0;
        return;
    }

    if (current == key_stable)
    {
        key_count = 0;
        return;
    }

    if (key_count < DEBOUNCE_MS)
        key_count++;

    if (key_count >= DEBOUNCE_MS)
    {
        key_stable = current;
        key_count = 0;

        if (key_stable)
        {
            uart2_send_string(
                "\r\nKEY1 = 1 (PRESSED)\r\n> "
            );
        }
        else
        {
            uart2_send_string(
                "\r\nKEY1 = 0 (RELEASED)\r\n> "
            );
        }
    }
}

/* ========================================
 * 8. String Compare
 * ======================================== */

int str_equal(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (*a != *b)
            return 0;

        a++;
        b++;
    }

    return (*a == '\0' && *b == '\0');
}

/* ========================================
 * 9. Print Status
 * ======================================== */

void print_status(void)
{
    uart2_send_string("\r\nDEVICE STATUS\r\n");

    uart2_send_string("LED1: ");
    uart2_send_string(
        led_state[0] ? "ON\r\n" : "OFF\r\n"
    );

    uart2_send_string("LED2: ");
    uart2_send_string(
        led_state[1] ? "ON\r\n" : "OFF\r\n"
    );

    uart2_send_string("LED3: ");
    uart2_send_string(
        led_state[2] ? "ON\r\n" : "OFF\r\n"
    );

    uart2_send_string("KEY1: ");
    uart2_send_string(
        key_stable ? "PRESSED (1)\r\n" :
                     "RELEASED (0)\r\n"
    );
}

void print_errors(void)
{
    uart2_send_string("\r\nUART ERRORS\r\n");

    uart2_send_string("ORE: ");
    uart2_send_number(ore_count);
    uart2_send_string("\r\n");

    uart2_send_string("FE : ");
    uart2_send_number(fe_count);
    uart2_send_string("\r\n");

    uart2_send_string("NE : ");
    uart2_send_number(ne_count);
    uart2_send_string("\r\n");

    uart2_send_string("PE : ");
    uart2_send_number(pe_count);
    uart2_send_string("\r\n");

    uart2_send_string("BUFFER: ");
    uart2_send_number(buffer_error_count);
    uart2_send_string("\r\n");
}

/* ========================================
 * 10. Command Processing
 * ======================================== */

void process_command(const char *cmd)
{
    if (str_equal(cmd, "led1 on"))
        led_control(1, 1);

    else if (str_equal(cmd, "led2 on"))
        led_control(2, 1);

    else if (str_equal(cmd, "led3 on"))
        led_control(3, 1);

    else if (str_equal(cmd, "led1 off"))
        led_control(1, 0);

    else if (str_equal(cmd, "led2 off"))
        led_control(2, 0);

    else if (str_equal(cmd, "led3 off"))
        led_control(3, 0);

    else if (str_equal(cmd, "123 on"))
        all_led_control(1);

    else if (str_equal(cmd, "123 off") ||
             str_equal(cmd, "off"))
        all_led_control(0);

    else if (str_equal(cmd, "key1"))
    {
        uart2_send_string("KEY1 = ");
        uart2_send_string(
            key_stable ? "1 (PRESSED)\r\n" :
                         "0 (RELEASED)\r\n"
        );
        return;
    }
    else if (str_equal(cmd, "errors"))
    {
        print_errors();
        return;
    }
    else if (str_equal(cmd, "clear"))
    {
        ore_count = 0;
        fe_count = 0;
        ne_count = 0;
        pe_count = 0;
        buffer_error_count = 0;

        uart2_send_string(
            "ERROR COUNTERS CLEARED\r\n"
        );
        return;
    }
    else if (str_equal(cmd, "status"))
    {
        print_status();
        return;
    }
    else
    {
        uart2_send_string("UNKNOWN COMMAND: [");
        uart2_send_string(cmd);
        uart2_send_string("]\r\n");
        return;
    }

    uart2_send_string("COMMAND OK\r\n");
    print_status();
}

/* ========================================
 * 11. Main
 * ======================================== */

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    gpio_init();
    uart2_init();
    key_init();

    char buffer[BUFFER_SIZE];
    uint32_t index = 0;
    uint8_t line_error = 0;

    uint32_t last_tick = HAL_GetTick();

    uart2_send_string(
        "\r\nSTM32F446RE HAL READY\r\n"
    );

    print_status();
    uart2_send_string("> ");

    while (1)
    {
        /* 1. Button scan every 1ms */
        uint32_t now = HAL_GetTick();

        if ((uint32_t)(now - last_tick) >= 1U)
        {
            last_tick = now;
            key_scan_1ms();
        }

        /* 2. UART receive */
        char c = 0;

        int result = uart2_receive_checked(&c);

        if (result == 0)
            continue;

        if (result == -1)
        {
            line_error = 1;
            continue;
        }

        /* 3. Enter */
        if (c == '\r' || c == '\n')
        {
            if (line_error)
            {
                uart2_send_string(
                    "\r\nRX ERROR: COMMAND DISCARDED\r\n> "
                );

                index = 0;
                line_error = 0;
                continue;
            }

            while (index > 0 &&
                   buffer[index - 1] == ' ')
            {
                index--;
            }

            if (index > 0)
            {
                buffer[index] = '\0';

                uint32_t start = 0;

                while (buffer[start] == ' ')
                    start++;

                uart2_send_string("\r\n");

                if (buffer[start] != '\0')
                    process_command(&buffer[start]);

                index = 0;
                uart2_send_string("> ");
            }
        }

        /* 4. Backspace */
        else if (c == '\b' || (uint8_t)c == 127U)
        {
            if (index > 0 && !line_error)
            {
                index--;
                uart2_send_string("\b \b");
            }
        }

        /* 5. ASCII input */
        else if (c >= 32 && c <= 126)
        {
            if (line_error)
                continue;

            if (index < sizeof(buffer) - 1U)
            {
                buffer[index++] = c;
                uart2_send_char(c);
            }
            else
            {
                buffer_error_count++;
                line_error = 1;
            }
        }
    }
}

/* ========================================
 * 12. Error Handler
 * ======================================== */

void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}
