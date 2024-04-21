#include "stm32f4xx.h"
#include "_mcpr_stm32f407.h"
#include <inttypes.h>
#include <stdio.h>
#include "display.h"

void LCD_Output16BitWord(uint16_t data);
void TIM4_Init(void);
void TIM4_IRQHandler(void);

volatile uint16_t tenSecCnt = 0;

void LCD_Output16BitWord(uint16_t data)
{
    GPIOD->ODR &= 0x3FFF; // Set Pins 14 & 15 to 0
    GPIOD->ODR |= (data & 0x0003u) << 14; // Set Pins 14 & 15 to 1

    GPIOD->ODR &= 0xFFFC; //  Set Pins 0 & 1 to 0
    GPIOD->ODR |= (data & 0x000C) >> 2; // -- to 1

    GPIOD->ODR &= 0xF8FF; // Pins 8-10 to 0
    GPIOD->ODR |= (data & 0xE000) >> 5;

    GPIOE->ODR &= 0x007F;
    GPIOE->ODR |= (data & 0x1FF0u) << 3;

    return;
}

void TIM4_IRQHandler() {
	TIM4->SR = 0x0000;
	tenSecCnt++;
}

void TIM4_Init() {
	RCC->APB1ENR |= (1 << 2); // Enable TIM4
	RCC->AHB1ENR |= (1 << 3); // Enable GPIOD
	
	GPIOD->MODER &= ~(3u << 13 * 2); // clear mode register for GPIOD P13
	GPIOD->MODER |= (2 << 13 * 2); // set GPIOD P13 to alternate function mode (10)
	GPIOD->AFR[1] |= (2 << 4 * 5); // set AF2 for AFRH13, link to TIM4_CH2
	
	TIM4->CR1 &= ~(3u << 5); // set CMS to 00 so DIR can be set
	TIM4->CR1 &= ~(1u << 4); // DIR to 0, counting upwards
	TIM4->CR1 |= 1; // enable internal clock for tim4
	
	TIM4->CCER &= ~(1u << 4); // temp. set CC2E to 0, so CC2S can be written
	TIM4->CCMR1 |= ~(3u << 8); // set CC2S to 00, compare mode
	TIM4->CCER |= (1 << 4); // set CC2E to 1, enable compare mode
	
	TIM4->PSC = 84-1;
	TIM4->ARR = 1000-1;
	TIM4->DIER |= 1;
	
	NVIC_SetPriority(TIM4_IRQn, 10);
	NVIC_EnableIRQ(TIM4_IRQn);
}

int main () {
	mcpr_SetSystemCoreClock();
<<<<<<< Updated upstream:Testing/mc-praktikum/main_6.c
	TIM12_Init();
=======
>>>>>>> Stashed changes:Testing/mc-praktikum/main.c
	LCD_Init();
	TIM4_Init();
}