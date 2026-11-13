
#ifndef ADXL345_H
#define ADXL345_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ADXL345 I2C 주소: SDO = GND */
#define ADXL345_ADDR           (0x53U << 1)

/* ADXL345 레지스터 주소 */
#define ADXL345_REG_DEVID      0x00U
#define ADXL345_REG_BW_RATE    0x2CU
#define ADXL345_REG_POWER_CTL  0x2DU
#define ADXL345_REG_DATA_FMT   0x31U
#define ADXL345_REG_DATAX0     0x32U

#define ADXL345_DEVICE_ID      0xE5U

/* FULL_RES 모드의 가속도 변환 비율 */
#define ADXL345_SCALE_G        (1.0f / 256.0f)

/* 라디안 -> 도 */
#define RAD_TO_DEG             57.2957795f

/* 3축 가속도 원시 데이터 */
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} ADXL345_Axis_t;

/* Roll/Pitch 각도 */
typedef struct
{
    float roll;
    float pitch;
} ADXL345_Angle_t;

/*
 * ADXL345 초기화
 *
 * - I2C2: PB10/PB11 설정
 * - ADXL345 장치 ID 확인
 * - 100Hz, FULL_RES, +/-2g 설정
 *
 * 반환값: 1 = 성공, 0 = 실패
 */
uint8_t ADXL345_Init(void);

/* X/Y/Z 원시 가속도 데이터 읽기 */
uint8_t ADXL345_ReadXYZ(ADXL345_Axis_t *axis);

/* 원시 데이터로 Roll/Pitch 계산 */
void ADXL345_CalculateAngle(
    const ADXL345_Axis_t *axis,
    ADXL345_Angle_t *angle
);

/* 센서 읽기와 각도 계산을 한 번에 실행 */
uint8_t ADXL345_GetAngle(ADXL345_Angle_t *angle);

#endif
