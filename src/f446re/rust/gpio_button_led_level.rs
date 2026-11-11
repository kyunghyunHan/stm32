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
//BASE
const RCC_BASE: usize = 0x4002_3800;
const GPIOA_BASE: usize = 0x4002_0000;
const GPIOC_BASE: usize = 0x4002_0800;

//클럭 bus
const RCC_AHB1ENR: usize = RCC_BASE + 0x30;

//mode
const GPIOA_MODER: usize = GPIOA_BASE + 0x00;
const GPIOC_MODER: usize = GPIOC_BASE + 0x00;
//I/0
const GPIOC_IDR: usize = GPIOC_BASE + 0x10;
const GPIOA_ODR: usize = GPIOA_BASE + 0x14;

unsafe fn reg_read(addr: usize) -> u32 {
    unsafe { read_volatile(addr as *const u32) }
}

unsafe fn reg_write(addr: usize, value: u32) {
    unsafe { write_volatile(addr as *mut u32, value) };
}
#[entry]
fn main() -> ! {
    unsafe {
        let value = reg_read(RCC_AHB1ENR);
        // GPIOA와 GPIOC 클럭을 함께 활성화합니다.
        reg_write(RCC_AHB1ENR, value | (1 << 2) | (1 << 0));
        let _ = reg_read(RCC_AHB1ENR);

        //MODER
        let value = reg_read(GPIOC_MODER);
        reg_write(GPIOC_MODER, value & !(3 << 26));

        /*RMW
        Read
        Modify
        Write
         */
        reg_write(GPIOA_MODER, reg_read(GPIOA_MODER) & !(3 << 10));
        reg_write(GPIOA_MODER, reg_read(GPIOA_MODER) | (1 << 10));

        
        reg_write(GPIOA_ODR, reg_read(GPIOA_ODR) & !(1u32 << 5));
        loop {
            let button = (reg_read(GPIOC_IDR) >> 13) & 1;
            if button == 0 {
                reg_write(GPIOA_ODR, reg_read(GPIOA_ODR) | (1 << 5));
            } else {
                reg_write(GPIOA_ODR, reg_read(GPIOA_ODR) & !(1 << 5));
            }
        }
    }
}
