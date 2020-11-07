/*
 * Algoritmo principal del proyecto de cuarto año de Ingenieria Electronica
 * que consiste en un sistema de monitorizacion de colmenas.
 */

// ---------------------------------------------------------------------
//                           IMPORTANTE
// LOS PINES DEL DISPLAY ESTAN DEFINIDOS CON "DEFINES" EN LA LIBRERIA .C"
// ---------------------------------------------------------------------
#define T_REFRESH_DISPLAY		250
#define	T_PULSADORES			100
// Cantidad de ms entre refrescos del display
#define SENSOR_INT				1
#define SENSOR_EXT				2

/* Librerias */
#include "stm32_ub_lcd_2x16.h"
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
void EXTI3_IRQHandler(void);

/* FNS REPRESENTATIVAS */
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
void muestrear_Min (void);
void brilloBajo (void);
void brilloMuyBajo (void);
void brilloAlto (void);
void brilloMuyAlto (void);
// Funciones de la configuracion
/* FNS REPRESENTATIVAS */

/* Private functions */

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/

Entrada puls_line_entrada_1 = {RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_6};
Entrada puls_line_entrada_2 = {RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_7};
Entrada puls_line_entrada_3 = {RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_8};
Entrada puls_line_entrada_4 = {RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_9};

EXTI_entrada int_infrarrojo1 = {
		EXTI_PortSourceGPIOB,
		EXTI_PinSource4,
		EXTI4_IRQn,
		EXTI_Line4,
		EXTI_Trigger_Rising
};
// Asignados de valores a las estructuras a los pulsadores
uint8_t global_puls, last_global_puls, pantalla = 0, level = 0, menu = 0, pantalla_int = 4;
// Variable que almacena el indicador del ultimo pulsador presionado
const char *mediciones[][3] = {
    {"Exterior", "Temp:", "Hum:"},
    {"Interior", "Temp:", "Hum:"},
    {"Carga de bat.", "Porce:", "Tension:"},
    {"Ing. y Egr.", "Ingresos:", "Egresos:"},
    {"Diferencia", "Diferencia:", "Promedio:"},
    {"Cronometro", "Dia:", "Hora:"}
};

float (*fn[][2])(void) = {
    {medir_temp_ext, medir_temp_int},
    {medir_hum_ext, medir_hum_int},
    {carga_bateria_porcentaje, carga_bateria_tension},
    {ver_ingresos, ver_egresos},
    {calcular_diferencia, calcular_dif_prom},
    {crono_dia, crono_hora}
};
// Arreglos de dos funciones
// Almacena las funciones de cada "pestaña" del menu

