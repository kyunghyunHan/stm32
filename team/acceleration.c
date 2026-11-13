
#include "stm32f4xx_hal.h"
#include "adxl345.h"
#include "uart.h"

/* 기존 프로젝트의 시스템 클럭 설정 함수 */
void SystemClock_Config(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* USART3 초기화 */
    UART_Init();

    UART_Send("=== ADXL345 START ===\r\n");

    /* I2C2 + ADXL345 초기화 */
    if (!ADXL345_Init())
    {
        UART_Send("ADXL345 INIT ERROR\r\n");

        while (1)
        {
            HAL_Delay(1000);
        }
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
            UART_Send("ADXL345 READ ERROR\r\n");
        }

        HAL_Delay(50);
    }
}
