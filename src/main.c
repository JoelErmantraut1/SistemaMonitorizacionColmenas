/*
 * Algoritmo principal del proyecto de cuarto año de Ingenieria Electronica
 * que consiste en un sistema de monitorizacion de colmenas.
 */

#include "main.h"

/**
**===========================================================================
**
**  Estructuras de los componentes
**
**===========================================================================
*/

Salida led_bateria1 = {
		RCC_AHB1Periph_GPIOE,
		GPIOE,
		GPIO_Pin_7,
		GPIO_PinSource7
};
Salida led_bateria2 = {
		RCC_AHB1Periph_GPIOE,
		GPIOE,
		GPIO_Pin_8,
		GPIO_PinSource8
};
Salida led_bateria3 = {
		RCC_AHB1Periph_GPIOE,
		GPIOE,
		GPIO_Pin_9,
		GPIO_PinSource9
};

Salida salida_pwm = {
		RCC_AHB1Periph_GPIOD,
		GPIOD,
		GPIO_Pin_12,
		GPIO_PinSource12
};
PWM pwm = {
		RCC_APB1Periph_TIM4,
		8399,
		TIM4,
		TIM_OCMode_PWM2
};
// PWM para controlar el brillo del LCD

// LEDs que reflejan la carga de la bateria

Entrada puls_line_entrada_1 = {RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_6};
Entrada puls_line_entrada_2 = {RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_7};
Entrada puls_line_entrada_3 = {RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_8};
Entrada puls_line_entrada_4 = {RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_9};

// Pulsadores de entrada

Entrada infrarrojo1 = {
		RCC_AHB1Periph_GPIOB,
		GPIOB,
		GPIO_Pin_4
};

Entrada infrarrojo2 = {
		RCC_AHB1Periph_GPIOB,
		GPIOB,
		GPIO_Pin_5
};

// Infrarrojos para detectar entrada o salida de abejas

EXTI_entrada int_infrarrojo1 = {
		EXTI_PortSourceGPIOB,
		EXTI_PinSource4,
		EXTI4_IRQn,
		EXTI_Line4,
		EXTI_Trigger_Rising
};
EXTI_entrada int_infrarrojo2 = {
		EXTI_PortSourceGPIOB,
		EXTI_PinSource5,
		EXTI9_5_IRQn,
		EXTI_Line5,
		EXTI_Trigger_Rising
};

// Lineas de interrupcion por entradas

DHT_Sensor sensor_int = {
		"",
		"",
		0,
		0,
		SENSOR_INT,
		GPIOE,
		GPIO_Pin_10,
		RCC_AHB1Periph_GPIOE,
		0,
		0,
		0
};
DHT_Sensor sensor_ext = {
		"",
		"",
		0,
		0,
		SENSOR_EXT,
		GPIOE,
		GPIO_Pin_11,
		RCC_AHB1Periph_GPIOE,
		0,
		0,
		0
};

// Sensores de temperatura y humedad, interno y externo

ADC_PIN battery_adc = {
		GPIOC,
		GPIO_Pin_2,
		RCC_AHB1Periph_GPIOC,
		RCC_APB2Periph_ADC2,
		ADC2,
		ADC_InjectedChannel_1,
		ADC_Channel_12
};

// ADC para medir la carga de la bateria

SD_Card card = {
		"/",
		"/temp.txt",
		"/hum.txt",
		"/health.txt",
		"/config.txt"
};

// Tarjeta de memoria SD
// Maneja un archivo por cada dato relevante a medir
// Temperatura, humedad y salud de las abejas

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

// Modulo Bluetooth para comunicacion con dipositivo movil

/**
**===========================================================================
**
**  Variables globales
**
**===========================================================================
*/

uint8_t global_puls, last_global_puls, pantalla = 0, level = 0, menu = 0, pantalla_int = 4;
// Variables globales para el menu
char BT_buffer;
// Variable que almacena el caracter recibido por BT
uint8_t sensors_ready = 0;
// Variable para avisar cuando leer el dato de los sensores
uint8_t direccion = 0;
// Variable que indica si una abeja entra o sale
/*
 * 0: Direccion no indicada
 * 1: Entrando
 * 2: Saliendo
 */
uint32_t ingresos = 0;
uint32_t egresos = 0;
// Contadores de cantidades de abejas que ingresaron y egresaron
uint32_t carga_bateria = 0;
// Variable para ver la carga actual de la bateria
uint16_t inactividad = 0;
// Variable que registra si se ha presionado pulsadores en el ultimo minuto

const char *mediciones[][3] = {
    {"Exterior", "Temp:", "Hum:"},
    {"Interior", "Temp:", "Hum:"},
    {"Carga de bat.", "Porc:", "Tension:"},
    {"Ing. y Egr.", "Ingresos:", "Egresos:"},
    {"Diferencia", "Diferencia:", "Promedio:"},
    {"Cronometro", "Dia:", "Hora:"}
};

