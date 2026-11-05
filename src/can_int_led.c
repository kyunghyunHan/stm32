#include <stdint.h>

#define RCC_BASE        0x40023800UL
#define CAN1_BASE       0x40006400UL
#define GPIOB_BASE      0x40020400UL

#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

#define GPIOB_MODER     (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_ODR       (*(volatile uint32_t *)(GPIOB_BASE + 0x14UL))

#define CAN_MCR         (*(volatile uint32_t *)(CAN1_BASE + 0x00UL))
#define CAN_MSR         (*(volatile uint32_t *)(CAN1_BASE + 0x04UL))
#define CAN_TSR         (*(volatile uint32_t *)(CAN1_BASE + 0x08UL))
#define CAN_RF0R        (*(volatile uint32_t *)(CAN1_BASE + 0x0CUL))
#define CAN_BTR         (*(volatile uint32_t *)(CAN1_BASE + 0x1CUL))

#define CAN_FMR         (*(volatile uint32_t *)(CAN1_BASE + 0x200UL))
#define CAN_FM1R        (*(volatile uint32_t *)(CAN1_BASE + 0x204UL))
#define CAN_FS1R        (*(volatile uint32_t *)(CAN1_BASE + 0x20CUL))
#define CAN_FFA1R       (*(volatile uint32_t *)(CAN1_BASE + 0x214UL))
#define CAN_FA1R        (*(volatile uint32_t *)(CAN1_BASE + 0x21CUL))

#define CAN_FiR1(bank)  (*(volatile uint32_t *)(CAN1_BASE + 0x240UL + ((bank) * 8UL)))
#define CAN_FiR2(bank)  (*(volatile uint32_t *)(CAN1_BASE + 0x244UL + ((bank) * 8UL)))

#define CAN_TI0R        (*(volatile uint32_t *)(CAN1_BASE + 0x180UL))
#define CAN_TDT0R       (*(volatile uint32_t *)(CAN1_BASE + 0x184UL))
#define CAN_TDL0R       (*(volatile uint32_t *)(CAN1_BASE + 0x188UL))
#define CAN_TDH0R       (*(volatile uint32_t *)(CAN1_BASE + 0x18CUL))

#define CAN_RI0R        (*(volatile uint32_t *)(CAN1_BASE + 0x1B0UL))
#define CAN_RDT0R       (*(volatile uint32_t *)(CAN1_BASE + 0x1B4UL))
#define CAN_RDL0R       (*(volatile uint32_t *)(CAN1_BASE + 0x1B8UL))
#define CAN_RDH0R       (*(volatile uint32_t *)(CAN1_BASE + 0x1BCUL))

uint8_t TxData[4] = {0x01, 0x02, 0x03, 0x04};
uint8_t RxData[4] = {0};
uint8_t match = 0;

void gpio_init(void)
{
    RCC_AHB1ENR |= (1U << 1);   // GPIOB clock enable

    // PB7 output
    GPIOB_MODER &= ~(3U << 14);
    GPIOB_MODER |=  (1U << 14);

    // LED OFF
    GPIOB_ODR &= ~(1U << 7);
}

void can1_init_loopback(void)
{
    // CAN1 clock enable
    RCC_APB1ENR |= (1U << 25);

    // init mode request
    CAN_MCR |= (1U << 0);
    while (!(CAN_MSR & (1U << 0)))
    {
    }

    // BTR:
    // LBKM = 1 (loopback)
    // SJW  = 1TQ  -> 0
    // TS1  = 12TQ -> 11
    // TS2  = 3TQ  -> 2
    // BRP  = 2    -> register value 1
    CAN_BTR = (1U << 30) | (2U << 20) | (11U << 16) | (1U << 0);

    // Filter init mode
    CAN_FMR |= (1U << 0);

    // Filter 0: mask mode, 32-bit, FIFO0, accept all
    CAN_FM1R &= ~(1U << 0);
    CAN_FS1R |=  (1U << 0);
    CAN_FFA1R &= ~(1U << 0);
    CAN_FiR1(0) = 0x00000000;
    CAN_FiR2(0) = 0x00000000;
    CAN_FA1R |= (1U << 0);

    // leave filter init mode
    CAN_FMR &= ~(1U << 0);

    // leave init mode
    CAN_MCR &= ~(1U << 0);
    while (CAN_MSR & (1U << 0))
    {
    }
}

void can1_transmit_std(uint16_t std_id, uint8_t *data, uint8_t dlc)
{
    while (!(CAN_TSR & (1U << 26)))   // TME0
    {
    }

    CAN_TI0R = ((uint32_t)std_id << 21);
    CAN_TDT0R = (dlc & 0xFU);
    CAN_TDL0R = ((uint32_t)data[3] << 24) |
                ((uint32_t)data[2] << 16) |
                ((uint32_t)data[1] << 8)  |
                ((uint32_t)data[0]);
    CAN_TDH0R = 0x00000000;

    CAN_TI0R |= (1U << 0);   // TXRQ
}

void can1_receive_std(uint16_t *std_id, uint8_t *data, uint8_t *dlc)
{
    while ((CAN_RF0R & 0x3U) == 0U)
    {
    }

    *std_id = (uint16_t)((CAN_RI0R >> 21) & 0x7FFU);
    *dlc = (uint8_t)(CAN_RDT0R & 0xFU);

    uint32_t rdlr = CAN_RDL0R;

    data[0] = (uint8_t)(rdlr & 0xFFU);
    data[1] = (uint8_t)((rdlr >> 8) & 0xFFU);
    data[2] = (uint8_t)((rdlr >> 16) & 0xFFU);
    data[3] = (uint8_t)((rdlr >> 24) & 0xFFU);

    // release FIFO0
    CAN_RF0R |= (1U << 5);
}

int main(void)
{
    uint16_t rx_id = 0;
    uint8_t rx_dlc = 0;

    gpio_init();
    can1_init_loopback();

    can1_transmit_std(0x123, TxData, 4);
    can1_receive_std(&rx_id, RxData, &rx_dlc);

    if ((rx_id == 0x123) &&
        (rx_dlc == 4) &&
        (RxData[0] == 0x01) &&
        (RxData[1] == 0x02) &&
        (RxData[2] == 0x03) &&
        (RxData[3] == 0x04))
    {
        match = 1;
        GPIOB_ODR |= (1U << 7);   // LED ON
    }
    else
    {
        match = 0;
        GPIOB_ODR &= ~(1U << 7);  // LED OFF
    }

    while (1)
    {
    }
}