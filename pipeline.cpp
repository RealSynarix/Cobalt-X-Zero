#include "pipeline.h"
#include <stm32g4xx.h>

#define DEBOUNCE_LOCKOUT 160u
#define QUEUE_SIZE 8u
#define QUEUE_MASK (QUEUE_SIZE - 1u)

static volatile uint8_t current_btn_state = 0;
static volatile uint16_t lmb_lock = 0;
static volatile uint16_t rmb_lock = 0;
static volatile uint16_t mmb_lock = 0;
static volatile uint16_t mac_lock = 0;

static volatile uint8_t report_queue[QUEUE_SIZE];
static volatile uint8_t q_head = 0;
static volatile uint8_t q_tail = 0;

static inline void q_push(uint8_t state) {
  uint8_t next = (q_head + 1u) & QUEUE_MASK;
  if (next != q_tail) {
    report_queue[q_head] = state;
    q_head = next;
  }
}

static inline void debounce_logic(volatile uint16_t *lock, uint8_t bit_idx, uint8_t pin) {
  if (*lock > 0) {
    (*lock)--;
    return;
  }

  uint8_t physical_pressed = !((GPIOB->IDR >> pin) & 1u);
  uint8_t logical_pressed = (current_btn_state >> bit_idx) & 1u;

  if (physical_pressed != logical_pressed) {
    if (physical_pressed) {
      current_btn_state |= (1u << bit_idx);
    } else {
      current_btn_state &= ~(1u << bit_idx);
    }
    *lock = DEBOUNCE_LOCKOUT;
    q_push(current_btn_state);
  }
}

void pipeline_init(void) {
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
  (void)RCC->AHB2ENR;
  GPIOB->MODER &= ~(0xFFu << 6u);
  GPIOB->PUPDR = (GPIOB->PUPDR & ~(0xFFu << 6u)) | (0x55u << 6u);
}

void pipeline_tick(void) {
  debounce_logic(&lmb_lock, 0, 3);
  debounce_logic(&rmb_lock, 1, 4);
  debounce_logic(&mmb_lock, 2, 5);
  debounce_logic(&mac_lock, 3, 6);
}

uint8_t pipeline_dequeue(uint8_t *out) {
  if (q_head == q_tail) return 0;
  *out = report_queue[q_tail];
  q_tail = (q_tail + 1u) & QUEUE_MASK;
  return 1;
}