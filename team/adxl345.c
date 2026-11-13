
#include "adxl345.h"
#include <math.h>

/* 이 모듈 내부에서 사용하는 I2C2 핸들 */
static I2C_HandleTypeDef hi2c2;

/* =====================================
 * I2C2 초기화
 * PB10 = SCL
 * PB11 = SDA
 * ===================================== */
static uint8_t ADXL345_I2C_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C2_CLK_ENABLE();

    /* I2C 핀: 오픈드레인, AF4 */
    gpio.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF4_I2C2;

    HAL_GPIO_Init(GPIOB, &gpio);

    /* I2C2: 100kHz, 7비트 주소 */
    hi2c2.Instance = I2C2;
    hi2c2.Init.ClockSpeed = 100000;
    hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode =
        I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.GeneralCallMode =
        I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode =
        I2C_NOSTRETCH_DISABLE;

    return HAL_I2C_Init(&hi2c2) == HAL_OK;
}

/* =====================================
 * ADXL345 레지스터 쓰기
 * ===================================== */
static HAL_StatusTypeDef ADXL345_Write(
    uint8_t reg,
    uint8_t value
)
{
    return HAL_I2C_Mem_Write(
        &hi2c2,
        ADXL345_ADDR,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        &value,
        1,
        100
    );
}

/* =====================================
 * ADXL345 레지스터 읽기
 * ===================================== */
static HAL_StatusTypeDef ADXL345_Read(
    uint8_t reg,
    uint8_t *data,
    uint16_t length
)
{
    return HAL_I2C_Mem_Read(
        &hi2c2,
        ADXL345_ADDR,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        data,
        length,
        100
    );
}

/* =====================================
 * ADXL345 초기화
 * ===================================== */
uint8_t ADXL345_Init(void)
{
    uint8_t id = 0;

    /* 1. I2C2 초기화 */
    if (!ADXL345_I2C_Init())
    {
        return 0;
    }

    /* 2. 센서 연결 확인 */
    if (HAL_I2C_IsDeviceReady(
        &hi2c2,
        ADXL345_ADDR,
        3,
        100
    ) != HAL_OK)
    {
        return 0;
    }

    /* 3. 장치 ID 읽기 */
    if (ADXL345_Read(
        ADXL345_REG_DEVID,
        &id,
        1
    ) != HAL_OK)
    {
        return 0;
    }

    /* ADXL345의 ID는 0xE5 */
    if (id != ADXL345_DEVICE_ID)
    {
        return 0;
    }

    /* 4. 설정을 위해 대기 모드 진입 */
    if (ADXL345_Write(
        ADXL345_REG_POWER_CTL,
        0x00
    ) != HAL_OK)
    {
        return 0;
    }

    /* 5. 데이터 출력 속도: 100Hz */
    if (ADXL345_Write(
        ADXL345_REG_BW_RATE,
        0x0A
    ) != HAL_OK)
    {
        return 0;
    }

    /* 6. FULL_RES, 측정 범위 +/-2g */
    if (ADXL345_Write(
        ADXL345_REG_DATA_FMT,
        0x08
    ) != HAL_OK)
    {
        return 0;
    }

    /* 7. 측정 모드 활성화 */
    if (ADXL345_Write(
        ADXL345_REG_POWER_CTL,
        0x08
    ) != HAL_OK)
    {
        return 0;
    }

    HAL_Delay(20);

    return 1;
}

/* =====================================
 * X/Y/Z 가속도 읽기
 *
 * 0x32부터 6바이트 연속 읽기
 * ===================================== */
uint8_t ADXL345_ReadXYZ(ADXL345_Axis_t *axis)
{
    uint8_t data[6];

    if (axis == NULL)
    {
        return 0;
    }

    if (ADXL345_Read(
        ADXL345_REG_DATAX0,
        data,
        6
    ) != HAL_OK)
    {
        return 0;
    }

    /* 하위/상위 바이트 결합 */
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

/* =====================================
 * Roll / Pitch 계산
 *
 * Roll  : X축 중심 회전
 * Pitch : Y축 중심 회전
 *
 * 중력 방향을 이용하므로
 * 정지하거나 천천히 움직일 때 적합
 * ===================================== */
void ADXL345_CalculateAngle(
    const ADXL345_Axis_t *axis,
    ADXL345_Angle_t *angle
)
{
    if (axis == NULL || angle == NULL)
    {
        return;
    }

    /* 원시 데이터를 g 단위로 변환 */
    float ax = axis->x * ADXL345_SCALE_G;
    float ay = axis->y * ADXL345_SCALE_G;
    float az = axis->z * ADXL345_SCALE_G;

    /* 0 벡터 방지 */
    float magnitude_sq =
        ax * ax + ay * ay + az * az;

    if (magnitude_sq < 0.0001f)
    {
        return;
    }

    /* Roll 계산 */
    angle->roll =
        atan2f(ay, az) * RAD_TO_DEG;

    /* Pitch 계산 */
    angle->pitch =
        atan2f(
            -ax,
            sqrtf(ay * ay + az * az)
        ) * RAD_TO_DEG;
}

/* =====================================
 * 가속도 읽기 + 각도 계산
 *
 * main.c에서 사용하는 통합 함수
 * ===================================== */
uint8_t ADXL345_GetAngle(ADXL345_Angle_t *angle)
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

    ADXL345_CalculateAngle(&axis, angle);

    return 1;
}
