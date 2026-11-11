
#![no_std]
#![no_main]

use cortex_m::delay::Delay;
use cortex_m_rt::entry;
use panic_halt as _;

use stm32f7xx_hal::{
    pac,
    prelude::*,
};

#[entry]
fn main() -> ! {
    // 1. MCU 주변장치 가져오기
    let dp = pac::Peripherals::take().unwrap();
    let cp = cortex_m::Peripherals::take().unwrap();

    // 2. GPIOA 초기화
    let gpioa = dp.GPIOA.split();

    // 3. PA5를 Push-Pull 출력으로 설정
    let mut led = gpioa.pa5.into_push_pull_output();

    // 4. LED 초기 OFF
    led.set_low();

    // 5. SysTick 기반 Delay 생성
    // 초기 시스템 클럭 HSI 16MHz 가정
    let mut delay = Delay::new(cp.SYST, 16_000_000);

    // 6. LED 토글 반복
    loop {
        led.toggle();

        // 3000ms = 3초
        delay.delay_ms(3000_u32);
    }
}
