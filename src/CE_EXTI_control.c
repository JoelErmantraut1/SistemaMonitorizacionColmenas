#include "CE_EXTI_control.h"

// Librerias

EXTI_InitTypeDef EXTI_InitStructure;
uint32_t TimingDelay_TIM2_INT = 0;

// Variables

void CE_EXTI_config(Entrada entrada, EXTI_entrada int_entrada) {

	NVIC_InitTypeDef   NVIC_InitStructure;

	/* Enable GPIOC clock */
	RCC_AHB1PeriphClockCmd(entrada.periferico, ENABLE);
	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	SYSCFG_EXTILineConfig(int_entrada.int_port, int_entrada.int_source);

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

void CE_EXTI_change_trigger(EXTI_entrada *int_entrada) {
	if (int_entrada->int_trigger == EXTI_Trigger_Rising) {
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
		int_entrada->int_trigger = EXTI_Trigger_Falling;
	} else {
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
		int_entrada->int_trigger = EXTI_Trigger_Rising;
	}

	EXTI_InitStructure.EXTI_Line = int_entrada->int_line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	// Configuro los demas parametros de la estructura
}

// Funciones relacionadas a la linea de interrupciones

void TIM2_Init(void)
{
	/*
	 *	TIMX_Init Configura los parametros del timer X.
	 *	Formulas:
	 *	TIM3CLK = 84MHz. Reloj del timer, para modiicarlo hay que cambiar el archivo system_stm32f4xx.c .
	 *	frecuencia. Frecuencia a la que cuenta el timer, este valor modifica el prescaler.
	 *	TIM_Period = (Tei*TIM3CLK)/(Prescalervalue+1). Tei es el valor de periodo de interrupcion deseado en segundos.
	 *
	 *	ATENCION: TIM_Period y PrescalerValue no deben superar el valor de 0xFFFF (65536) ya que son de 16bits.
	 */

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	uint16_t PrescalerValue = 0; // Variable para el prescaler.
	uint32_t frecuencia = 2e3; // Frecuencia del contador a 10kHz. Tener cuidado de no cometer overflow en la variable PrescalerValue.
	PrescalerValue = (uint16_t) ((SystemCoreClock /2) / frecuencia) - 1; //Conversion de frecuencia al valor de Prescaler.
	TIM_TimeBaseStructure.TIM_Period = 10; // 321.5uS (10e3 = 1 seg --> 3.215 = 321.5uS) de periodo.
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE); // habilitacion de las interrupciones por el timer 5.
	TIM_Cmd(TIM2, ENABLE); // Habilita el contador para el timer 5.

}

void TIM2_Config(void)
{
	/*
	 *	TIMX_Config habilita el reloj y las interrupciones globales para el timer X
	 */

	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // Se habilita el reloj.

	/* Se habilitan las interrupciones globales para el timer X*/
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

void CE_EXTI_TIM_Start(void)
{
	/*
	 *	TIMX_Start deja el timer X ready to go.
	 */

	TIM2_Config(); // Configuracion del timer.
	TIM2_Init(); // Inicializacion del timer.
	TIM_Cmd(TIM2, DISABLE);
}

uint8_t CE_EXTI_TIM_ready(void) {
	if (!TimingDelay_TIM2_INT) return 1;
	else return 0;
}

void set_TIM_delay(uint32_t tiempo) {
	TimingDelay_TIM2_INT = tiempo;
	// Tiempo en us

	TIM_Cmd(TIM2, ENABLE);
}

void CE_delay_EXTI_TIM(uint32_t tiempo)
{
	/* Funcion delay()
	 * Realiza un retardo en funcion de la frecuencia de reloj del microcontrolador.
	 * Recibe como parametro el retraso, para este caso, en micro segundos debido a la configuracion del Systick.
	 */

	TIM_Cmd(TIM2, ENABLE);
	TimingDelay_TIM2_INT = tiempo;
	while(TimingDelay_TIM2_INT != 0);
	TIM_Cmd(TIM2, DISABLE);

}
/*
void TIM2_IRQHandler(void)
{

	if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC1); // Se limpia la bandera de interrupcion.

		if (TimingDelay_TIM2 > 0)
		{
			TimingDelay_TIM2--;
		} else TIM_Cmd(TIM2, DISABLE);

	}
}
*/


// Funciones relacionadas al timer que se utiliza para el antirebote
