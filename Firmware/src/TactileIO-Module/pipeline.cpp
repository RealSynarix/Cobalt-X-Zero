#include "pipeline.h"
#include "../HyprX-Module/macro.h"
#include <stm32g4xx.h>

#define DEBOUNCE_LOCKOUT 160u
#define ENCODER_DIVIDER 2
#define TICKS_PER_FRAME 32u

static uint8_t current_btn_state = 0;
static uint16_t locks[4] = {0};
static int32_t wheel_accum = 0;
static uint32_t last_enc_count = 0;
static uint8_t frame_tick = 0;
static volatile pipeline_report_t ready_report = {0, 0};

static inline void debounce_step(uint8_t idx, uint8_t pin) {
  if (locks[idx]!= 0) {
    locks[idx]--;
    return;
  }
  uint8_t phys = ((GPIOB->IDR >> pin) & 1u) == 0u? 1u : 0u;
  uint8_t log = (current_btn_state >> idx) & 1u;
  if (phys!= log) {
    if (phys) current_btn_state |= (1u << idx);
    else current_btn_state &= (uint8_t)~(1u << idx);
    locks[idx] = DEBOUNCE_LOCKOUT;
  }
}

void pipeline_init(void) {
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;
  __DSB();
  GPIOB->MODER &= ~(0xFFu << 6u);
  GPIOB->PUPDR = (GPIOB->PUPDR & ~(0xFFu << 6u)) | (0x55u << 6u);
  GPIOA->MODER = (GPIOA->MODER & ~0x0Fu) | 0x0Au;
  GPIOA->AFR[0] = (GPIOA->AFR[0] & ~0xFFu) | 0x11u;
  GPIOA->PUPDR = (GPIOA->PUPDR & ~0x0Fu) | 0x05u;
  GPIOA->OSPEEDR = (GPIOA->OSPEEDR & ~0x0Fu) | 0x0Fu;
  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
  __DSB();
  TIM2->CR1 = 0u;
  TIM2->SMCR = 3u;
  TIM2->CCMR1 = 0xF1F1u;
  TIM2->ARR = 0xFFFFFFFFu;
  TIM2->CNT = 0u;
  TIM2->CR1 = TIM_CR1_CEN;
  macro_init();
}

void pipeline_tick(void) {
  debounce_step(0, 3);
  debounce_step(1, 4);
  debounce_step(2, 5);
  debounce_step(3, 6);
  uint32_t enc = TIM2->CNT;
  int32_t delta = (int32_t)(enc - last_enc_count);
  last_enc_count = enc;
  uint8_t base = macro_update(current_btn_state, &delta);
  wheel_accum += delta;
  frame_tick++;
  if (frame_tick >= TICKS_PER_FRAME) {
    frame_tick = 0;
    int32_t steps = wheel_accum / (int32_t)ENCODER_DIVIDER;
    wheel_accum %= (int32_t)ENCODER_DIVIDER;
    if (steps > 127) steps = 127;
    if (steps < -127) steps = -127;
    uint8_t synth = macro_frame_tick();
    ready_report.buttons = (base | synth) & 0x07u;
    ready_report.wheel = (int8_t)steps;
  }
}

uint8_t pipeline_get_ready_report(pipeline_report_t *out) {
  __disable_irq();
  out->buttons = ready_report.buttons;
  out->wheel = ready_report.wheel;
  ready_report.wheel = 0;
  __enable_irq();
  return 1;
}
