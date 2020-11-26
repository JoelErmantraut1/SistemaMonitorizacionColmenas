/*
 * Algoritmo principal del proyecto de cuarto año de Ingenieria Electronica
 * que consiste en un sistema de monitorizacion de colmenas.
 */

/*
 * PENDIENTES
 *
 * - Corregir lo de mostrar el caracter de grados centigrados o humedad
 * - Incorporar configuracion por BT
 * - Vaciar en main.c
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
		"/temp_ext.csv",
		"/temp_int.csv",
		"/hum_ext.csv",
		"/hum_int.csv",
		"/health.csv",
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
char brillo;
// Variable que almacena el nivel de brillo

const char *mediciones[][3] = {
    {"Exterior", "Temp:", "Hum:"},
    {"Interior", "Temp:", "Hum:"},
    {"Carga de bat.", "Porc:", "Tension:"},
    {"Ing. y Egr.", "Ingresos:", "Egresos:"},
    {"Diferencia", "Diferencia:", "Promedio:"},
    {"Cronometro", "Dia:", "Hora:"}
};

const char *simbolos[][2] = {
		{"ßC", "%"}, // El caracter ß, es el 223, que en la tabla del display, es el °
		{"ßC", "%"},
		{"%", "V"},
		{"", ""},
		{"", ""},
		{"", ""}
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

	CE_DHT11_TIM5_Start(); // Inicializa el timer del DHT

    CE_PWM_init(salida_pwm, pwm);
	UB_LCD_2x16_Init();
	CE_Print_StartScreen();
	// Imprime el cartel inicial durante 5 segundos

	CE_TIM5_delay(5000000);
	// 5e6 us = 5000ms = 5s

	UB_LCD_2x16_Clear();
	do {
		UB_LCD_2x16_String(0, 0, "Carg. Config.");
		UB_LCD_2x16_String(0, 1, "Verificando SD.");
	} while (!cargar_configuracion());
	// Carga la configuracion
	// Y de paso verifica el funcionamiento de la SD

	PORT_init();
	// Configura pulsadores, LEDs e infrarrojos

	CE_ADC_init(battery_adc);
	carga_bateria = CE_ADC_read(battery_adc);
	LEDs_indicadores(carga_bateria);
	// Medimos bateria y presentamos la carga con los LEDs

	UB_LCD_2x16_Clear();
	UB_LCD_2x16_String(0, 0, "Verific. Sens.");
	UB_LCD_2x16_String(0, 1, "Temp. y Hum.");

	SysTick_Config(SystemCoreClock / SYSTICK_CONSTANT);
	// El SysTick incorpora la mayoria de la funciones temporizadas

	while (1) {
		if (sensors_ready) {
			CE_leer_dht(&sensor_int);
			CE_leer_dht(&sensor_ext);
			// Mido temperatura y humedad

			char buffer[10];

			siprintf(buffer, "%d.%d,", sensor_ext.temp_entero, sensor_ext.temp_decimal);
			CE_write_SD(card, card.temp_ext_filename, buffer, 0);
			siprintf(buffer, "%d.%d,", sensor_int.temp_entero, sensor_int.temp_decimal);
			CE_write_SD(card, card.temp_int_filename, buffer, 0);
			siprintf(buffer, "%d,", sensor_ext.humedad);
			CE_write_SD(card, card.hum_ext_filename, buffer, 0);
			siprintf(buffer, "%d,", sensor_int.humedad);
			CE_write_SD(card, card.hum_int_filename, buffer, 0);
			// Los guardo en la tarjeta

			sensors_ready = 0;
			// Clareo flag para la siguiente medicion
		}
		// Cuando la variable cambia con el SysTick, se leen los datos
		// de los sensores y se guardan en su correspondiente archivo
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
void activar_bluetooth (void) { CE_init_BT(bt); }
void desactivar_bluetooth (void) { ; }
void muestrear_hora (void)  {; }
void muestrear_min (void) { ; }
// Funciones de la configuracion

/* --------------------------------------------------------------------- */
/* --------------------------  FNS GENERALES  -------------------------- */
/* --------------------------------------------------------------------- */
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

