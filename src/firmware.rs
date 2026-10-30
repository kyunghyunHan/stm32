#![no_std]
#![no_main]

use core::panic::PanicInfo;
use cortex_m_rt::entry;

#[cfg(feature = "f446")]
use stm32f4::stm32f446 as pac;
#[cfg(feature = "f767")]
use stm32f7::stm32f767 as pac;

#[entry]
fn main() -> ! {
    let _peripherals = pac::Peripherals::take().unwrap();

    // Add GPIO/UART setup here for the selected MCU and board.
    loop {
        cortex_m::asm::nop();
    }
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {
        cortex_m::asm::nop();
    }
}
