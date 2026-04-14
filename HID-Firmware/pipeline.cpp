#include "pipeline.h"
#include <stm32g4xx.h>

#define DEBOUNCE_LOCKOUT 160u
#define QUEUE_SIZE 8u
#define QUEUE_MASK (QUEUE_SIZE - 1u)
#define ENCODER_DIVIDER 2
#define TICKS_PER_FRAME 32u

static volatile uint8_t current_btn_state = 0;
static volatile uint16_t lmb_lock = 0;
static volatile uint16_t rmb_lock = 0;
static volatile uint16_t mmb_lock = 0;
static volatile uint16_t mac_lock = 0;

static volatile pipeline_report_t report_queue[QUEUE_SIZE];
static volatile uint8_t q_head = 0;
static volatile uint8_t q_tail = 0;

static uint32_t last_enc_count = 0;
static int32_t wheel_accum = 0;
static uint8_t frame_tick = 0;

static inline void q_push(pipeline_report_t r) {
  uint8_t next = (q_head + 1u) & QUEUE_MASK;
  if (next != q_tail) {
    report_queue[q_head].buttons = r.buttons;
    report_queue[q_head].wheel = r.wheel;
    q_head = next;
  }
}

static inline void debounce_logic(volatile uint16_t *lock, uint8_t bit_idx, uint8_t pin) {
  if (*lock > 0) {
    (*lock)--;
    return;
  }
  uint8_t physical = !((GPIOB->IDR >> pin) & 1u);
  uint8_t logical = (current_btn_state >> bit_idx) & 1u;
  if (physical != logical) {
    if (physical) current_btn_state |= (1u << bit_idx);
    else current_btn_state &= ~(1u << bit_idx);
    *lock = DEBOUNCE_LOCKOUT;
    pipeline_report_t r = { current_btn_state, 0 };
    q_push(r);
  }
}

void pipeline_init(void) {
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
  (void)RCC->AHB2ENR;
  GPIOB->MODER &= ~(0xFFu << 6u);
  GPIOB->PUPDR = (GPIOB->PUPDR & ~(0xFFu << 6u)) | (0x55u << 6u);

  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
  (void)RCC->AHB2ENR;
  GPIOA->MODER = (GPIOA->MODER & ~0x0Fu) | 0x0Au;
  GPIOA->AFR[0] = (GPIOA->AFR[0] & ~0xFFu) | 0x11u;
  GPIOA->PUPDR = (GPIOA->PUPDR & ~0x0Fu) | 0x05u;
  GPIOA->OSPEEDR = (GPIOA->OSPEEDR & ~0x0Fu) | 0x0Fu;

  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
  (void)RCC->APB1ENR1;

  TIM2->CR1 = 0;
  TIM2->SMCR = TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0;
  TIM2->CCMR1 = TIM_CCMR1_CC1S_0
                | (0x0Fu << TIM_CCMR1_IC1F_Pos)
                | TIM_CCMR1_CC2S_0
                | (0x0Fu << TIM_CCMR1_IC2F_Pos);
  TIM2->CCER = 0;
  TIM2->PSC = 0;
  TIM2->ARR = 0xFFFFFFFFu;
  TIM2->CNT = 0;
  TIM2->EGR = TIM_EGR_UG;
  TIM2->CR1 = TIM_CR1_CEN;
  last_enc_count = 0;
  wheel_accum = 0;
  frame_tick = 0;
}

void pipeline_tick(void) {
  debounce_logic(&lmb_lock, 0, 3);
  debounce_logic(&rmb_lock, 1, 4);
  debounce_logic(&mmb_lock, 2, 5);
  debounce_logic(&mac_lock, 3, 6);

  uint32_t current_count = TIM2->CNT;
  wheel_accum += (int32_t)(current_count - last_enc_count);
  last_enc_count = current_count;
  if (++frame_tick >= TICKS_PER_FRAME) {
    frame_tick = 0;
    int32_t steps = wheel_accum / ENCODER_DIVIDER;
    wheel_accum -= steps * ENCODER_DIVIDER;
    if (steps != 0) {
      int8_t w = (steps > 127) ? 127 : (steps < -127) ? -127
                                                      : (int8_t)steps;
      pipeline_report_t r = { current_btn_state, w };
      q_push(r);
    }
  }
}

uint8_t pipeline_dequeue(pipeline_report_t *out) {
  if (q_head == q_tail) return 0;
  out->buttons = report_queue[q_tail].buttons;
  out->wheel = report_queue[q_tail].wheel;
  q_tail = (q_tail + 1u) & QUEUE_MASK;
  return 1;
}