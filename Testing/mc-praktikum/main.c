#include "stm32f4xx.h"
#include "_mcpr_stm32f407.h"
#include <inttypes.h>
#include "display.h"

void LCD_Output16BitWord(uint16_t data);
void TIM12_Init(void);
void TIM8_BRK_TIM12_IRQHandler(void);
void LCD_Init(void);

void LCD_Init() {
	
	RCC->AHB1ENR |= (1 << 4) | (1 << 3) | 1; // 3 for Port D (Display), 4 for Port E
	GPIOD->MODER |= (1 << 13); // Pin 13 Display
}

void TIM12_Init() {
	RCC->AHB1ENR |= 1 << 1; // enable GPIOB
	GPIOB->MODER |= 2u << 14 * 2; // Pin 14 to 10 - alternate function
	GPIOB->AFR[1] |= 9 << 23; // AFRH to 9
	
	RCC->APB1ENR |= (1 << 6); // Enable Clock for TIM12
	TIM12->CR1 |= 1;					// CEN 1
	
	TIM12->CCMR1 &= ~(0xFFu); // Filter IC1F to 0
	TIM12->CCMR1 |= 1;				// Input, TIy-ICy
	TIM12->CCER |= 1;					// CC1E enable
	TIM12->SMCR |= 5;					// T1FP1 as trigger source (0b101)
}

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