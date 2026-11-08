
#include "lcd.h"

/*
 * STM32F439ZI + I2C LCD 1602
 *
 * I2C1
 * PB8 : SCL (AF4)
 * PB9 : SDA (AF4)
 *
 * LCD 주소: 0x27
 *
 * PCF8574 일반적인 핀 배치:
 * P0 = RS
 * P1 = RW
 * P2 = EN
 * P3 = Backlight
 * P4~P7 = LCD D4~D7
 */

/* LCD 제어 비트 */
#define LCD_RS         0x01U
#define LCD_EN         0x04U
#define LCD_BACKLIGHT  0x08U

/* LCD 전용 I2C 핸들 */
static I2C_HandleTypeDef hlcd_i2c;

/* 외부에서 직접 호출하지 않는 내부 함수 */
static void LCD_I2C_Init(void);
static void LCD_Send4Bit(uint8_t data);
static void LCD_SendByte(uint8_t data, uint8_t rs);
static void LCD_Command(uint8_t command);
static void LCD_Data(uint8_t data);

/* =====================================
 * I2C1 초기화
 * PB8 : SCL
 * PB9 : SDA
 * ===================================== */
static void LCD_I2C_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* GPIOB 및 I2C1 클럭 활성화 */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* I2C1 GPIO 설정 */
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF4_I2C1;

    HAL_GPIO_Init(GPIOB, &gpio);

    /* I2C1: 100kHz */
    hlcd_i2c.Instance = I2C1;
    hlcd_i2c.Init.ClockSpeed = 100000;
    hlcd_i2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hlcd_i2c.Init.OwnAddress1 = 0;
    hlcd_i2c.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;
    hlcd_i2c.Init.DualAddressMode =
        I2C_DUALADDRESS_DISABLE;
    hlcd_i2c.Init.OwnAddress2 = 0;
    hlcd_i2c.Init.GeneralCallMode =
        I2C_GENERALCALL_DISABLE;
    hlcd_i2c.Init.NoStretchMode =
        I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hlcd_i2c) != HAL_OK)
    {
        while (1)
        {
        }
    }
}

/* =====================================
 * LCD에 4비트 데이터 전송
 *
 * ENABLE HIGH → LOW로 변경해
 * LCD가 데이터를 받아들이도록 함
 * ===================================== */
static void LCD_Send4Bit(uint8_t data)
{
    uint8_t tx[2];

    tx[0] = data | LCD_BACKLIGHT | LCD_EN;
    tx[1] = data | LCD_BACKLIGHT;

    HAL_I2C_Master_Transmit(
        &hlcd_i2c,
        LCD_I2C_ADDR,
        tx,
        2,
        100
    );

    HAL_Delay(1);
}

/* =====================================
 * 8비트 데이터를 4비트씩 나누어 전송
 *
 * 상위 4비트 → 하위 4비트
 * ===================================== */
static void LCD_SendByte(uint8_t data, uint8_t rs)
{
    LCD_Send4Bit((data & 0xF0U) | rs);

    LCD_Send4Bit(
        ((data << 4) & 0xF0U) | rs
    );
}

/* LCD 제어 명령 전송 */
static void LCD_Command(uint8_t command)
{
    LCD_SendByte(command, 0);
}

/* LCD 문자 데이터 전송 */
static void LCD_Data(uint8_t data)
{
    LCD_SendByte(data, LCD_RS);
}

/* =====================================
 * LCD 초기화
 *
 * main.c에서 LCD_Init() 호출
 * ===================================== */
void LCD_Init(void)
{
    /* I2C1 먼저 초기화 */
    LCD_I2C_Init();

    HAL_Delay(50);

    /* LCD를 4비트 모드로 진입 */
    LCD_Send4Bit(0x30);
    HAL_Delay(5);

    LCD_Send4Bit(0x30);
    HAL_Delay(1);

    LCD_Send4Bit(0x30);
    HAL_Delay(1);

    LCD_Send4Bit(0x20);

    /* 4비트, 2줄, 5x8 문자 */
    LCD_Command(0x28);

    /* 표시 OFF */
    LCD_Command(0x08);

    /* 화면 초기화 */
    LCD_Clear();

    /* 문자 출력 후 커서 오른쪽 이동 */
    LCD_Command(0x06);

    /* 화면 ON / 커서 OFF */
    LCD_Command(0x0C);
}

/* =====================================
 * LCD 전체 화면 지우기
 * ===================================== */
void LCD_Clear(void)
{
    LCD_Command(0x01);
    HAL_Delay(2);
}

/* =====================================
 * 커서 위치 설정
 *
 * row = 0 : 첫 번째 줄
 * row = 1 : 두 번째 줄
 *
 * col = 0~15
 * ===================================== */
void LCD_SetCursor(uint8_t row, uint8_t col)
{
    if (row > 1 || col > 15)
    {
        return;
    }

    uint8_t address =
        (row == 0) ? 0x80U : 0xC0U;

    LCD_Command(address + col);
}

/* =====================================
 * 문자열 출력
 * ===================================== */
void LCD_Print(const char *text)
{
    if (text == NULL)
    {
        return;
    }

    while (*text != '\0')
    {
        LCD_Data((uint8_t)*text);
        text++;
    }
}

/* =====================================
 * 특정 위치에 문자열 출력
 * ===================================== */
void LCD_PrintAt(
    uint8_t row,
    uint8_t col,
    const char *text
)
{
    if (row > 1 || col > 15)
    {
        return;
    }

    LCD_SetCursor(row, col);
    LCD_Print(text);
}
