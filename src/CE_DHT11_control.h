#ifndef CE_DHT11_CONTROL_H
#define CE_DHT11_CONTROL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"

// Librerias

#define STATE_DHT_CHECKSUM_GOOD 1
#define STATE_DHT_CHECKSUM_BAD 2

#define SENSOR_INT 1
#define SENSOR_EXT 2

// Defines

typedef struct
{
	char temp_string[20];
	char hum_string[20];
	uint8_t estado;
	uint8_t check;
	int num_identificacion;
	GPIO_TypeDef* puerto;
	uint16_t pin;
	uint8_t periferico;
	int temp_entero;
	int temp_decimal;
	int humedad;
}DHT_Sensor;

// Estructuras

void TIM5_Init(void);
void CE_DHT11_TIM5_Start(void);
void TIM5_Config(void);
void TIM5_IRQHandler(void);
void CE_TIM5_delay(uint32_t tiempo);          // delay en micro segundos

void DHT11_start(DHT_Sensor sensor);
void check_response(DHT_Sensor sensor);
void CE_leer_dht(DHT_Sensor *sensor);
uint8_t read_data(DHT_Sensor sensor);

// Prototipos

#endif
