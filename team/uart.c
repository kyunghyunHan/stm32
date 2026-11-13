
#include "uart.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* 이 모듈 내부에서 사용하는 USART3 핸들 */
static UART_HandleTypeDef huart3;

/* =====================================
 * USART3 초기화
 *
 * PD8: TX
 * PD9: RX
 * ===================================== */
void UART_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();

    /* USART3 GPIO 설정 */
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF7_USART3;

    HAL_GPIO_Init(GPIOD, &gpio);

    /* USART3 통신 설정 */
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

/* =====================================
 * 문자열 UART 전송
 * ===================================== */
void UART_Send(const char *str)
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

/* =====================================
 * Roll/Pitch 데이터 전송
 *
 * Rust에서 읽을 형식:
 * ROLL:12.3,PITCH:-5.4
 *
 * printf의 %f 기능 없이 전송
 * ===================================== */
void UART_Send_Attitude(
    float roll,
    float pitch
)
{
    char buffer[80];

    /* 소수 첫째 자리까지 정수화 */
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