float (*fn[][2])(void) = {
    {medir_temp_ext, medir_hum_ext},
    {medir_temp_int, medir_hum_int},
    {carga_bateria_porcentaje, carga_bateria_tension},
    {ver_ingresos, ver_egresos},
    {calcular_diferencia, calcular_dif_prom},
    {crono_dia, crono_hora}
};
// Arreglos de dos funciones por fila
// Almacena las funciones de cada "pestaña" del menu

/**
**===========================================================================
**
**  Programa principal y funciones
**
**===========================================================================
*/

int main(void)
{
	SystemInit();

	PORT_init();

    CE_init_BT(bt);

	CE_DHT11_TIM5_Start(); // Inicializa el timer del DHT
	CE_EXTI_TIM_Start();
	// Timer que se usa como antirebote para las interrupciones
	// de los sensores infrarrojos

    CE_ADC_init(battery_adc);
    CE_PWM_init(salida_pwm, pwm);
	UB_LCD_2x16_Init();

	SysTick_Config(SystemCoreClock / 1000);

	while (1) {
		if (sensors_ready) {
			CE_leer_dht(&sensor_int);
			CE_leer_dht(&sensor_ext);

			CE_write_SD(card, card.temp_filename, sensor_int.temp_string, 0);
			CE_write_SD(card, card.temp_filename, sensor_ext.temp_string, 0);
			CE_write_SD(card, card.hum_filename, sensor_int.hum_string, 0);
			CE_write_SD(card, card.hum_filename, sensor_int.hum_string, 0);

			sensors_ready = 0;
		}
	}
}

/* --------------------------------------------------------------------- */
/* --------------------- FUNCIONES REPRESENTATIVAS --------------------- */
/* -------- ESTAS NO SON LAS FNS QUE VAMOS A USAR EN EL PROYECTO ------- */
/* --------------------------------------------------------------------- */

float calcular_dif_prom() { return 99.0; }
float crono_dia() { return 23.17; }
float crono_hora() { return 43.34; }
// Funciones de las mediciones
void activar_bluetooth (void) { ; }
void desactivar_bluetooth (void) { ; }
void muestrear_hora (void)  {; }
void muestrear_min (void) { ; }
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
	// Pines de entrada del keypad de pulsadores tactiles

    CE_conf_out(led_bateria1);
    CE_conf_out(led_bateria2);
    CE_conf_out(led_bateria3);
    // LEDs que marcan el estado actual de la bateria

    CE_conf_in(infrarrojo1, GPIO_PuPd_NOPULL);
    CE_EXTI_config(infrarrojo1, int_infrarrojo1);
    CE_conf_in(infrarrojo2, GPIO_PuPd_NOPULL);
    CE_EXTI_config(infrarrojo2, int_infrarrojo2);
    // Configuracion de sensores infrarrojos con interrupciones
}

