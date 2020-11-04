#ifndef CE_PWM_CONTROL_H
#define CE_PWM_CONTROL_H

#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "CE_GPIO_control.h"

// Librerias

typedef struct {
	uint32_t timer_periferico; // RCC_APB1Periph_TIM4
	uint32_t timer_period; // 8399;
	TIM_TypeDef *timer; // TIM4
	uint16_t timer_OC_mode; // TIM_OCMode_PWM2
} PWM;

// Estructuras

void CE_PWM_GPIO_init(Salida salida_pwm);
void CE_PWM_timer_init(PWM pwm);
void CE_PWM_mode_init(PWM pwm);
void CE_PWM_init(Salida salida_pwm, PWM pwm);
void CE_PMW_change_duty(PWM pwm, uint8_t duty);

// Prototipos

#endif
