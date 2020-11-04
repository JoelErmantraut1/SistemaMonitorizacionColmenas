#include "CE_EXTI_control.h"

// Librerias

EXTI_InitTypeDef EXTI_InitStructure;

// Variables

void CE_EXTI_config(Entrada entrada, EXTI_entrada int_entrada) {

	NVIC_InitTypeDef   NVIC_InitStructure;

	/* Enable GPIOC clock */
	RCC_AHB1PeriphClockCmd(entrada.periferico, ENABLE);
	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Connect EXTI Line6 to PC6 pin */
	SYSCFG_EXTILineConfig(int_entrada.int_port, int_entrada.int_source);

	/* Configure EXTI Line6 */
	EXTI_InitStructure.EXTI_Line = int_entrada.int_line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = int_entrada.int_trigger;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI Line6 Interrupt to the lowest priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = int_entrada.int_channel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void CE_EXTI_change_trigger(EXTI_entrada int_entrada) {
	if (int_entrada.int_trigger == EXTI_Trigger_Rising) {
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
		int_entrada.int_trigger = EXTI_Trigger_Falling;
	} else {
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
		int_entrada.int_trigger = EXTI_Trigger_Rising;
	}

	EXTI_InitStructure.EXTI_Line = int_entrada.int_line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	// Configuro los demas parametros de la estructura
}
