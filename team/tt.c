// ADXL345 가속도를 I2C로 읽고 Roll·Pitch를 UART로 전송하는 통합 실습
#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// SCL=D15(PB8), SDA=D14(PB9), CS=3.3V, GND 공통; SDA/SCL 외부 풀업 필요
static uint16_t adxl345_addr = (0x53U << 1);

#define ADXL345_REG_DEVID     0x00U
#define ADXL345_REG_BW_RATE   0x2CU
#define ADXL345_REG_POWER_CTL 0x2DU
#define ADXL345_REG_DATA_FMT  0x31U
#define ADXL345_REG_DATAX0    0x32U

#define ADXL345_DEVICE_ID     0xE5U

#define ADXL345_SCALE_G       0.00390625f
#define RAD_TO_DEG            57.2957795f

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} ADXL345_Axis_t;

typedef struct
{
    float roll;
    float pitch;
} ADXL345_Angle_t;

static I2C_HandleTypeDef hi2c1;
static UART_HandleTypeDef huart3;

static void SystemClock_Config(void);
static void UART_Init(void);
static void UART_Send(const char *str);
static void UART_Send_Attitude(float roll, float pitch);

static uint8_t ADXL345_Init(void);
static uint8_t ADXL345_ReadXYZ(ADXL345_Axis_t *axis);
static uint8_t ADXL345_GetAngle(ADXL345_Angle_t *angle);

static uint8_t ADXL345_I2C_Init(void);
static HAL_StatusTypeDef ADXL345_Write(uint8_t reg,
                                       uint8_t value);
static HAL_StatusTypeDef ADXL345_Read(uint8_t reg,
                                      uint8_t *data,
                                      uint16_t length);

// HSI = 16MHz
// SYSCLK = 16MHz
// APB1 = 16MHz

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    __HAL_RCC_PWR_CLK_ENABLE();

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        while (1) {}
    }

    clk.ClockType =
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(
            &clk,
            FLASH_LATENCY_0) != HAL_OK)
    {
        while (1) {}
    }
}

static void UART_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();

    // USART3: PD8=TX, PD9=RX, 115200bps / 8N1
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF7_USART3;

    HAL_GPIO_Init(GPIOD, &gpio);

    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling =
        UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
        while (1) {}
    }
}

static void UART_Send(const char *str)
{
    if (str == NULL)
    {
        return;
    }

    size_t len = strlen(str);

    if (len == 0 || len > UINT16_MAX)
    {
        return;
    }

    HAL_UART_Transmit(
        &huart3,
        (uint8_t *)str,
        (uint16_t)len,
        100
    );
}

static void UART_Send_Attitude(float roll, float pitch)
{
    char buffer[80];

    int32_t r = (int32_t)(roll * 10.0f);
    int32_t p = (int32_t)(pitch * 10.0f);

    int32_t ra = (r < 0) ? -r : r;
    int32_t pa = (p < 0) ? -p : p;

    int len = snprintf(
        buffer,
        sizeof(buffer),
        "ROLL:%s%ld.%ld,PITCH:%s%ld.%ld\r\n",
        (r < 0) ? "-" : "",
        (long)(ra / 10),
        (long)(ra % 10),
        (p < 0) ? "-" : "",
        (long)(pa / 10),
        (long)(pa % 10)
    );

    if (len > 0 && len < (int)sizeof(buffer))
    {
        HAL_UART_Transmit(
            &huart3,
            (uint8_t *)buffer,
            (uint16_t)len,
            100
        );
    }
}

static uint8_t ADXL345_I2C_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    // 핀 지정이 없으면 HAL_GPIO_Init이 I2C 핀을 설정하지 못함
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF4_I2C1;

    HAL_GPIO_Init(GPIOB, &gpio);

    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode =
        I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode =
        I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode =
        I2C_NOSTRETCH_DISABLE;

    return HAL_I2C_Init(&hi2c1) == HAL_OK;
}

static HAL_StatusTypeDef ADXL345_Write(
    uint8_t reg,
    uint8_t value)
{
    return HAL_I2C_Mem_Write(
        &hi2c1,
        adxl345_addr,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        &value,
        1,
        100
    );
}

static HAL_StatusTypeDef ADXL345_Read(
    uint8_t reg,
    uint8_t *data,
    uint16_t length)
{
    return HAL_I2C_Mem_Read(
        &hi2c1,
        adxl345_addr,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        data,
        length,
        100
    );
}

