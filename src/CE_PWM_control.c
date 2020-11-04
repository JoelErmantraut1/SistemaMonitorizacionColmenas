#include "CE_PWM_control.h"

TIM_OCInitTypeDef TIM_OCStruct;

void CE_PWM_timer_init(PWM pwm) {
    TIM_TimeBaseInitTypeDef TIM_BaseStruct;

    /* Enable clock for TIM4 */
    RCC_APB1PeriphClockCmd(pwm.timer_periferico, ENABLE);

    TIM_BaseStruct.TIM_Prescaler = 0;
    /* Count up */
    TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_BaseStruct.TIM_Period = pwm.timer_period; /* 10kHz PWM */
    TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_BaseStruct.TIM_RepetitionCounter = 0;
    /* Initialize TIM4 */
    TIM_TimeBaseInit(pwm.timer, &TIM_BaseStruct);
    /* Start count on TIM4 */
    TIM_Cmd(pwm.timer, ENABLE);
}

void CE_PWM_mode_init(PWM pwm) {
    TIM_OCStruct.TIM_OCMode = pwm.timer_OC_mode;
    TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OCStruct.TIM_Pulse = pwm.timer_period; // Inicialmente, empezara con duty cycle de 100%
    TIM_OC1Init(pwm.timer, &TIM_OCStruct);
    TIM_OC1PreloadConfig(pwm.timer, TIM_OCPreload_Enable);
}

void CE_PWM_GPIO_init(Salida salida_pwm) {
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Clock for GPIOD */
    RCC_AHB1PeriphClockCmd(salida_pwm.periferico, ENABLE);

    /* Alternating functions for pins */
    GPIO_PinAFConfig(salida_pwm.port, salida_pwm.pin_source, GPIO_AF_TIM4);

    /* Set pins */
    GPIO_InitStruct.GPIO_Pin = salida_pwm.pin;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void CE_PWM_init(Salida salida_pwm, PWM pwm) {
    CE_PWM_GPIO_init(salida_pwm);

    CE_PWM_timer_init(pwm);

    CE_PWM_mode_init(pwm);
}

void CE_PMW_change_duty(PWM pwm, uint8_t duty) {
	/*
	 * 'duty' es un valor entre 0 y 100 que representa el
	 * valor porcentual del duty cycle
	 */

	uint16_t CCR1_Val;

	if (duty > 100) duty = 100;
	// La salida del PWM es una continua de Vref si DC > 100%.
	// De cualquier manera se ajusta con este condicional, para evitar conflictos

	CCR1_Val = (uint16_t) ((duty * pwm.timer_period) / 100);
	// Calculo el valor del CCR1_Val para el duty solicitado

	TIM_Cmd(pwm.timer, DISABLE); // Desactivo el timer para modificarlo

	TIM_OCStruct.TIM_Pulse = CCR1_Val;

	TIM_OC1Init(pwm.timer, &TIM_OCStruct);
	// Actualizo el valor del duty cycle

	// TIM4 enable counter
	TIM_Cmd(pwm.timer, ENABLE);
	// Ya lo modifique, lo vuelvo a activar
}