int main(void)
{
	ADC_PIN adc = {
			GPIOC,
			GPIO_Pin_2,
			RCC_AHB1Periph_GPIOC,
			RCC_APB2Periph_ADC2,
			ADC2,
			ADC_InjectedChannel_1,
			ADC_Channel_12
	};

	PWM pwm = {
			RCC_APB1Periph_TIM4,
			8399,
			TIM4,
			TIM_OCMode_PWM2
	};

	Salida salida_pwm = {
			RCC_AHB1Periph_GPIOD,
			GPIOD,
			GPIO_Pin_12,
			GPIO_PinSource12
	};

    SD_Card card = {
    		"/",
			"/temp.txt",
			"/hum.txt",
			"/health.txt"
    };

    Entrada infrarrojo1 = {
    		RCC_AHB1Periph_GPIOB,
    		GPIOB,
    		GPIO_Pin_4
    };

    BT bt = {
		GPIOD,
		GPIO_Pin_14,
		RCC_AHB1Periph_GPIOD,
		GPIOA,
		GPIO_Pin_2,
		GPIO_Pin_3,
		USART2,
		GPIO_AF_USART2,
		RCC_APB1Periph_USART2,
		RCC_AHB1Periph_GPIOA,
		GPIO_PinSource2,
		GPIO_PinSource3
    };

    CE_init_BT(bt);

    DHT_Sensor sensor_int;
    DHT_Sensor sensor_ext;

	CE_DHT11_TIM5_Start(); // Inicializa el timer del DHT.

	sensor_int.num_identificacion = SENSOR_INT;
	sensor_int.puerto = GPIOE;
	sensor_int.pin = GPIO_Pin_10;
	sensor_int.periferico = RCC_AHB1Periph_GPIOE;
	sensor_int.temp_entero = 0;
	sensor_int.temp_decimal = 0;
	sensor_int.humedad = 0;

	sensor_ext.num_identificacion = SENSOR_EXT;
	sensor_ext.puerto = GPIOE;
	sensor_ext.pin = GPIO_Pin_11;
	sensor_ext.periferico = RCC_AHB1Periph_GPIOE;
	sensor_int.temp_entero = 0;
	sensor_int.temp_decimal = 0;
	sensor_int.humedad = 0;

    CE_conf_in(infrarrojo1, GPIO_PuPd_NOPULL);
    CE_EXTI_config(infrarrojo1, int_infrarrojo1);

	SystemInit();

    CE_ADC_init(adc);
    CE_PWM_init(salida_pwm, pwm);
	UB_LCD_2x16_Init();
	PORT_init();

	UB_LCD_2x16_String(0, 0, "Holaaa");

	SysTick_Config(SystemCoreClock / 1000); // 1ms

	while (1) {
		char buffer[20];
		uint8_t response = CE_read_SD(card, "/exit.txt", buffer);

		CE_leer_dht(&sensor_int);
		UB_LCD_2x16_String(0,0, sensor_int.temp_string);
		UB_LCD_2x16_String(0,1, sensor_int.hum_string); // Texto en la linea 1

		CE_leer_dht(&sensor_ext);
		UB_LCD_2x16_String(0,0, sensor_ext.temp_string);
		UB_LCD_2x16_String(0,1, sensor_ext.hum_string); // Texto en la linea 1

		CE_send_BT(bt, "r");
		CE_read_BT(bt, buffer);

		UB_LCD_2x16_String(0, 0, buffer);
	}
}

/* --------------------------------------------------------------------- */
/* --------------------- FUNCIONES REPRESENTATIVAS --------------------- */
/* -------- ESTAS NO SON LAS FNS QUE VAMOS A USAR EN EL PROYECTO ------- */
/* --------------------------------------------------------------------- */

float medir_temp_ext() { return 10.3; }
float medir_temp_int() { return 9.2; }
float medir_hum_ext() { return 50.9; }
float medir_hum_int() { return 49.13; }
float carga_bateria_porcentaje() { return 32.90; }
float carga_bateria_tension() { return 38.90; }
float ver_ingresos() { return 100.9; }
float ver_egresos() { return 120.9; }
float calcular_diferencia() { return 0; }
float calcular_dif_prom() { return 99.0; }
float crono_dia() { return 23.17; }
float crono_hora() { return 43.34; }
// Funciones de las mediciones
void activar_bluetooth (void) { ; }
void desactivar_bluetooth (void) { ; }
void muestrear_hora (void)  {; }
void muestrear_Min (void) { ; }
void brilloBajo (void) { ; }
void brilloMuyBajo (void) { ; }
void brilloAlto (void) { ; }
void brilloMuyAlto (void) { ; }
// Funciones de la configuracion

/* --------------------------------------------------------------------- */
/* --------------------- FUNCIONES REPRESENTATIVAS --------------------- */
/* -------- ESTAS NO SON LAS FNS QUE VAMOS A USAR EN EL PROYECTO ------- */
/* --------------------------------------------------------------------- */

void PORT_init(void) {
	CE_conf_in(puls_line_entrada_1, GPIO_PuPd_DOWN);
	CE_conf_in(puls_line_entrada_2, GPIO_PuPd_DOWN);
	CE_conf_in(puls_line_entrada_3, GPIO_PuPd_DOWN);
	CE_conf_in(puls_line_entrada_4, GPIO_PuPd_DOWN);
	// Los pines de entradas, que despues se configuran con interrupciones
}

void controlador_systick(void) { // Esta funcion es llamada en la interrupcion del SysTick
	// Configurado para interrumpir cada 1ms
	static int contSystick = 0;

	++contSystick;

	if (contSystick % T_REFRESH_DISPLAY == 0)
	{
		refrescoDisplay();
		contSystick=0;
	} else if (contSystick % T_PULSADORES == 0) {
		global_puls = CE_pulsador_presionado(
				puls_line_entrada_1,
				puls_line_entrada_2,
				puls_line_entrada_3,
				puls_line_entrada_4
		);
	}
}

