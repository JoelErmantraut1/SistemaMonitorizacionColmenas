#include "CE_BT_control.h"

void CE_init_BT(BT bt) {
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(bt.USART_periferico, ENABLE);
	RCC_AHB1PeriphClockCmd(bt.port_periferico, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitStructure.GPIO_Pin = bt.tx_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(bt.port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = bt.rx_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(bt.port, &GPIO_InitStructure);

	GPIO_PinAFConfig(bt.port, bt.tx_port_source, bt.alt_fun);
	GPIO_PinAFConfig(bt.port, bt.rx_port_source, bt.alt_fun);

	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(bt.USART, &USART_InitStructure);

	USART_Cmd(bt.USART, ENABLE); // enable USART2
}

void CE_send_BT(BT bt, char *content) {
	/*
	 * Recibe una cadena e itera en cada caracter, enviandolos
	 * ciclicamente. Esta funcion trabaja por polling.
	 */

	int i;
	for (i = 0; i < strlen(content); i++) {

		char dato = content[i];
		USART2->DR = dato;

		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	}
}

uint8_t CE_read_BT(BT bt, char *buffer) {
	/*
	 * Esta funcion debe ser llamada en el handler de la interrupcion
	 * USART que se defina. Cuando recibe el caracter de retorno de carro
	 * (CR) se termina la transmision de datos, y devuelve 1, indicando
	 * que ya se puede utilizar la informacion.
	 */

	if (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET) {
		*buffer = USART_ReceiveData(USART2);

		return 1;
	}

	return 0;
}
