
#include <stdint.h>

/*
 * UART 문자로 LED 제어
 *
 * '1' 수신 → LED ON
 * '0' 수신 → LED OFF
 *
 * 
 * 레지스터 직접 접근 실습
 */

// 1. RCC_BASE 주소 정의

// 2. GPIOC_BASE 주소 정의

// 3. GPIOD_BASE 주소 정의

// 4. USART2_BASE 주소 정의


// 5. RCC_AHB1ENR 레지스터 정의

// 6. RCC_APB1ENR 레지스터 정의


// 7. GPIOC_MODER 레지스터 정의

// 8. GPIOC_ODR 레지스터 정의

// 9. GPIOD_MODER 레지스터 정의

// 10. GPIOD_AFRL 레지스터 정의


// 11. USART2_CR1 레지스터 정의

// 12. USART2_CR2 레지스터 정의

// 13. USART2_CR3 레지스터 정의

// 14. USART2_BRR 레지스터 정의

// 15. USART2_ISR 레지스터 정의

// 16. USART2_RDR 레지스터 정의


void UART2_Init(void)
{
    // 17. GPIOC 클럭 활성화 (bit 2)


    // 18. GPIOD 클럭 활성화 (bit 3)


    // 19. USART2 클럭 활성화 (bit 17)


    // 20. PC6을 Output 모드(01)로 설정


    // 21. LED 초기 상태 OFF (PC6 HIGH)


    // 22. PD5, PD6을 Alternate Function 모드(10)로 설정


    // 23. PD5, PD6을 AF7로 설정


    // 24. USART2 비활성화 및 CR1, CR2, CR3 초기화


    // 25. Baud Rate를 9600bps로 설정
    //     APB1 = 16MHz 가정


    // 26. USART2 송신 및 수신 활성화


    // 27. USART2 활성화

}


char UART2_ReceiveChar(void)
{
    // 28. RXNE 플래그가 1이 될 때까지 대기


    // 29. 수신 데이터 레지스터에서 문자 읽고 반환

}


int main(void)
{
    char received_char = 0;

    // 30. UART2_Init() 호출


    while (1)
    {
        // 31. UART로 문자 1개 수신


        // 32. 수신 문자가 '1'이면
        //     PC6 LOW → LED ON


        // 33. 수신 문자가 '0'이면
        //     PC6 HIGH → LED OFF


        // 34. 다른 문자라면 LED 상태 유지

    }
}
