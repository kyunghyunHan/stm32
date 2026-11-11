# STM32

## GPIOD
- GPIO Port D
- ex) PD3 = Port D의 3번 핀

## RCC : Reset and Clock Control
- GPIO 등의 주변장치에 클럭을 공급
- GPIO를 사용하기 전에 해당 GPIO의 클럭을 활성화해야 함

## GPIO_MODER : Mode Register
- 핀을 어떤 용도로 사용할지 설정
- 핀당 2bit

00 → Input
01 → Output
10 → Alternate Function
11 → Analog


## GPIO_OTYPER : Output Type Register
- GPIO의 출력 방식을 설정
- 핀당 1bit

0 → Push-Pull
1 → Open-Drain


## GPIO_OSPEEDR : Output Speed Register
- GPIO 출력 신호의 전환 속도를 설정
- 핀당 2bit

00 → Low speed
01 → Medium speed
10 → Fast speed
11 → High speed


## GPIO_PUPDR : Pull-Up / Pull-Down Register
- 핀의 내부 Pull-up / Pull-down 저항 설정
- 핀당 2bit

00 → Pull-up/down 없음
01 → Pull-up
10 → Pull-down
11 → Reserved


## GPIO_IDR : Input Data Register
- 핀으로 들어오는 값을 읽음
- 핀당 1bit

0 → LOW가 들어오고 있음
1 → HIGH가 들어오고 있음


## GPIO_ODR : Output Data Register
- 핀으로 출력할 값을 설정
- 핀당 1bit

0 → LOW 출력 (약 0V)
1 → HIGH 출력 (3.3V 계열에서는 대략 3.3V)