void refrescoDisplay(void) {
	char fila1[17] = "", fila2[17] = ""; // 16 caracteres + salto de linea
	// Tienen que estar inicializados porque sino generan caracteres
	// raros entre pantallas

	if (menu == 1) select_menu(fila1, fila2); // Configuraciones
	else if (menu == 2) select_menu_config(fila1, fila2); // Mediciones
	else {
		if (global_puls == 1) menu = 1;
		else if (global_puls == 2) menu = 2;
		else {
			siprintf(fila1, "%s", "Esperando...");
			siprintf(fila2, "%s", "");
		}
		global_puls = NO_BUTTON;
	}

	UB_LCD_2x16_Clear();
	UB_LCD_2x16_String(0, 0, fila1);
	UB_LCD_2x16_String(0, 1, fila2);
}

void select_menu(char *fila1, char *fila2) {

	static uint8_t last_global_puls = NO_BUTTON;

	if (level == 0) {
		if (global_puls == BUTTON_4) menu = 0;
		else if (global_puls != NO_BUTTON) level = 1;
		else {
			siprintf(fila1, "%s", "Mediciones");
			siprintf(fila2, "%s", "");
		}
	} else if (level == 1) {
		if (global_puls == BUTTON_1 || global_puls == BUTTON_2) {
			last_global_puls = global_puls;
			level = 2;
		} else if (global_puls == BUTTON_3) {
	        if (pantalla == 4) pantalla = 0; // 4 + 2 = 6, mas de lo que necesita
	        else pantalla = pantalla + 2; 	 // Suma de a dos por comodidad
		} else if (global_puls == BUTTON_4) {
			level = 0;
		} else {
		    // Pantalla = 0 -> Opciones 0 y 1
		    // Pantalla = 2 -> Opciones 2 y 3
		    // Pantalla = 4 -> Opciones 4 y 5
			siprintf(fila1, "%d. %s", (pantalla) % 2 + 1, mediciones[pantalla][0]);
			siprintf(fila2, "%d. %s", (pantalla + 1) % 2 + 1, mediciones[pantalla + 1][0]);
		}
	} else {
		if (global_puls == BUTTON_4) {
			level = 1;
		} else {
	        CE_print(fila1, mediciones[pantalla + last_global_puls - 1][1], fn[pantalla + last_global_puls - 1][0]());
	        CE_print(fila2, mediciones[pantalla + last_global_puls - 1][2], fn[pantalla + last_global_puls - 1][1]());
		}
	}

	global_puls = NO_BUTTON;
}

