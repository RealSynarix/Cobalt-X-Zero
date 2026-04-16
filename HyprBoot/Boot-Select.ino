#include <Arduino.h>
#include <stm32g4xx.h>

#define HID_BASE   0x08004000u
#define VDRIVE_BASE 0x08014000u

static void jump_to_image(uint32_t base) {
    uint32_t sp = *(__IO uint32_t *)base;
    uint32_t rh = *(__IO uint32_t *)(base + 4u);

    if ((sp & 0x2FFE0000u) != 0x20000000u) {
        while (1) {
        }
    }

    __disable_irq();
    SysTick->CTRL = 0;
    SysTick->VAL = 0;
    SCB->VTOR = base;
    __DSB();
    __ISB();
    __set_MSP(sp);
    ((void (*)(void))rh)();
}

void setup(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    (void)RCC->AHB2ENR;

    GPIOB->MODER &= ~(3u << (6u * 2u));
    GPIOB->PUPDR = (GPIOB->PUPDR & ~(3u << (6u * 2u))) | (1u << (6u * 2u));

    for (volatile uint32_t i = 0; i < 20000u; ++i) {
        __NOP();
    }

    if ((GPIOB->IDR & (1u << 6u)) == 0u) {
        jump_to_image(VDRIVE_BASE);
    } else {
        jump_to_image(HID_BASE);
    }
}

void loop(void) {
}
