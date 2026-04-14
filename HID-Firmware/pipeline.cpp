#include "pipeline.h"
#include "macro.h"
#include <stm32g4xx.h>

#define DEBOUNCE_LOCKOUT 160u
#define ENCODER_DIVIDER 2
#define TICKS_PER_FRAME 32u

static volatile uint8_t current_btn_state = 0;
static volatile uint16_t locks[4] = { 0, 0, 0, 0 };
static volatile int32_t wheel_accum = 0;
static volatile uint32_t last_enc_count = 0;
static volatile uint8_t frame_tick = 0;
static volatile pipeline_report_t ready_report = { 0, 0 };

static inline void debounce_step(uint8_t idx, uint8_t pin) {
  if (locks[idx] > 0) {
    locks[idx]--;
    return;
  }
  uint8_t phys = !((GPIOB->IDR >> pin) & 1u);
  uint8_t log = (current_btn_state >> idx) & 1u;
  if (phys != log) {
    if (phys) current_btn_state |= (1u << idx);
    else current_btn_state &= ~(1u << idx);
    locks[idx] = DEBOUNCE_LOCKOUT;
  }
}

void pipeline_init(void) {
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;
  (void)RCC->AHB2ENR;
  GPIOB->MODER &= ~(0xFFu << 6u);
  GPIOB->PUPDR = (GPIOB->PUPDR & ~(0xFFu << 6u)) | (0x55u << 6u);
  GPIOA->MODER = (GPIOA->MODER & ~0x0Fu) | 0x0Au;
  GPIOA->AFR[0] = (GPIOA->AFR[0] & ~0xFFu) | 0x11u;
  GPIOA->PUPDR = (GPIOA->PUPDR & ~0x0Fu) | 0x05u;
  GPIOA->OSPEEDR = (GPIOA->OSPEEDR & ~0x0Fu) | 0x0Fu;

  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
  (void)RCC->APB1ENR1;
  TIM2->CR1 = 0;
  TIM2->SMCR = 3;
  TIM2->CCMR1 = 0xF1F1;
  TIM2->CCER = 0;
  TIM2->ARR = 0xFFFFFFFF;
  TIM2->CNT = 0;
  TIM2->CR1 = 1;

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

  ready_report.buttons = macro_update(current_btn_state, &delta);
  wheel_accum += delta;

  if (++frame_tick >= TICKS_PER_FRAME) {
    frame_tick = 0;
    int32_t steps = wheel_accum / ENCODER_DIVIDER;
    wheel_accum %= ENCODER_DIVIDER;
    ready_report.wheel = (steps > 127) ? 127 : (steps < -127) ? -127
                                                              : (int8_t)steps;
  }
}

uint8_t pipeline_get_ready_report(pipeline_report_t *out) {
  out->buttons = ready_report.buttons;
  out->wheel = ready_report.wheel;
  ready_report.wheel = 0;
  return 1;
}