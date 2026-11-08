
#ifndef LCD_H
#define LCD_H

#include "stm32f4xx_hal.h"

/* LCD I2C 주소: 기본 0x27 */
#define LCD_I2C_ADDR   (0x27U << 1)

/* LCD 초기화: I2C1 PB8/PB9 포함 */
void LCD_Init(void);

/* LCD 전체 화면 지우기 */
void LCD_Clear(void);

/* LCD 커서 위치 설정: row 0~1, col 0~15 */
void LCD_SetCursor(uint8_t row, uint8_t col);

/* 현재 커서 위치에 문자열 출력 */
void LCD_Print(const char *text);

/* 원하는 위치에 문자열 출력 */
void LCD_PrintAt(uint8_t row, uint8_t col, const char *text);

#endif
