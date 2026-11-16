// USART1과 USART3을 함께 초기화하는 실습 초안

#include <stdint.h>

int main(void)
{
    uart3_init();
    uart1_init();

    while (1)
    {
        
    }
}