void BT_sender() {
	/*
	 * Para cada caracter que le llega genera una cadena
	 * con la informacion correspondiente y la envia
	 */
	char buffer[50];
	char first_buffer[10];
	char second_buffer[10];

	switch (BT_buffer) {
	case 'e':
		// Envia la temperatura y humedad externas
		CE_format_float(medir_temp_ext(), first_buffer);
		CE_format_float(medir_hum_ext(), second_buffer);
		siprintf(
				buffer,
				"T%s|H%s",
				first_buffer,
				second_buffer
		);
		break;
	case 'i':
		// Envia la temperatura y la humedad internas
		CE_format_float(medir_temp_int(), first_buffer);
		CE_format_float(medir_hum_int(), second_buffer);
		siprintf(
				buffer,
				"t%s|h%s",
				first_buffer,
				second_buffer
		);
		break;
	case 'a':
		// Envia la cantidad de ingresos y egresos
		CE_format_float(ver_ingresos(), first_buffer);
		CE_format_float(ver_egresos(), second_buffer);
		siprintf(
				buffer,
				"i%s|e%s",
				first_buffer,
				second_buffer
		);
		break;
	case 'd':
		// Envia la diferencia entre ingresos y egresos
		// Y la salud de la colmena
		CE_format_float(calcular_diferencia(), first_buffer);
		CE_format_float(20.0, second_buffer);
		siprintf(
				buffer,
				"d%s|s%s",
				first_buffer,
				second_buffer
		);
		break;
	case 'c':
		// Envia la carga de la bateria como tension y como porcentaje
		CE_format_float(carga_bateria_tension(), first_buffer);
		CE_format_float(carga_bateria_porcentaje(), second_buffer);
		siprintf(
				buffer,
				"v%s|p%s",
				first_buffer,
				second_buffer
		);
		break;
	case 'r':
		// Envia informacion del cronometro
		// Falta incorporar RTC
		break;
	case 'b':
		// Cambia el estado del BT
		// Envia el estado actual
		break;
	case 'm':
		// Cambia la frecuencia de muestreo
		// Envia la frecuencia actual
		break;
	case 'x':
		// Cambia el brillo
		// Envia el brillo actual
		break;
	default:
		break;
	}

	CE_send_BT(bt, buffer);
}

void LEDs_indicadores(uint32_t carga) {
	/*
	 * Si la carga es menor al 30%:
	 * 		Apaga todos
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

void ajustar_brillo(char brillo) {
	switch (brillo) {
	case '0':
		brilloMuyBajo();
		break;
	case '1':
		brilloBajo();
		break;
	case '2':
		brilloAlto();
		break;
	case '3':
		brilloMuyAlto();
		break;
	default: // El brillo por defecto es "muy alto"
		brilloMuyAlto();
		break;
	}
	// Ajustar brillo
}

int cargar_configuracion(void) {
	char buffer[MAX_CONF_LEN];
	if (!CE_read_SD(card, card.config_filename, buffer))
		return 0;

	/*
	 * Primer caracter:
	 *  - 0: BT apagado
	 *  - 1: BT prendido
	 * Segundo caracter:
	 *  - 0: Muestreo cada 15 min
	 *  - 1: Muestreo cada 1 hora
	 * Tercer caracter:
	 *  - 0: Brillo muy bajo
	 *  - 1: Brillo bajo
	 *  - 2: Brillo alto
	 *  - 3: Brillo muy alto
	 */
	if (buffer[BT_INDEX] == '1') activar_bluetooth();
	else desactivar_bluetooth();
	// Activar - desactivar BT
	if (buffer[FREC_SAMPLE_INDEX] == '1') muestrear_min();
	else muestrear_hora();
	// Ajustar frecuencia de muestreo
	brillo = buffer[BRIGHTNESS_INDEX];
	ajustar_brillo(brillo);
	// Ajustar brillo

	return 1;
}

void cambiar_configuracion(uint8_t item, uint8_t value) {
	/*
	 * Recibe el item que se quiere modificar y el valor
	 * que se le quiere poner.
	 */

	char buffer[MAX_CONF_LEN];
	CE_read_SD(card, card.config_filename, buffer);
	// Lee el contenido actual

	buffer[item] = value;
	// Lo modifica

	CE_write_SD(card, card.config_filename, buffer, 1);
	// Lo vuelve a guardar en la SD
}

