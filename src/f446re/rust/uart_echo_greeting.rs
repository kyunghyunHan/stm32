
#![no_std]
#![no_main]

use core::panic::PanicInfo;
use core::ptr::{read_volatile, write_volatile};
use cortex_m_rt::entry;
use stm32f4 as _;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {
        cortex_m::asm::nop();
    }
}

/*
 * STM32F429ZI USART3
 *
 * Clock    : HSI = 16MHz, APB1 = 16MHz
 * Baudrate : 9600 bps
 * Format   : 8N1
 *
 * PD8 = USART3_TX (AF7)
 * PD9 = USART3_RX (AF7)
 *
 * ST-LINK Virtual COM Port
 */

const RCC_BASE: usize = 0x4002_3800;
const GPIOD_BASE: usize = 0x4002_0C00;
const USART3_BASE: usize = 0x4000_4800;

// RCC
const RCC_AHB1ENR: usize = RCC_BASE + 0x30;
const RCC_APB1ENR: usize = RCC_BASE + 0x40;

// GPIOD
const GPIOD_MODER: usize = GPIOD_BASE + 0x00;
const GPIOD_AFRH: usize = GPIOD_BASE + 0x24;

// USART3
const USART3_SR: usize = USART3_BASE + 0x00;
const USART3_DR: usize = USART3_BASE + 0x04;
const USART3_BRR: usize = USART3_BASE + 0x08;
const USART3_CR1: usize = USART3_BASE + 0x0C;
const USART3_CR2: usize = USART3_BASE + 0x10;
const USART3_CR3: usize = USART3_BASE + 0x14;

// USART 상태 비트
const TXE: u32 = 1 << 7;
const RXNE: u32 = 1 << 5;

// 레지스터 읽기
unsafe fn reg_read(addr: usize) -> u32 {
    unsafe { read_volatile(addr as *const u32) }
}

// 레지스터 쓰기
unsafe fn reg_write(addr: usize, value: u32) {
    unsafe { write_volatile(addr as *mut u32, value) };
}

// 레지스터 특정 비트 설정
unsafe fn reg_set(addr: usize, mask: u32) {
    unsafe {
        reg_write(addr, reg_read(addr) | mask);
    }
}

// 레지스터 특정 비트 초기화
unsafe fn reg_clear(addr: usize, mask: u32) {
    unsafe {
        reg_write(addr, reg_read(addr) & !mask);
    }
}

// USART3 초기화
fn uart3_init() {
    unsafe {
        // 1. GPIOD Clock Enable (bit 3)
        reg_set(RCC_AHB1ENR, 1 << 3);

        // 2. USART3 Clock Enable (bit 18)
        reg_set(RCC_APB1ENR, 1 << 18);

        // 3. PD8, PD9 Alternate Function (10)
        reg_clear(
            GPIOD_MODER,
            (3 << 16) | (3 << 18),
        );

        reg_set(
            GPIOD_MODER,
            (2 << 16) | (2 << 18),
        );

        // 4. PD8, PD9 = AF7 (USART3)
        reg_clear(
            GPIOD_AFRH,
            (0xF << 0) | (0xF << 4),
        );

        reg_set(
            GPIOD_AFRH,
            (7 << 0) | (7 << 4),
        );

        // 5. USART 비활성화 및 초기화
        reg_write(USART3_CR1, 0);
        reg_write(USART3_CR2, 0);
        reg_write(USART3_CR3, 0);

        // 6. Baudrate = 9600
        // APB1 = 16MHz, OVER8 = 0
        reg_write(USART3_BRR, 1667);

        // 7. USART 활성화
        // UE(bit13), TE(bit3), RE(bit2)
        reg_write(
            USART3_CR1,
            (1 << 13) | (1 << 3) | (1 << 2),
        );
    }
}

// 문자 1바이트 송신
fn uart3_send_char(c: u8) {
    unsafe {
        // TXE(bit7) = 1이면 TDR에 다음 데이터 기록 가능
        while (reg_read(USART3_SR) & TXE) == 0 {}

        // 데이터 전송
        reg_write(USART3_DR, c as u32);
    }
}

// 문자열 송신
fn uart3_send_string(s: &str) {
    for byte in s.bytes() {
        uart3_send_char(byte);
    }
}

// 문자 1바이트 수신
fn uart3_receive_char() -> u8 {
    unsafe {
        // RXNE(bit5) = 1이면 수신 데이터 준비
        while (reg_read(USART3_SR) & RXNE) == 0 {}

        // DR 읽기
        (reg_read(USART3_DR) & 0xFF) as u8
    }
}

#[entry]
fn main() -> ! {
    uart3_init();

    // 부팅 시 Hello World 출력
    uart3_send_string("Hello World!\r\n");

    loop {
        // PC에서 문자 수신
        let received = uart3_receive_char();

        // 수신한 문자를 다시 PC로 전송
        uart3_send_char(received);
    }
}
