/*
 * Que te parece que es
 */

#ifndef CE_BT_CONTROL_H
#define CE_BT_CONTROL_H

#include <stm32f4xx.h>
#include <stm32f4xx_usart.h>
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h"
#include <stdio.h>
#include <string.h>

// Librerias

typedef struct {
	GPIO_TypeDef* active_port; // Puerto del pin conectado a VCC de modulo
	uint16_t active_pin; // Pin conectado a VCC del modulo
	GPIO_TypeDef* port; // Puerto de los pines USART
	uint16_t tx_pin;
	uint16_t rx_pin;
	USART_TypeDef * USART;
	uint8_t alt_fun;
	IRQn_Type USART_Channel;
	uint32_t USART_periferico;
	uint32_t port_periferico;
	uint8_t tx_port_source;
	uint8_t rx_port_source;
} BT;

// Estructuras

void CE_init_BT(BT bt);
void CE_send_BT(BT bt, char *content);
uint8_t CE_read_BT(BT bt, char *buffer);

// Prototipos

#endif
