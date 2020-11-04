#include "CE_GPIO_control.h"

uint8_t CE_leer_entrada(Entrada entrada) {
	uint8_t state = GPIO_ReadInputDataBit(entrada.port, entrada.pin);

	return state;
}

void CE_escribir_salida(Salida salida, uint8_t state) {
	if (!state)	GPIO_ResetBits(salida.port, salida.pin);
	else GPIO_SetBits(salida.port, salida.pin);

	// Si state es 0, apaga
	// Cualquier otro valor, prende
}

void CE_conf_in(Entrada entrada, GPIOPuPd_TypeDef PuPd) {
	GPIO_InitTypeDef GPIO_InitStructure; 					//Estructura de configuracion

	RCC_AHB1PeriphClockCmd(entrada.periferico, ENABLE);		//Habilitacion de la senal de
															//reloj para el periferico GPIOD
	//Enmascaramos los pines que usaremos

	GPIO_InitStructure.GPIO_Pin =	  entrada.pin;

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;			//Los pines seleccionados como salida
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;		//Tipo de salida como push pull
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;	//Velocidad del clock para el GPIO
	GPIO_InitStructure.GPIO_PuPd	= PuPd;					//Con pull downs

	GPIO_Init(entrada.port, &GPIO_InitStructure); 				//Se aplica la configuracion definidas anteriormente
}

void CE_conf_out(Salida salida) {
	GPIO_InitTypeDef GPIO_InitStructure; 					//Estructura de configuracion

	RCC_AHB1PeriphClockCmd(salida.periferico, ENABLE);		//Habilitacion de la senal de
															//reloj para el periferico GPIOD
	//Enmascaramos los pines que usaremos

	GPIO_InitStructure.GPIO_Pin =	salida.pin;

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;					//Los pines seleccionados como salida
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;					//Tipo de salida como push pull
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;				//Velocidad del clock para el GPIO
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;					//Con pull downs

	GPIO_Init(salida.port, &GPIO_InitStructure); //Se aplica la configuracion definidas anteriormente
}

uint8_t CE_pulsador_presionado(
		Salida puls_line_salida_1,
		Salida puls_line_salida_2,
		Entrada puls_line_entrada_1,
		Entrada puls_line_entrada_2
) {
	/*
	 * Esta funcion considera un keypad o matriz de pulsadores de 4 botones
	 */

	uint8_t botonApretado = (uint8_t)NO_BUTTON;

	CE_escribir_salida(puls_line_salida_1, 0);
	CE_escribir_salida(puls_line_salida_2, 1);
	// Alimentamos una sola fila

	if(CE_leer_entrada(puls_line_entrada_1))
		botonApretado = (uint8_t)BUTTON_3;
	else if (CE_leer_entrada(puls_line_entrada_2))
		botonApretado = (uint8_t)BUTTON_4;
	// Y aca testeamos cada columna

	CE_escribir_salida(puls_line_salida_1, 1);
	CE_escribir_salida(puls_line_salida_2, 0);
	// Dejo de alimentar la primer fila y alimentamos la otra fila

	if(CE_leer_entrada(puls_line_entrada_1))
		botonApretado = (uint8_t)BUTTON_1;
	else if (CE_leer_entrada(puls_line_entrada_2))
		botonApretado = (uint8_t)BUTTON_2;
	// Y testeamos las mismas columnas

	CE_escribir_salida(puls_line_salida_1, 1);
	CE_escribir_salida(puls_line_salida_2, 1);
	// Vuelvo a alimentar las dos filas para habilitar la interrupcion

	return botonApretado;
}
