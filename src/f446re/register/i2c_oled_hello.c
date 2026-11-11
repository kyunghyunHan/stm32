
#include <stdint.h>

/* =========================================
   STM32F446RE OLED + UART
   HSI 16MHz, APB1 16MHz
   I2C1 PB8(SCL), PB9(SDA)
   USART2 PA2(TX), 9600bps
   SSD1306 128x64, address 0x3C
   ========================================= */

#define REG32(a) (*(volatile uint32_t *)(a))

#define RCC_BASE      0x40023800UL
#define GPIOA_BASE    0x40020000UL
#define GPIOB_BASE    0x40020400UL
#define USART2_BASE   0x40004400UL
#define I2C1_BASE     0x40005400UL
#define SYSTICK_BASE  0xE000E010UL

/* RCC */
#define RCC_AHB1ENR REG32(RCC_BASE + 0x30)
#define RCC_APB1ENR REG32(RCC_BASE + 0x40)

/* GPIO */
#define GPIOA_MODER   REG32(GPIOA_BASE + 0x00)
#define GPIOA_AFRL    REG32(GPIOA_BASE + 0x20)

#define GPIOB_MODER   REG32(GPIOB_BASE + 0x00)
#define GPIOB_OTYPER  REG32(GPIOB_BASE + 0x04)
#define GPIOB_OSPEEDR REG32(GPIOB_BASE + 0x08)
#define GPIOB_PUPDR   REG32(GPIOB_BASE + 0x0C)
#define GPIOB_AFRH    REG32(GPIOB_BASE + 0x24)

/* USART2 */
#define USART2_SR     REG32(USART2_BASE + 0x00)
#define USART2_DR     REG32(USART2_BASE + 0x04)
#define USART2_BRR    REG32(USART2_BASE + 0x08)
#define USART2_CR1    REG32(USART2_BASE + 0x0C)

/* I2C1 */
#define I2C1_CR1      REG32(I2C1_BASE + 0x00)
#define I2C1_CR2      REG32(I2C1_BASE + 0x04)
#define I2C1_DR       REG32(I2C1_BASE + 0x10)
#define I2C1_SR1      REG32(I2C1_BASE + 0x14)
#define I2C1_SR2      REG32(I2C1_BASE + 0x18)
#define I2C1_CCR      REG32(I2C1_BASE + 0x1C)
#define I2C1_TRISE    REG32(I2C1_BASE + 0x20)

/* SysTick */
#define STK_CTRL      REG32(SYSTICK_BASE + 0x00)
#define STK_LOAD      REG32(SYSTICK_BASE + 0x04)
#define STK_VAL       REG32(SYSTICK_BASE + 0x08)

#define OLED_ADDR     0x3CU
#define TIMEOUT       200000UL

/* =========================================
   1. Delay
   ========================================= */
static void delay_ms(uint32_t ms)
{
    STK_CTRL = 0;
    STK_LOAD = 16000U - 1U;
    STK_VAL = 0;
    STK_CTRL = (1U << 2) | (1U << 0);

    while (ms--)
    {
        while ((STK_CTRL & (1U << 16)) == 0U) {}
    }

    STK_CTRL = 0;
}

/* =========================================
   2. USART2
   ========================================= */
static void uart_init(void)
{
    RCC_AHB1ENR |= (1U << 0);
    RCC_APB1ENR |= (1U << 17);

    /* PA2 = AF7 */
    GPIOA_MODER &= ~(3U << 4);
    GPIOA_MODER |=  (2U << 4);

    GPIOA_AFRL &= ~(0xFU << 8);
    GPIOA_AFRL |=  (7U << 8);

    /* 16MHz / 9600, oversampling by 16 */
    USART2_BRR = 0x0683U;

    /* UE + TE */
    USART2_CR1 = (1U << 13) | (1U << 3);
}

static void uart_char(char c)
{
    while ((USART2_SR & (1U << 7)) == 0U) {}

    USART2_DR = (uint8_t)c;
}

