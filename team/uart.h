
#ifndef UART_H
#define UART_H

#include "stm32f4xx_hal.h"

/*
 * USART3 초기화
 *
 * TX: PD8
 * RX: PD9
 * 115200bps, 8N1
 */
void UART_Init(void);

/* 문자열 UART 전송 */
void UART_Send(const char *str);

/* Roll/Pitch 값 UART 전송 */
void UART_Send_Attitude(
    float roll,
    float pitch
);

#endif