void controlador_systick(void) { // Esta funcion es llamada en la interrupcion del SysTick
	// Configurado para interrumpir cada SYSTICK_CONSTANT us
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
			/*
			 * Esta funcion reduce el brillo de la pantalla al minimo
			 * cuando no se presionaron teclas durante aproximadamente
			 * 1 minuto.
			 * Luego, cuando se presiona una tecla, pone el brillo alto.
			 */
			++inactividad;
			if (inactividad >= 600) {
				// No hubo actividad en los ultimos 60 segundos
				inactividad = 0;
				CE_PMW_change_duty(pwm, 10);
				// Fuerza el brillo del display al minimo
			}
		} else {
			 inactividad = 0;
			 ajustar_brillo(brillo);
			 // Pone el brillo del display alto para empezar a interactuar
		}
	} else if (contSystick % BT_CHECK_TIME == 0) {
		if (CE_read_BT(bt, &BT_buffer)) BT_sender();
		// Si recibio algo, envia lo que corresponde
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
	/*
	 * Se encarga de decidir que contenido mostrar en el menu
	 * dentro del apartado de mediciones
	 */

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
	        CE_print(fila1,
	        		mediciones[pantalla + last_global_puls - 1][1],
					fn[pantalla + last_global_puls - 1][0](),
					simbolos[pantalla + last_global_puls - 1][0]
			);
	        CE_print(fila2,
	        		mediciones[pantalla + last_global_puls - 1][2],
					fn[pantalla + last_global_puls - 1][1](),
					simbolos[pantalla + last_global_puls - 1][1]
			);
		}
	}

	global_puls = NO_BUTTON;
}