static void uart_print(const char *str)
{
    while (*str)
    {
        uart_char(*str++);
    }
}

/* =========================================
   3. I2C1 GPIO + Peripheral
   ========================================= */
static void i2c_init(void)
{
    RCC_AHB1ENR |= (1U << 1);
    RCC_APB1ENR |= (1U << 21);

    /* PB8, PB9 = Alternate Function */
    GPIOB_MODER &= ~((3U << 16) | (3U << 18));
    GPIOB_MODER |=  ((2U << 16) | (2U << 18));

    /* Open-Drain */
    GPIOB_OTYPER |= (1U << 8) | (1U << 9);

    /* High GPIO output speed */
    GPIOB_OSPEEDR |= (3U << 16) | (3U << 18);

    /* Internal pull-up (external pull-ups recommended) */
    GPIOB_PUPDR &= ~((3U << 16) | (3U << 18));
    GPIOB_PUPDR |=  ((1U << 16) | (1U << 18));

    /* AF4 = I2C1 */
    GPIOB_AFRH &= ~((0xFU << 0) | (0xFU << 4));
    GPIOB_AFRH |=  ((4U << 0) | (4U << 4));

    /* Configure I2C while disabled */
    I2C1_CR1 = 0;

    /* APB1 = 16MHz */
    I2C1_CR2 = 16U;

    /* Standard mode, 100kHz: 16MHz / (2 * 100kHz) */
    I2C1_CCR = 80U;

    /* 1000ns rise time */
    I2C1_TRISE = 17U;

    I2C1_CR1 = (1U << 0);  // PE
}

/* =========================================
   4. I2C status and timeout
   ========================================= */
static int i2c_wait(uint32_t flag)
{
    uint32_t timeout = TIMEOUT;

    while ((I2C1_SR1 & flag) == 0U)
    {
        if (I2C1_SR1 & ((1U << 8) | (1U << 9) |
                        (1U << 10) | (1U << 14)))
        {
            return -2;  // Bus / Arbitration / NACK / Timeout
        }

        if (--timeout == 0U)
        {
            return -1;
        }
    }

    return 0;
}

static int i2c_write(uint8_t control,
                     const uint8_t *data,
                     uint32_t len)
{
    uint32_t timeout = TIMEOUT;

    /* Wait while bus is busy */
    while (I2C1_SR2 & (1U << 1))
    {
        if (--timeout == 0U)
            return -1;
    }

    /* START */
    I2C1_CR1 |= (1U << 8);

    if (i2c_wait(1U << 0) != 0)
        goto error;

    /* 7-bit address + write bit */
    I2C1_DR = OLED_ADDR << 1;

    if (i2c_wait(1U << 1) != 0)
        goto error;

    /* Clear ADDR by reading SR1 and SR2 */
    (void)I2C1_SR1;
    (void)I2C1_SR2;

    /* Control byte: 0x00 = command, 0x40 = data */
    if (i2c_wait(1U << 7) != 0)
        goto error;

    I2C1_DR = control;

    for (uint32_t i = 0; i < len; i++)
    {
        if (i2c_wait(1U << 7) != 0)
            goto error;

        I2C1_DR = data[i];
    }

    /* Wait until final byte is transmitted */
    if (i2c_wait(1U << 2) != 0)
        goto error;

    I2C1_CR1 |= (1U << 9); // STOP
    return 0;

error:
    I2C1_CR1 |= (1U << 9);
    I2C1_SR1 &= ~((1U << 8) | (1U << 9) |
                  (1U << 10) | (1U << 14));
    return -1;
}

/* =========================================
   5. SSD1306 command
   ========================================= */
static int oled_cmd(uint8_t cmd)
{
    return i2c_write(0x00U, &cmd, 1U);
}