void select_menu_config(char *fila1, char *fila2) {

	static uint8_t last_global_puls = NO_BUTTON;

	if (level == 0)
	{
		if (global_puls == BUTTON_4) menu = 0;
		else if (global_puls != NO_BUTTON) level = 1;
		else
		{
			siprintf(fila1, "%s", "Configuraciones");
			siprintf(fila2, "%s", "");
		}
	}
	else if (level == 1)
	{
		if (global_puls == BUTTON_1 || global_puls == BUTTON_2)
		{
			last_global_puls = global_puls;
			level = 2;
		}
		else if (global_puls == BUTTON_3)
		{
	        if (pantalla == 2)
	        	pantalla = 0; // 4 + 2 = 6, mas de lo que necesita
	        else
	        	pantalla = pantalla + 2; 	 // Suma de a dos por comodidad
		}
		else if (global_puls == BUTTON_4)
		{
			level = 0;
		}
		else
		{
		    // Pantalla = 0 -> Opciones 0 y 1
		    // Pantalla = 2 -> Opciones 2 y 3

			if (pantalla == 0)
			{
				siprintf(fila1, "%s", "1. Bluetooth");
				siprintf(fila2, "%s", "2. Muestreo");
			}
			if (pantalla == 2)
			{
				siprintf(fila1, "%s", "1. Brillo Bajo");
				siprintf(fila2, "%s", "2. Brillo Alto");
			}
		}
	}
	else if (level == 2)
	{
		if (global_puls == BUTTON_4)
		{
			level = 1;
		}
		else if(global_puls == BUTTON_1 || global_puls == BUTTON_2)
		{
			last_global_puls = global_puls;
			level = 3;
		}
		else
		{
			if (pantalla == 0 && last_global_puls == BUTTON_1)
			{
				pantalla_int = 0;                                                     // Esta variable solo funciona como una auxiliar dentro del nivel 3
																					  // que se configura aca y se utiliza en dicho nivel para ayudar a
				siprintf(fila1, "%s", "1. Activar Bt");								  // diferenciar que boton se presiono y que funcion se debe llamar
				siprintf(fila2, "%s", "2. Desactivar Bt");
			}
			else if (pantalla == 0 && last_global_puls == BUTTON_2)
			{
				pantalla_int = 1;

				siprintf(fila1, "%s", "1. Una Hora (24)");
				siprintf(fila2, "%s", "2. 15 Min. (96)");
			}
			else if (pantalla == 2 && last_global_puls == BUTTON_1)
			{
				pantalla_int = 2;

				siprintf(fila1, "%s", "1. Muy Bajo");
				siprintf(fila2, "%s", "2. Bajo");
			}
			else if (pantalla == 2 && last_global_puls == BUTTON_2)
			{
				pantalla_int = 3;

				siprintf(fila1, "%s", "1. Alto");
				siprintf(fila2, "%s", "2. Muy Alto");
			}

		}
	}
	else
	{
		if (pantalla_int == 0 && last_global_puls == BUTTON_1)
		{
			activar_bluetooth();
			level = 2;
			pantalla_int = 4;

			// Aca se puede agregar un mensaje y un delay de un par de segundos para que el usuario vea el resultado de su opcion
			// Se podria agregar un chequeo tambien para ver si el bluetooth se activo o no mediante una variable que devuelva la
			// funcion de bluetooth si el proceso fue exitoso y en base a eso el mensaje que salga.

			// UB_LCD_2x16_Clear();
			// siprintf (fila1, "%s", "Bt Activado");
			// siprintf (fila2, "%s", "");
			// delay (2 seg);
		}
		if (pantalla_int == 0 && last_global_puls == BUTTON_2)
		{
			desactivar_bluetooth();
			level = 2;
			pantalla_int = 4;
			last_global_puls = BUTTON_1;

			// Exactamente lo mismo que para la opcion de activar bluetooth aca.
		}
		if (pantalla_int == 1 && last_global_puls == BUTTON_1)
		{
			muestrear_hora();
			level = 2;
			pantalla_int = 4;
			last_global_puls = BUTTON_2;

			// Aca se puede agregar un mensaje y un delay de un par de segundos para que el usuario vea el resultado de su opcion
			// Este mensaje saldria siempre

			// UB_LCD_2x16_Clear();
			// siprintf (fila1, "Set c/u hora");
			// delay (2 seg);
		}
		if (pantalla_int == 1 && last_global_puls == BUTTON_2)
		{
			muestrear_Min();
			level = 2;
			pantalla_int = 4;


			//misma idea que para muestrear_ hora
		}
		if (pantalla_int == 2 && last_global_puls == BUTTON_1)
		{
			brilloMuyBajo();
			level = 2;
			pantalla_int = 4;

		}
		if (pantalla_int == 2 && last_global_puls == BUTTON_2)
		{
			brilloBajo();
			level = 2;
			pantalla_int = 4;
			last_global_puls = BUTTON_1;

		}
		if (pantalla_int == 3 && last_global_puls == BUTTON_1)
		{
			brilloAlto();
			level = 2;
			pantalla_int = 4;
			last_global_puls = BUTTON_2;

		}
		if (pantalla_int == 3 && last_global_puls == BUTTON_2)
		{
			brilloMuyAlto();
			level = 2;
			pantalla_int = 4;

		}
	}

	global_puls = NO_BUTTON;
}

void EXTI4_IRQHandler(void)
{
	if(EXTI_GetITStatus(int_infrarrojo1.int_line) != RESET)
	{
		static int state = 0;
		state = !state;

		UB_LCD_2x16_Clear();
		if (state) {
			UB_LCD_2x16_String(0, 0, "Tapado 1");
		} else {
			UB_LCD_2x16_String(0, 0, "NO Tapado 1");
		}

		CE_EXTI_change_trigger(int_infrarrojo1);

		EXTI_ClearITPendingBit(int_infrarrojo1.int_line);
	}
}
