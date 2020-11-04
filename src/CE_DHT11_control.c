#include "CE_DHT11_control.h"
// Librerias

uint32_t TimingDelay = 0;

// Variables

void DHT11_start(DHT_Sensor sensor) {
	GPIO_InitTypeDef GPIO_InitStructure; // Estrucura de datos para configurar el GPIO

	//
	//Inicializacion del pin.
	//

	RCC_AHB1PeriphClockCmd(sensor.periferico, ENABLE);
	GPIO_InitStructure.GPIO_Pin= sensor.pin;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_OUT ;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType= GPIO_OType_PP ;
	GPIO_InitStructure.GPIO_PuPd= GPIO_PuPd_NOPULL;

	GPIO_Init(sensor.puerto, &GPIO_InitStructure); // Carga de la estrucura de datos.

	GPIO_WriteBit(sensor.puerto, sensor.pin, 0); // pull the pin low
	delay(18000);   // wait for 18ms

	RCC_AHB1PeriphClockCmd(sensor.periferico, ENABLE);
	GPIO_InitStructure.GPIO_Pin= sensor.pin;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd= GPIO_PuPd_NOPULL;
	GPIO_Init(sensor.puerto, &GPIO_InitStructure); // Carga de la estrucura de datos.
}

void check_response(DHT_Sensor sensor)
{
	delay(40);
	if (!(GPIO_ReadInputDataBit(sensor.puerto,sensor.pin)))
	{
		delay(80);
		if ((GPIO_ReadInputDataBit(sensor.puerto,sensor.pin)))
			sensor.check = 1;
	}

	while (GPIO_ReadInputDataBit(sensor.puerto,sensor.pin));   // wait for the pin to go low
}

uint8_t read_data(DHT_Sensor sensor_read)
{
	uint8_t i,j;
	for (j=0;j<8;j++)
	{

		while (!(GPIO_ReadInputDataBit(sensor_read.puerto,sensor_read.pin)));   // wait for the pin to go high

		delay(40);   // wait for 40 us

		if ((GPIO_ReadInputDataBit(sensor_read.puerto,sensor_read.pin)) == 0)   // if the pin is low
		{
			i&= ~(1<<(7-j));   // write 0
		}
		else
			i|= (1<<(7-j));  // if the pin is high, write 1


		while ((GPIO_ReadInputDataBit(sensor_read.puerto,sensor_read.pin)));  // wait for the pin to go low or the timeout dead.
	}
	return i;
}

void CE_leer_dht(DHT_Sensor *sensor)
{

	uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
	uint16_t sum;
	char aux[20] = "";

	DHT11_start(*sensor);

	check_response(*sensor);

	Rh_byte1 = read_data(*sensor);
	Rh_byte2 = read_data(*sensor);
	Temp_byte1 = read_data(*sensor);
	Temp_byte2 = read_data(*sensor);
	sum = read_data(*sensor);

	if (sum == (Rh_byte1+Rh_byte2+Temp_byte1+Temp_byte2)) {
		if(sensor->num_identificacion == SENSOR_INT) {
			sprintf(aux,"Tint: %d.%d %cC",Temp_byte1,Temp_byte2,223); // Se crea una cadena con la temperatura.
			strcpy(sensor->temp_string,aux); // Se guarda la temperatura en la estructura del dht.

			sprintf(aux,"Hint: %d %%",Rh_byte1);
			strcpy(sensor->hum_string,aux);
			sensor->estado = STATE_DHT_CHECKSUM_GOOD; // Se cambia el estado del sensor.
		} else if (sensor->num_identificacion == SENSOR_EXT) {
			sprintf(aux,"Text: %d.%d %cC",Temp_byte1,Temp_byte2,223); // Se crea una cadena con la temperatura.
			strcpy(sensor->temp_string,aux); // Se guarda la temperatura en la estructura del dht.

			sprintf(aux,"Hext: %d %%",Rh_byte1);
			strcpy(sensor->hum_string,aux);
			sensor->estado = STATE_DHT_CHECKSUM_GOOD; // Se cambia el estado del sensor.
		}
	} else {
		sensor->estado = STATE_DHT_CHECKSUM_BAD; // Se cambia el estado del sensor.
	}

}

// Funciones del DHT11

void delay(uint32_t tiempo)
{
	/* Funcion delay()
	 * Realiza un retardo en funcion de la frecuencia de reloj del microcontrolador.
	 * Recibe como parametro el retraso, para este caso, en micro segundos debido a la configuracion del Systick.
	 */

	TIM_Cmd(TIM5, ENABLE);
	TimingDelay = tiempo;
	while(TimingDelay!=0);
	TIM_Cmd(TIM5, DISABLE);

}

void TIM5_Init(void)
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
	uint32_t frecuencia = 10e6; // Frecuencia del contador a 10kHz. Tener cuidado de no cometer overflow en la variable PrescalerValue.
	PrescalerValue = (uint16_t) ((SystemCoreClock /2) / frecuencia) - 1; //Conversion de frecuencia al valor de Prescaler.
	TIM_TimeBaseStructure.TIM_Period = 10; // 321.5uS (10e3 = 1 seg --> 3.215 = 321.5uS) de periodo.
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	TIM_ITConfig(TIM5, TIM_IT_CC1, ENABLE); // habilitacion de las interrupciones por el timer 5.
	TIM_Cmd(TIM5, ENABLE); // Habilita el contador para el timer 5.

}

void TIM5_Config(void)
{
	/*
	 *	TIMX_Config habilita el relog y las interrupciones globales para el timer X
	 */

	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); // Se habilita el reloj.

	/* Se habilitan las interrupciones globales para el timer X*/
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

void TIM5_Start(void)
{
	/*
	 *	TIMX_Start deja el timer X ready to go.
	 */

	TIM5_Config(); // Configuracion del timer.
	TIM5_Init(); // Inicializacion del timer.
	TIM_Cmd(TIM5, DISABLE);
}

void TIM5_IRQHandler (void)
{
	/* TIM5_IRQHandler rutina de interrupcion del timer 5.
	 *  Realiza un toogle en el led azul de la placa discovery.
	 */

	if (TIM_GetITStatus(TIM5, TIM_IT_CC1) != RESET)
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_CC1); // Se limpia la bandera de interrupcion.

		if (TimingDelay != 0)
		{
			TimingDelay--;
		}

	}
}

// Funciones del timer
