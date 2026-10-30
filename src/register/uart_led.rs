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

const RCC_BASE: usize = 0x4002_3800;
const GPIOA_BASE: usize = 0x4002_0000;
const USART2_BASE: usize = 0x4000_4400;

// RCC
const RCC_AHB1ENR: usize = RCC_BASE + 0x30;
const RCC_APB1ENR: usize = RCC_BASE + 0x40;

// GPIOA
const GPIOA_MODER: usize = GPIOA_BASE + 0x00;
const GPIOA_ODR: usize = GPIOA_BASE + 0x14;
const GPIOA_AFRL: usize = GPIOA_BASE + 0x20;

// USART2
const USART2_SR: usize = USART2_BASE + 0x00;
const USART2_DR: usize = USART2_BASE + 0x04;
const USART2_BRR: usize = USART2_BASE + 0x08;
const USART2_CR1: usize = USART2_BASE + 0x0C;
const USART2_CR2: usize = USART2_BASE + 0x10;
const USART2_CR3: usize = USART2_BASE + 0x14;

// 레지스터 읽기
fn reg_read(addr: usize) -> u32 {
    unsafe { read_volatile(addr as *const u32) }
}

// 레지스터 쓰기
fn reg_write(addr: usize, value: u32) {
    unsafe {
        write_volatile(addr as *mut u32, value);
    }
}

fn uart2_init() {
    // 1. GPIOA 클럭 활성화
    let value = reg_read(RCC_AHB1ENR);
    reg_write(RCC_AHB1ENR, value | (1 << 0));

    // 2. USART2 클럭 활성화
    let value = reg_read(RCC_APB1ENR);
    reg_write(RCC_APB1ENR, value | (1 << 17));
    // 클럭 활성화가 주변장치에 반영될 때까지 read-back합니다.
    let _ = reg_read(RCC_AHB1ENR);
    let _ = reg_read(RCC_APB1ENR);

    // 3. PA5 출력 모드 (LED)
    let value = reg_read(GPIOA_MODER);
    reg_write(GPIOA_MODER, (value & !(3 << 10)) | (1 << 10));

    // LED 초기 상태 OFF
    let value = reg_read(GPIOA_ODR);
    reg_write(GPIOA_ODR, value & !(1 << 5));

    // 4. PA2, PA3 Alternate Function 모드
    let value = reg_read(GPIOA_MODER);
    reg_write(
        GPIOA_MODER,
        (value & !((3 << 4) | (3 << 6))) | ((2 << 4) | (2 << 6)),
    );

    // 5. PA2, PA3 -> AF7 (USART2)
    let value = reg_read(GPIOA_AFRL);
    reg_write(
        GPIOA_AFRL,
        (value & !((0xF << 8) | (0xF << 12))) | ((7 << 8) | (7 << 12)),
    );

    // 6. USART2 설정 초기화
    reg_write(USART2_CR1, 0);
    reg_write(USART2_CR2, 0);
    reg_write(USART2_CR3, 0);

    // 7. Baud Rate = 9600
    // APB1 = 16 MHz, Oversampling = 16
    reg_write(USART2_BRR, 1667);

    // 8. TX 활성화 (TE = bit 3)
    // RX 활성화 (RE = bit 2)
    // USART 활성화 (UE = bit 13)
    reg_write(USART2_CR1, (1 << 3) | (1 << 2) | (1 << 13));
}

// 문자 수신
fn uart2_receiver_char() -> u8 {
    // RXNE = 1 될 때까지 대기
    while reg_read(USART2_SR) & (1 << 5) == 0 {}

    // 데이터 읽기
    (reg_read(USART2_DR) & 0xFF) as u8
}

#[entry]
fn main() -> ! {
    uart2_init();

    loop {
        let received_char = uart2_receiver_char();

        // '1' 수신 -> LED ON
        if received_char == b'1' {
            let value = reg_read(GPIOA_ODR);
            reg_write(GPIOA_ODR, value | (1 << 5));
        }

        // '0' 수신 -> LED OFF
        if received_char == b'0' {
            let value = reg_read(GPIOA_ODR);
            reg_write(GPIOA_ODR, value & !(1 << 5));
        }
    }
}