static int oled_init(void)
{
    /* 128x64 OLED initialization sequence */
    static const uint8_t commands[] =
    {
        0xAE,       // Display OFF
        0xD5, 0x80, // Clock
        0xA8, 0x3F, // Multiplex 64
        0xD3, 0x00, // Display offset
        0x40,       // Start line
        0x8D, 0x14, // Charge pump ON
        0x20, 0x02, // Page addressing
        0xA1,       // Segment remap
        0xC8,       // COM scan direction
        0xDA, 0x12, // COM pins
        0x81, 0x7F, // Contrast
        0xD9, 0xF1, // Precharge
        0xDB, 0x40, // VCOMH
        0xA4,       // Display RAM
        0xA6,       // Normal display
        0xAF        // Display ON
    };

    delay_ms(100);

    for (uint32_t i = 0; i < sizeof(commands); i++)
    {
        if (oled_cmd(commands[i]) != 0)
            return -1;
    }

    return 0;
}

/* =========================================
   6. OLED page drawing
   ========================================= */
static int oled_page(uint8_t page, const uint8_t *data)
{
    if (oled_cmd(0xB0U | page) != 0) return -1;
    if (oled_cmd(0x00U) != 0) return -1;
    if (oled_cmd(0x10U) != 0) return -1;

    /* Send 128 display columns */
    return i2c_write(0x40U, data, 128U);
}

static int oled_clear(void)
{
    uint8_t blank[128] = {0};

    for (uint8_t page = 0; page < 8; page++)
    {
        if (oled_page(page, blank) != 0)
            return -1;
    }

    return 0;
}

/* =========================================
   7. Minimal 5x7 font
   Only characters for "HELLO STM32"
   ========================================= */
static const uint8_t *glyph(char c)
{
    static const uint8_t H[5] = {0x7F,0x08,0x08,0x08,0x7F};
    static const uint8_t E[5] = {0x7F,0x49,0x49,0x49,0x41};
    static const uint8_t L[5] = {0x7F,0x40,0x40,0x40,0x40};
    static const uint8_t O[5] = {0x3E,0x41,0x41,0x41,0x3E};
    static const uint8_t S[5] = {0x46,0x49,0x49,0x49,0x31};
    static const uint8_t T[5] = {0x01,0x01,0x7F,0x01,0x01};
    static const uint8_t M[5] = {0x7F,0x02,0x0C,0x02,0x7F};
    static const uint8_t N3[5] = {0x22,0x41,0x49,0x49,0x36};
    static const uint8_t N2[5] = {0x62,0x51,0x49,0x49,0x46};
    static const uint8_t SPACE[5] = {0,0,0,0,0};

    switch (c)
    {
        case 'H': return H;
        case 'E': return E;
        case 'L': return L;
        case 'O': return O;
        case 'S': return S;
        case 'T': return T;
        case 'M': return M;
        case '3': return N3;
        case '2': return N2;
        default:  return SPACE;
    }
}

static int oled_hello(void)
{
    uint8_t line[128] = {0};
    const char *str = "HELLO STM32";

    /* Start near horizontal center */
    uint32_t x = 31U;

    while (*str && (x + 6U <= 128U))
    {
        const uint8_t *font = glyph(*str++);

        for (uint32_t i = 0; i < 5U; i++)
            line[x++] = font[i];

        line[x++] = 0x00U;
    }

    /* Display text on page 3 */
    return oled_page(3U, line);
}

/* =========================================
   8. MAIN
   ========================================= */
int main(void)
{
    uart_init();

    uart_print("\r\n[INFO] SYSTEM START\r\n");

    i2c_init();
    uart_print("[INFO] I2C1 INIT OK\r\n");

    if (oled_init() != 0)
    {
        uart_print("[ERROR] OLED INIT FAILED\r\n");
        while (1) {}
    }

    uart_print("[INFO] OLED INIT OK\r\n");

    if (oled_clear() != 0)
    {
        uart_print("[ERROR] OLED CLEAR FAILED\r\n");
        while (1) {}
    }

    if (oled_hello() != 0)
    {
        uart_print("[ERROR] OLED WRITE FAILED\r\n");
        while (1) {}
    }

    uart_print("[INFO] HELLO STM32 DISPLAYED\r\n");

    while (1)
    {
        delay_ms(1000);
    }
}
