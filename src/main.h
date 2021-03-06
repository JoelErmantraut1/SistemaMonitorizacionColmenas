#ifndef MAIN_H
#define MAIN_H

// ---------------------------------------------------------------------
//                           IMPORTANTE
// LOS PINES DEL DISPLAY ESTAN DEFINIDOS CON "DEFINES" EN LA LIBRERIA .C"
// ---------------------------------------------------------------------
#define T_REFRESH_DISPLAY		250
#define	T_PULSADORES			100
#define T_SENSORES				2500
#define BT_CHECK_TIME			20
#define BATTERY_CHECK			5000
// Cantidad de us entre las diferentes funciones dentro del SysTick
#define MAX_CHAR_LCD			17
// Cantidad maxima de caracteres para cadenas
#define MAX_ADC_VALUE			4095.0f // Valor para el 100% de bateria
#define MIN_ADC_VALUE			0.0f	 // Valor para el 0% de bateria
// Indentificadores para la carga de la bateria
#define MAX_ADC_VOLTS			2.9f
// Maxima tension que mide el ADC
#define	INFRA_DELAY				250
// Limite para el antirrebote de los sensores de proximidad en us
#define SYSTICK_CONSTANT		1000
// Constante que establece cada cuanto se interrumpe el Systick
#define MAX_CONF_LEN			3
// Maximo tama�o de la cadena que almacena la configuracion
#define BT_INDEX				0
#define FREC_SAMPLE_INDEX		1
#define BRIGHTNESS_INDEX		2
// Define las posicion de cada funcion en el archivo de configuracion
#define FREC_SAMPLE_15			0
#define FREC_SAMPLE_60			1
// Defines que definen a que frecuencia corresponde cada digito

/* Librerias */
#include <math.h>
#include <stdio.h>
// Librerias estandar de C
#include "stm32_ub_lcd_2x16.h"
#include "tm_stm32f4_rtc.h"
// Librerias de terceros
#include "CE_GPIO_control.h"
#include "CE_generic_functions.h"
#include "CE_SD_card_control.h"
#include "CE_BT_control.h"
#include "CE_ADC_control.h"
#include "CE_PWM_control.h"
#include "CE_EXTI_control.h"
#include "CE_DHT11_control.h"
// Librerias Coletto - Ermantraut

/* Private macro */
/* Private variables */
/* Private function prototypes */

void controlador_systick(void);
void PORT_init(void);
void refrescoDisplay(void);
void select_menu(char *fila1, char *fila2);
void select_menu_config(char *fila1, char *fila2);
// Relacionado al menu
void EXTI4_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
// Para los sensores infrarrojos
void BT_sender();
// Funciones BT
void LEDs_indicadores(uint32_t carga);
// Funciones para medir e indicar carga bateria
void CE_Print_StartScreen(void);
// Funcion para imprimir caracteres personalizados en el LCD
int cargar_configuracion(void);
// Carga la configuracion al inicio del programa
void cambiar_configuracion(uint8_t item, uint8_t value);
// Funcion que cambia la configuracion cada vez que hay algun cambio
void ajustar_brillo(char brillo);
// Funcion que ajusta el brillo del LCD

float medir_temp_ext();
float medir_temp_int();
float medir_hum_ext();
float medir_hum_int();
float carga_bateria_porcentaje();
float carga_bateria_tension();
float ver_ingresos();
float ver_egresos();
float calcular_diferencia();
float calcular_dif_prom();
float crono_dia();
float crono_hora();
// Funciones de las mediciones
void activar_bluetooth (void);
void desactivar_bluetooth (void);
void muestrear_hora (void);
void muestrear_min (void);
void brilloBajo (void);
void brilloMuyBajo (void);
void brilloAlto (void);
void brilloMuyAlto (void);
// Funciones de la configuracion

#endif
