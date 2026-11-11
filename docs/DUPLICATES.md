# 중복 및 학습 변형

동일해도 원본 학습 기록을 보존하기 위해 파일을 합치거나 삭제하지 않았습니다.

## 바이트 단위 동일

- `src/f429zi/register/gpio_button_event_counter.c` / `src/f429zi/register/gpio_button_event_counter_variant.c`
- `src/f446re/register/uart_rx_led_control.c` / `src/f446re/register/uart_rx_led_control_variant.c`

## 주석과 공백 제외 동일

- `src/f429zi/register/gpio_button_event_counter.c` / `src/f429zi/register/gpio_button_event_counter_variant.c`
- `src/f446re/register/uart_rx_led_control.c` / `src/f446re/register/uart_rx_led_control_variant.c`

## 의도적으로 유지한 유사 구현

- F767 ADC/EXTI/Timer/UART의 Initialize_MCU: 각 파일의 독립 학습 흐름 보존.
- F429 UART greeting echo / 단순 echo: 부팅 출력 차이 보존.
- F446 UART 통신: 표본 디바운스와 blocking 디바운스 비교.
- GPIO 버튼: raw edge, open-drain, 20ms debounce, 이벤트 큐/long press 비교.
- Register와 HAL 명령 처리: GPIO/UART 설정과 시간 기반 처리 방식 비교.
