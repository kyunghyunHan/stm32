// #include "stm32f767xx.h"
#include "stm32f446xx.h"

int main(void)
{
    // 1) GPIOD 클럭 활성화
    // RCC_AHB1ENR 레지스터의 비트 구조:
    // [31:9] Reserved [8]GPIOIEN ... [3]GPIODEN [2]GPIOCEN [1]GPIOBEN [0]GPIOAEN
    //    RCC->AHB1ENR |= 0x00000008;  // bit 3 = 1 (0000 0000 0000 0000 0000 0000 0000 1000)
    RCC->AHB1ENR |= 0x00000001; // GPIOAEN (LED)

    RCC->AHB1ENR |= 0x00000004; // GPIOCEN

    // 2) PC6을 General-purpose output 모드로 설정 (MODER6[1:0] = 01)
    //    GPIOC->MODER  &= ~0x00003000;  // clear bits 13:12
    //    GPIOC->MODER  |=  0x00001000;  // set bit 12
    // LED: PC6 → PA5
    GPIOA->MODER &= ~0x00000C00; //32비트 4바이트
    GPIOA->MODER |= 0x00000400;
    // 3) PC6 중간 속도 설정 (OSPEEDR6[1:0] = 01)
    //    GPIOC->OSPEEDR &= ~0x00003000;  // clear bits 13:12
    //    GPIOC->OSPEEDR |=  0x00001000;  // set bit 12
    GPIOA->OSPEEDR &= ~0x00000C00;
    GPIOA->OSPEEDR |= 0x00000400;
    // 4) PD4를 입력으로 설정 (스위치)
    // MODER4[9:8] = 00 (입력모드)
    //    GPIOD->MODER &= ~0x00000300;  // bit 9,8 클리어 (0011 0000 0000)
    GPIOC->MODER &= ~0x0C000000;
    while (1)
    {
        // IDR 레지스터 읽기: bit 4 확인
        // IDR = [31:16]Reserved [15]IDR15 ... [4]IDR4 [3]IDR3 ... [0]IDR0
        //        if ((GPIOD->IDR & 0x00000010) == 0)  // bit 4 체크 (0001 0000)
        //        {
        //            // 스위치 눌림 → LED ON
        //        	GPIOC->ODR &= ~0x00000040; // clear bit 6
        //        }
        //        else
        //        {
        //            // 스위치 떼짐 → LED OFF
        //        	GPIOC->ODR |=  0x00000040; // set bit 6
        //        }
        if ((GPIOC->IDR & 0x00002000) == 0) // PC13 버튼
        {
            GPIOA->ODR |= 0x00000020; // PA5 LED ON
        }
        else
        {
            GPIOA->ODR &= ~0x00000020; // PA5 LED OFF
        }
    }
}
