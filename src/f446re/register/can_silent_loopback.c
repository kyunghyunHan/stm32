#include "stm32f446xx.h"
#include <stdint.h>

// 디버거에서 결과 확인
volatile uint32_t received_id;
volatile uint8_t battery_soc;
volatile uint8_t battery_temp;
volatile uint32_t done;

static void can_init(void)
{
    // 1. CAN1 클럭 활성화
    RCC->APB1ENR |= (1U << 25);
    (void)RCC->APB1ENR;

    // CAN1을 초기 상태로 리셋
    RCC->APB1RSTR |=  (1U << 25);
    RCC->APB1RSTR &= ~(1U << 25);

    // 2. 슬립 해제 + 초기화 모드 요청
    CAN1->MCR &= ~(1U << 1);  // SLEEP = 0
    CAN1->MCR |=  (1U << 0);  // INRQ = 1

    // 초기화 모드 진입 확인: INAK = 1
    while ((CAN1->MSR & (1U << 0)) == 0) {
    }

    /*
     * 3. 비트 타이밍 + 내부 테스트 모드
     *
     * APB1 클럭이 16MHz일 때:
     * 분주 = 2
     * BS1 = 13 TQ
     * BS2 = 2 TQ
     *
     * 16MHz / (2 × (1 + 13 + 2)) = 500kbit/s
     *
     * 레지스터에는 실제 값보다 1 작은 값을 기록.
     */
    CAN1->BTR =
          (1U << 31)   // SILM: 외부 버스로 송신하지 않음
        | (1U << 30)   // LBKM: 내부 루프백
        | (1U << 20)   // TS2: 2 - 1
        | (12U << 16)  // TS1: 13 - 1
        | 1U;          // BRP: 2 - 1, SJW는 1 TQ

    // 4. 수신 필터 설정
    CAN1->FMR |= (1U << 0);     // 필터 초기화 모드

    CAN1->FA1R &= ~(1U << 0);   // 필터 0 비활성화
    CAN1->FM1R &= ~(1U << 0);   // 마스크 방식
    CAN1->FS1R |=  (1U << 0);   // 32비트 필터
    CAN1->FFA1R &= ~(1U << 0);  // FIFO0에 저장

    // 마스크가 0이면 모든 ID 허용
    CAN1->sFilterRegister[0].FR1 = 0;
    CAN1->sFilterRegister[0].FR2 = 0;

    CAN1->FA1R |= (1U << 0);    // 필터 0 활성화
    CAN1->FMR &= ~(1U << 0);    // 필터 설정 종료

    // 5. 초기화 모드 종료
    CAN1->MCR &= ~(1U << 0);

    while ((CAN1->MSR & (1U << 0)) != 0) {
    }
}

static void can_send(void)
{
    // 송신 메일박스 0이 빌 때까지 대기
    while ((CAN1->TSR & (1U << 26)) == 0) {
    }

    // 표준 ID = 0x100
    // IDE = 0: 표준 ID, RTR = 0: 데이터 프레임
    CAN1->sTxMailBox[0].TIR = (0x100U << 21);

    // 데이터 길이: 2바이트
    CAN1->sTxMailBox[0].TDTR = 2U;

    // 첫 번째 바이트 = 70, 두 번째 바이트 = 30
    CAN1->sTxMailBox[0].TDLR =
          70U
        | (30U << 8);

    CAN1->sTxMailBox[0].TDHR = 0;

    // TXRQ = 1: 송신 요청
    CAN1->sTxMailBox[0].TIR |= (1U << 0);
}

static void can_receive(void)
{
    // FIFO0의 수신 메시지 개수가 0이면 대기
    while ((CAN1->RF0R & 0x3U) == 0) {
    }

    uint32_t rir = CAN1->sFIFOMailBox[0].RIR;
    uint32_t dlc = CAN1->sFIFOMailBox[0].RDTR & 0xFU;
    uint32_t data = CAN1->sFIFOMailBox[0].RDLR;

    // 표준 데이터 프레임인지, 길이가 2인지 확인
    if ((rir & ((1U << 2) | (1U << 1))) == 0 &&
        dlc == 2U) {

        received_id = (rir >> 21) & 0x7FFU;
        battery_soc = data & 0xFFU;
        battery_temp = (data >> 8) & 0xFFU;
    }

    // RFOM0 = 1: 읽은 메시지를 FIFO에서 해제
    CAN1->RF0R = (1U << 5);
}

int main(void)
{
    /*
     * HAL_Init(), SystemClock_Config() 사용 안 함.
     * 리셋 기본 클럭 설정을 유지한다고 가정:
     * HSI 16MHz, APB1 분주 1.
     */
    can_init();
    can_send();
    can_receive();

    done = 1;

    while (1) {
        // 여기에 중단점 설정
    }
}