void controlador_systick(void) { // Esta funcion es llamada en la interrupcion del SysTick
	// Configurado para interrumpir cada 1ms
	static int contSystick = 0;

	++contSystick;

	if (contSystick % BATTERY_CHECK == 0) {
		carga_bateria = CE_ADC_read(battery_adc);
		LEDs_indicadores(carga_bateria);
		contSystick=0;
	} else if (contSystick % T_SENSORES == 0) {
		sensors_ready = 1;
	} else if (contSystick % T_REFRESH_DISPLAY == 0) {
		refrescoDisplay();
	} else if (contSystick % T_PULSADORES == 0) {
		global_puls = CE_pulsador_presionado(
				puls_line_entrada_1,
				puls_line_entrada_2,
				puls_line_entrada_3,
				puls_line_entrada_4
		);
		if (global_puls == NO_BUTTON) {
			++inactividad;
			if (inactividad >= 600) {
				// No hubo actividad en los ultimos 60 segundos
				inactividad = 0;
				brilloMuyBajo();
				// Pone el brillo del display al minimo
			}
		} else {
			 inactividad = 0;
			 // ACA AGREGAR EL BRILLO CONFIGURADO
		}
	} else if (contSystick % BT_CHECK_TIME == 0) {
		if (CE_read_BT(bt, &BT_buffer)) BT_sender();
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
			muestrear_min();
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

/* --------------------------------------------------------------------- */
/* ------------------------  OTRAS FUNCIONES	------------------------ */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

void BT_sender() {
	/*
	 * Para cada caracter que le llega genera una cadena
	 * con la informacion correspondiente y la envia
	 */
	char buffer[50];
	char temp_buffer[10];
	char hum_buffer[10];

	switch (BT_buffer) {
	case 'e':
		// Envia la temperatura y humedad externas
		CE_format_float(medir_temp_ext(), temp_buffer);
		CE_format_float(medir_hum_ext(), hum_buffer);
		siprintf(
				buffer,
				"T%s|H%s",
				temp_buffer,
				hum_buffer
		);
		break;
	case 'i':
		// Envia la temperatura y la humedad internas
		CE_format_float(medir_temp_int(), temp_buffer);
		CE_format_float(medir_hum_int(), hum_buffer);
		siprintf(
				buffer,
				"T%s|H%s",
				temp_buffer,
				hum_buffer
		);
		break;
	default:
		break;
	}

	CE_send_BT(bt, buffer);
}

void LEDs_indicadores(uint32_t carga) {
	/*
	 * Si la carga de la bateria supera el 30%:
	 * 		Prende solo el primero
	 * Si supera el 60%:
	 * 		Prende los primeros dos
	 * Si supera el 90%:
	 * 		Prende los tres LEDs
	 */

	uint8_t porcentaje = ((float) carga / (MAX_ADC_VALUE - MIN_ADC_VALUE)) * 100;

	CE_escribir_salida(led_bateria1, 0);
	CE_escribir_salida(led_bateria1, 0);
	CE_escribir_salida(led_bateria1, 0);

	if (porcentaje > 30)
		CE_escribir_salida(led_bateria1, 1);
	if (porcentaje > 60)
		CE_escribir_salida(led_bateria2, 1);
	if (porcentaje > 90)
		CE_escribir_salida(led_bateria3, 1);
}

/* --------------------------------------------------------------------- */
/* --------------------- 	FUNCIONES "MEDICION" 	-------------------- */
/* ---------------	FNS DEL APARTADO "MEDICIONES" DEL MENU	------------ */
/* --------------------------------------------------------------------- */

float medir_temp_ext(){
	float aux_entero;
	float aux_frac;
	float resultado;

	aux_entero = (float) sensor_ext.temp_entero;
	aux_frac = (float) sensor_ext.temp_decimal;
	resultado = (float)(aux_entero + (aux_frac/10));

	return resultado;
}

float medir_temp_int(){
	float aux_entero;
	float aux_frac;
	float resultado;

	aux_entero = (float) sensor_ext.temp_entero;
	aux_frac = (float)sensor_ext.temp_decimal;
	resultado = (float)(aux_entero + (aux_frac/10));

	return resultado;
}

float medir_hum_ext(){
	int aux_entero;
	float resultado;

	aux_entero = sensor_ext.humedad;
	resultado = (float)(aux_entero);

	return resultado;
}

float medir_hum_int(){
	int aux_entero;
	float resultado;

	aux_entero = sensor_int.humedad;
	resultado = (float)(aux_entero);

	return resultado;
}

float ver_ingresos() {
	return (float) ingresos;
}
float ver_egresos() {
	return (float) egresos;
}

float carga_bateria_porcentaje() {
	return ((float) carga_bateria / MAX_ADC_VALUE) * 100.0;
}
float carga_bateria_tension() {
	return ((float) carga_bateria / MAX_ADC_VALUE) * MAX_ADC_VOLTS;
}

float calcular_diferencia() {
	return (float) abs(ingresos - egresos);
}

/* --------------------------------------------------------------------- */
/* ------------------- 	FUNCIONES "CONFIGURACION" 	-------------------- */
/* ----------   FNS DEL APARTADO "CONFIGURACION" DEL MENU	------------ */
/* --------------------------------------------------------------------- */

void brilloMuyBajo (void) {
	CE_PMW_change_duty(pwm, 10);
}
void brilloBajo (void) {
	CE_PMW_change_duty(pwm, 50);
}
void brilloAlto (void) {
	CE_PMW_change_duty(pwm, 75);
}
void brilloMuyAlto (void) {
	CE_PMW_change_duty(pwm, 100);
}

/* --------------------------------------------------------------------- */
/* ------------------------   INTERRUPCIONES   ------------------------- */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

void EXTI4_IRQHandler(void)
{
	if(EXTI_GetITStatus(int_infrarrojo1.int_line) != RESET && CE_EXTI_TIM_ready())
	{
		if (direccion == 2) {
			direccion = 0;
			egresos++;
		} else direccion = 1;

		CE_delay_EXTI_TIM(100000);

		EXTI_ClearITPendingBit(int_infrarrojo1.int_line);
	}
}

void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(int_infrarrojo2.int_line) != RESET && CE_EXTI_TIM_ready())
	{
		if (direccion == 1) {
			direccion = 0;
			ingresos++;
		} else direccion = 2;

		CE_delay_EXTI_TIM(100000);

		EXTI_ClearITPendingBit(int_infrarrojo2.int_line);
	}
}