static uint8_t ADXL345_Init(void)
{
    uint8_t id = 0;

    if (!ADXL345_I2C_Init())
    {
        UART_Send("I2C1 INIT ERROR\r\n");
        return 0;
    }

    // SDO가 GND면 0x53, 3.3V면 0x1D
    const uint16_t addresses[] = {0x53U << 1, 0x1DU << 1};
    uint8_t found = 0;
    for (uint32_t i = 0; i < 2U; ++i)
    {
        adxl345_addr = addresses[i];
        if (HAL_I2C_IsDeviceReady(&hi2c1, adxl345_addr, 3, 100) == HAL_OK)
        {
            found = 1;
            break;
        }
    }
    if (!found)
    {
        UART_Send("ADXL345 NOT FOUND: CHECK PB8/PB9, CS, POWER, PULLUPS\r\n");
        return 0;
    }

    if (ADXL345_Read(
            ADXL345_REG_DEVID,
            &id,
            1) != HAL_OK)
    {
        return 0;
    }

    if (id != ADXL345_DEVICE_ID)
    {
        UART_Send("ADXL345 DEVICE ID ERROR\r\n");
        return 0;
    }

    // Standby mode
    if (ADXL345_Write(
            ADXL345_REG_POWER_CTL,
            0x00) != HAL_OK)
    {
        return 0;
    }

    // 100Hz
    if (ADXL345_Write(
            ADXL345_REG_BW_RATE,
            0x0A) != HAL_OK)
    {
        return 0;
    }

    // FULL_RES, +/-2g
    if (ADXL345_Write(
            ADXL345_REG_DATA_FMT,
            0x08) != HAL_OK)
    {
        return 0;
    }

    // Measurement mode
    if (ADXL345_Write(
            ADXL345_REG_POWER_CTL,
            0x08) != HAL_OK)
    {
        return 0;
    }

    HAL_Delay(20);

    return 1;
}

static uint8_t ADXL345_ReadXYZ(ADXL345_Axis_t *axis)
{
    uint8_t data[6];

    if (axis == NULL)
    {
        return 0;
    }

    if (ADXL345_Read(
            ADXL345_REG_DATAX0,
            data,
            6) != HAL_OK)
    {
        return 0;
    }

    axis->x = (int16_t)(
        ((uint16_t)data[1] << 8) | data[0]
    );

    axis->y = (int16_t)(
        ((uint16_t)data[3] << 8) | data[2]
    );

    axis->z = (int16_t)(
        ((uint16_t)data[5] << 8) | data[4]
    );

    return 1;
}

static uint8_t ADXL345_CalculateAngle(
    const ADXL345_Axis_t *axis,
    ADXL345_Angle_t *angle)
{
    if (axis == NULL || angle == NULL)
    {
        return 0;
    }

    float ax = axis->x * ADXL345_SCALE_G;
    float ay = axis->y * ADXL345_SCALE_G;
    float az = axis->z * ADXL345_SCALE_G;

    float magnitude_sq =
        ax * ax + ay * ay + az * az;

    if (magnitude_sq < 0.0001f)
    {
        return 0;
    }

    angle->roll =
        atan2f(ay, az) * RAD_TO_DEG;

    angle->pitch =
        atan2f(
            -ax,
            sqrtf(ay * ay + az * az)
        ) * RAD_TO_DEG;
    return 1;
}

static uint8_t ADXL345_GetAngle(
    ADXL345_Angle_t *angle)
{
    ADXL345_Axis_t axis;

    if (angle == NULL)
    {
        return 0;
    }

    if (!ADXL345_ReadXYZ(&axis))
    {
        return 0;
    }

    return ADXL345_CalculateAngle(&axis, angle);
}

int main(void)
{
    // 프로젝트의 SysTick_Handler에서 HAL_IncTick()을 호출해야 함
    if (HAL_Init() != HAL_OK)
    {
        while (1) {}
    }
    SystemClock_Config();

    UART_Init();

    UART_Send("\r\n");
    UART_Send("=== ADXL345 START ===\r\n");

    HAL_Delay(100);
    while (!ADXL345_Init())
    {
        UART_Send("ADXL345 INIT ERROR: RETRY IN 1 SECOND\r\n");
        HAL_Delay(1000);
        HAL_I2C_DeInit(&hi2c1);
    }

    UART_Send("ADXL345 INIT OK\r\n");

    ADXL345_Angle_t angle = {0};

    while (1)
    {
        if (ADXL345_GetAngle(&angle))
        {
            UART_Send_Attitude(
                angle.roll,
                angle.pitch
            );
        }
        else
        {
            UART_Send(
                "ADXL345 READ ERROR\r\n"
            );
        }

        HAL_Delay(50);
    }
}
