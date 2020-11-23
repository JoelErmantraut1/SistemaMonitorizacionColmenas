# Sistema de Monitorizacion de Colmenas
Algoritmo principal del proyecto de cuarto año de Ingeniería Electrónica que consiste en un sistema de monitorización de colmenas

## Descripción
  El mismo consiste en un dispositivo electrónico que se dedica a analizar algunas variables fundamentales para poder establecer la salud de una colmena. Estas variables son la temperatura y humedad, tanto fuera como dentro de la misma, además de un conteo de la cantidad de abejas que ingresan y egresan de la colmena.
  La lógica de esta última función es que cuando una abeja sale de la colmena a realizar el proceso de polinización, debería regresar a la misma luego de finalizada su actividad. Si el estado de salud de la abeja no es bueno, morirá en el camino, y nunca volverá a ingresar a la colmena. De esta manera, se puede emplear la diferencia entre ingresos y egresos para determinar la salud de la población.
  
## Sinopsis del proyecto
 - Monitorear a través de distintas variables la salud de la colmena.
 - Ofrecer esta información para almacenar y recopilar datos.
 - Conteo diferencial de entrada y salida de abejas.
 - Capacidad de monitorización en un período determinado.
 - Sistema compacto, transportable y adaptable.
  
## Funciones generales
 - Lecturas de temperatura y humedad dentro y fuera de la colmena.
 - Memoria SD para almacenamiento y registro de mediciones.
 - Menú integrado para navegar entre las distintas funciones.
 - Conexión Bluetooth para el levantamiento de datos y configuración del sistema.
 - Conteo diferencial del ingreso y egreso de abejas a la colmena.
 - Configuración de múltiples parámetros, como frecuencia de muestreo y brillo.
 - Leds indicadores de batería junto con visualización de la carga.
 - Funciones favorables con el ambiente, modo de bajo consumo cuando no se interactúa con el dispositivo, desactivación de periféricos fuera de uso.
 - Visualización de todas las mediciones y cronometraje desde el momento de encendido.
 
## Componentes incorporados
 - Display LCD de 2 filas por 16 columnas.
 - Teclado númerico de 4 botones.
 - Lector de tarjeta de memoria SD.
 - Módulo Bluetooth.
 - 2 sensores de temperatura y humedad.
 - Sensores infrarrojos.
 - Batería de 12V - 4Ah.
 
## Estado de avance actual del proyecto
 - Menú finalizado.
 - Medición de temperatura y humedad finalizado.
 - Registro de datos en SD listo.
 - Interacción con aplicación móvil a través de Bluetooth funcionando.
 - Conteo de abejas terminado.
 - Mayor parte de las funciones de configuración terminadas.
 - Presentación de carga de batería lista.
 - Algunas de las funciones “pro ambientales” integradas.

## Pendientes
 - Finalizar de integrar modos de bajo consumo.
 - Incorporar funciones de cronometraje.
 - Emprolijar y afinar detalles.
 
## Posibles Modificaciones futuras
 - Integración de comunicación inalámbrica de mayor distancia, como puede ser Wi-Fi.
 - Modificación del sistema de conteo de abejas por una más funcional y/o eficiente.
 - Incorporación de análisis de audio para conocer la salud de la población por medio de la frecuencia y secuencia de los zumbidos.
 - Modificación de la interfaz gráfica de la aplicación móvil.
 - Utilización de una interfaz SDIO para la SD, en lugar de SPI, para ahorrar el hardware necesario.
