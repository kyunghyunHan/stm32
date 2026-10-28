
#![no_std]
#![no_main]

use core::ptr::{read_volatile, write_volatile};
use cortex_m_rt::entry;
use panic_halt as _;

/* =========================================
 * 1. RCC 레지스터
 * ========================================= */

const RCC_BASE: usize = 0x4002_3800;
const RCC_AHB1ENR: usize = RCC_BASE + 0x30;

/* =========================================
 * 2. GPIOA 레지스터
 * ========================================= */

const GPIOA_BASE: usize = 0x4002_0000;

const GPIOA_MODER: usize = GPIOA_BASE + 0x00;
const GPIOA_ODR: usize = GPIOA_BASE + 0x14;

/* =========================================
 * 3. SysTick 레지스터
 * ========================================= */

const SYSTICK_BASE: usize = 0xE000_E010;

const SYSTICK_CTRL: usize = SYSTICK_BASE + 0x00;
const SYSTICK_LOAD: usize = SYSTICK_BASE + 0x04;
const SYSTICK_VAL: usize = SYSTICK_BASE + 0x08;

const CPU_CLOCK_HZ: u32 = 16_000_000;

/* =========================================
 * 4. 레지스터 읽기 / 쓰기
 * ========================================= */

unsafe fn reg_read(addr: usize) -> u32 {
    unsafe { read_volatile(addr as *const u32) }
}

unsafe fn reg_write(addr: usize, value: u32) {
    unsafe { write_volatile(addr as *mut u32, value) };
}

/* =========================================
 * 5. delay_ms
 * ========================================= */

fn delay_ms(ms: u32) {
    unsafe {
        // 1ms 주기 설정
        reg_write(SYSTICK_LOAD, CPU_CLOCK_HZ / 1000 - 1);

        // 현재 카운터 초기화
        reg_write(SYSTICK_VAL, 0);

        // CPU Clock 사용 + SysTick 활성화
        reg_write(SYSTICK_CTRL, (1 << 2) | (1 << 0));

        for _ in 0..ms {
            // COUNTFLAG(bit 16) 확인
            while (reg_read(SYSTICK_CTRL) & (1 << 16)) == 0 {
            }
        }

        // SysTick 비활성화
        reg_write(SYSTICK_CTRL, 0);
    }
}

/* =========================================
 * 6. main
 * ========================================= */

#[entry]
fn main() -> ! {
    unsafe {
        // GPIOA Clock Enable
        let value = reg_read(RCC_AHB1ENR);
        reg_write(RCC_AHB1ENR, value | (1 << 0));

        // PA5 Output Mode (01)
        let value = reg_read(GPIOA_MODER);
        reg_write(GPIOA_MODER, value & !(0x3 << 10));

        let value = reg_read(GPIOA_MODER);
        reg_write(GPIOA_MODER, value | (0x1 << 10));
    }

    loop {
        unsafe {
            // PA5 출력 상태 반전
            let value = reg_read(GPIOA_ODR);
            reg_write(GPIOA_ODR, value ^ (1 << 5));
        }

        delay_ms(3000);
    }
}
