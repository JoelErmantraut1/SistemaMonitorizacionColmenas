#ifndef CE_EXTI_CONTROL_H
#define CE_EXTI_CONTROL_H

#include "stm32f4xx_rcc.h"
#include <misc.h>
#include "CE_GPIO_control.h"

// Librerias

typedef struct {
	uint8_t  int_port;
	uint8_t  int_source;
	IRQn_Type int_channel;
	uint32_t int_line;
	EXTITrigger_TypeDef int_trigger;
} EXTI_entrada;
// Tiene el prefijo 'int' aunque es obvio para diferenciar los metodos
// de EXTI de los de GPIO

// Estructuras

void CE_EXTI_config(Entrada entrada, EXTI_entrada int_entrada);
void CE_EXTI_change_trigger(EXTI_entrada *int_entrada);
// Prototipos relacionados a la linea de interrupcion
void TIM2_Init(void);
void TIM2_Config(void);
void CE_EXTI_TIM_Start(void);
void TIM2_IRQHandler(void);
uint8_t CE_EXTI_TIM_ready(void);
void set_TIM_delay(uint32_t tiempo);
void CE_delay_EXTI_TIM(uint32_t tiempo);
// Prototipos relacionados al timer que hace de antirebote

// Prototipos

#endif
