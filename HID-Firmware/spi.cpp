#include "spi.h"
#include <stm32g4xx.h>

#define CS_LOW() (GPIOA->BSRR = (uint32_t)(1u << (4u + 16u)))
#define CS_HIGH() (GPIOA->BSRR = (1u << 4u))

static uint8_t spi_transfer(uint8_t d) {
  while (!(SPI1->SR & SPI_SR_TXE))
    ;
  *((__IO uint8_t *)&SPI1->DR) = d;
  while (!(SPI1->SR & SPI_SR_RXNE))
    ;
  return *((__IO uint8_t *)&SPI1->DR);
}

static void delay_short(void) {
  for (volatile int i = 0; i < 200; i++) __NOP();
}

void spi_init(void) {
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  (void)RCC->AHB2ENR;

  GPIOA->MODER = (GPIOA->MODER & ~(3u << (4u * 2u))) | (1u << (4u * 2u));
  GPIOA->MODER |= (2u << (5u * 2u)) | (2u << (6u * 2u)) | (2u << (7u * 2u));
  GPIOA->AFR[0] |= (5u << (5u * 4u)) | (5u << (6u * 4u)) | (5u << (7u * 4u));
  GPIOA->OSPEEDR |= (3u << (4u * 2u)) | (3u << (5u * 2u)) | (3u << (6u * 2u)) | (3u << (7u * 2u));

  CS_HIGH();

  SPI1->CR1 = 0;
  SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
  SPI1->CR1 |= SPI_CR1_CPOL | SPI_CR1_CPHA;
  SPI1->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1;
  SPI1->CR1 |= SPI_CR1_SPE;
}

void sensor_read_motion(int8_t *dx, int8_t *dy) {
  uint8_t xl, yl;

  CS_LOW();
  spi_transfer(0x02);
  delay_short();
  xl = spi_transfer(0);
  spi_transfer(0);
  yl = spi_transfer(0);
  spi_transfer(0);
  CS_HIGH();

  *dx = (int8_t)xl;
  *dy = (int8_t)yl;
}