void select_menu_config(char *fila1, char *fila2) {
	/*
	 * Se encarga de decidir que contenido mostrar en el menu
	 * dentro del apartado de configuraciones
	 */

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
/* --------------------- 	FUNCIONES "MEDICION" 	-------------------- */
/* ---------------	FNS DEL APARTADO "MEDICIONES" DEL MENU	------------ */
/* --------------------------------------------------------------------- */

float medir_temp_ext() {
	float aux_entero;
	float aux_frac;
	float resultado;

	aux_entero = (float) sensor_ext.temp_entero;
	aux_frac = (float) sensor_ext.temp_decimal;
	resultado = (float)(aux_entero + (aux_frac/10));

	return resultado;
}
float medir_temp_int() {
	float aux_entero;
	float aux_frac;
	float resultado;

	aux_entero = (float) sensor_int.temp_entero;
	aux_frac = (float)sensor_int.temp_decimal;
	resultado = (float)(aux_entero + (aux_frac/10));

	return resultado;
}
// Medicion de temperatura externa e interna

float medir_hum_ext() {
	int aux_entero;
	float resultado;

	aux_entero = sensor_ext.humedad;
	resultado = (float)(aux_entero);

	return resultado;
}
float medir_hum_int() {
	int aux_entero;
	float resultado;

	aux_entero = sensor_int.humedad;
	resultado = (float)(aux_entero);

	return resultado;
}
// Medicion de humedad externa e interna

float ver_ingresos() {
	return (float) ingresos;
}
float ver_egresos() {
	return (float) egresos;
}
// Cantidad de ingresos o egresos de abejas a la colmena

float carga_bateria_porcentaje() {
	return ((float) carga_bateria / MAX_ADC_VALUE) * 100.0;
}
float carga_bateria_tension() {
	return ((float) carga_bateria / MAX_ADC_VALUE) * MAX_ADC_VOLTS;
}
// Ambas muestran la misma carga, solo que una la muestra en
// volts y la otra porcentualmente

float calcular_diferencia() {
	return (float) abs(ingresos - egresos);
	// Los ingresos pueden ser mayores que los egresos o al reves
	// Por eso se agrega un valor absoluto para que muestre el
	// indicador modular que caracteriza a la opcion
}

/* --------------------------------------------------------------------- */
/* ------------------- 	FUNCIONES "CONFIGURACION" 	-------------------- */
/* ----------   FNS DEL APARTADO "CONFIGURACION" DEL MENU	------------ */
/* --------------------------------------------------------------------- */

void brilloMuyBajo (void) {
	CE_PMW_change_duty(pwm, 10);
	cambiar_configuracion(BRIGHTNESS_INDEX, '0');
	brillo = '0';
}
void brilloBajo (void) {
	CE_PMW_change_duty(pwm, 40);
	cambiar_configuracion(BRIGHTNESS_INDEX, '1');
	brillo = '1';
}
void brilloAlto (void) {
	CE_PMW_change_duty(pwm, 60);
	cambiar_configuracion(BRIGHTNESS_INDEX, '2');
	brillo = '2';
}
void brilloMuyAlto (void) {
	CE_PMW_change_duty(pwm, 100);
	cambiar_configuracion(BRIGHTNESS_INDEX, '3');
	brillo = '3';
}
// Funciones que varian el brillo del LCD

void CE_Print_StartScreen(void)
{
	/*
	 * Esta funcion configura un cartel que se puede colocar
	 * al inicio de programa para mostrar una pantalla de
	 * bienvenida con el nombre y logo del proyecto
	 */

	// Definición de un carácter especial en 8 bytes.
	uint8_t buf0[8];
	buf0[0]=0b00000000;
	buf0[1]=0b00000000;
	buf0[2]=0b00000000;
	buf0[3]=0b00000110;
	buf0[4]=0b00000101;
	buf0[5]=0b00000000;
	buf0[6]=0b00000000;
	buf0[7]=0b00000000;
	UB_LCD_2x16_WriteCG(0, buf0); // Almacenar un carácter especial en el CG-RAM desde la pantalla
	UB_LCD_2x16_PrintCG(0, 0, 0); // Salida del carácter especial en la pos. X, Y

	uint8_t buf1[8];
	buf1[0]=0b00000000;
	buf1[1]=0b00000000;
	buf1[2]=0b00000000;
	buf1[3]=0b00000000;
	buf1[4]=0b00010001;
	buf1[5]=0b00001010;
	buf1[6]=0b00001110;
	buf1[7]=0b00011111;
	UB_LCD_2x16_WriteCG(1, buf1);
	UB_LCD_2x16_PrintCG(1, 0, 1);

	uint8_t buf2[8];
	buf2[0]=0b00000000;
	buf2[1]=0b00000000;
	buf2[2]=0b00000000;
	buf2[3]=0b00001100;
	buf2[4]=0b00010100;
	buf2[5]=0b00000000;
	buf2[6]=0b00000000;
	buf2[7]=0b00000000;
	UB_LCD_2x16_WriteCG(2, buf2);
	UB_LCD_2x16_PrintCG(2, 0, 2);

	uint8_t buf3[8];
	buf3[0]=0b00000000;
	buf3[1]=0b00001101;
	buf3[2]=0b00001110;
	buf3[3]=0b00000111;
	buf3[4]=0b00000111;
	buf3[5]=0b00000111;
	buf3[6]=0b00001110;
	buf3[7]=0b00000100;
	UB_LCD_2x16_WriteCG(3, buf3);
	UB_LCD_2x16_PrintCG(0, 1, 3);

	uint8_t buf4[8];
	buf4[0]=0b00000000;
	buf4[1]=0b00011111;
	buf4[2]=0b00000000;
	buf4[3]=0b00011111;
	buf4[4]=0b00000000;
	buf4[5]=0b00001110;
	buf4[6]=0b00000000;
	buf4[7]=0b00001110;
	UB_LCD_2x16_WriteCG(4, buf4);
	UB_LCD_2x16_PrintCG(1, 1, 4);

	uint8_t buf5[8];
	buf5[0]=0b00000000;
	buf5[1]=0b00010110;
	buf5[2]=0b00001110;
	buf5[3]=0b00011100;
	buf5[4]=0b00011100;
	buf5[5]=0b00011100;
	buf5[6]=0b00001110;
	buf5[7]=0b00000100;
	UB_LCD_2x16_WriteCG(5, buf5);
	UB_LCD_2x16_PrintCG(2, 1, 5);

	UB_LCD_2x16_String(4,0,"Monitor de");
	UB_LCD_2x16_String(5,1,"Colmenas");
}

/* --------------------------------------------------------------------- */
/* ------------------------   INTERRUPCIONES   ------------------------- */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

void EXTI4_IRQHandler(void)
{

	if(EXTI_GetITStatus(int_infrarrojo1.int_line) != RESET) {
		if (int_infrarrojo1.int_trigger == EXTI_Trigger_Rising) {
			if (direccion == 2) {
				direccion = 0;
				egresos++;
			} else direccion = 1;
		}

		// CE_EXTI_change_trigger(&int_infrarrojo1);

		EXTI_ClearITPendingBit(int_infrarrojo1.int_line);
	}
}

void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(int_infrarrojo2.int_line) != RESET) {
		if (int_infrarrojo2.int_trigger == EXTI_Trigger_Rising) {
			if (direccion == 1) {
				direccion = 0;
				ingresos++;
			} else direccion = 2;
		}

		// CE_EXTI_change_trigger(&int_infrarrojo2);

		EXTI_ClearITPendingBit(int_infrarrojo2.int_line);
	}
}
