#ifndef CE_SD_CARD_CONTROL_H
#define CE_SD_CARD_CONTROL_H

#include "defines.h"
#include "stm32f4xx.h"
#include "tm_stm32f4_delay.h"
#include "tm_stm32f4_fatfs.h"
#include <stdio.h>
#include <string.h>

// Librerias

#define MAX_NUMBER_CHARS	10
// Este identificador se utiliza en la funcion "CE_read_SD"

// Defines

typedef struct {
	char *path;
	char *temp_ext_filename; // Nombre archivo que guarda las temperaturas externas
	char *temp_int_filename; // Nombre archivo que guarda las temperaturas internas
	char *hum_ext_filename; // Nombre archivo que guarda las humedades externas
	char *hum_int_filename; // Nombre archivo que guarda las humedades internas
	char *health_filename; // Nombre archivo que guarda la cantidad de abejas
	char *config_filename; // Nombre archivo que almacena las configuraciones
	char *diferencia_filename; // Nombre archivo que almacena las diferencias diarias
} SD_Card;

// Estructuras

uint8_t CE_write_SD(SD_Card card, char *filename, char *text, uint8_t ow);
uint8_t CE_read_SD(SD_Card card, char *filename, char *buffer);

// Prototipos

#endif
