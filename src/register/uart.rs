#![no_std]
#![no_main]

use core::panic::PanicInfo;
use core::ptr::{read_volatile, write_volatile};
use cortex_m_rt::entry;
// F446 장치의 인터럽트 벡터 테이블을 링크합니다.
use stm32f4 as _;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {
        cortex_m::asm::nop();
    }
}

const RCC_BASE: usize = 0x40023800;
const GPIOD_BASE: usize = 0x40020C00;
const USART3_BASE: usize = 0x40004800;

const RCC_AHB1ENR: usize = RCC_BASE + 0x30;
const RCC_APB1ENR: usize = RCC_BASE + 0x40;

const GPIOD_MODER: usize = GPIOD_BASE + 0x00;
const GPIOD_APRH: usize = GPIOD_BASE + 0x24;

const USART3_SR: usize = USART3_BASE + 0x00;
