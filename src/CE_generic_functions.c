#include "CE_generic_functions.h"

void CE_print(char *buffer, const char* text, float num, const char *simbolo) {
    int num_entero = (int) num;                         // Asigno la parte entera del total
    float frac_num = num - num_entero;                  // Resto el total menos la parte entera y obtengo los decimales
    int entero_frac_num = trunc(frac_num * 10);        // Obtengo los decimales como un entero para mostrarlos en el display
    // Lo multiplico por 10 para tener un decimal

    if (entero_frac_num >= 10) {
        siprintf(buffer, "%s %d.%d %s", text, num_entero, entero_frac_num, simbolo);
    } else {
        siprintf(buffer, "%s %d.%d %s", text, num_entero, entero_frac_num, simbolo);
    }
}

void CE_format_float(float num, char *buffer) {
    int num_entero = (int) num;                         // Asigno la parte entera del total
    float frac_num = num - num_entero;                  // Resto el total menos la parte entera y obtengo los decimales
    int entero_frac_num = trunc(frac_num * 100);        // Obtengo los decimales como un entero para mostrarlos en el display
    // Lo multiplico por 100 para tener dos decimales

    if (entero_frac_num >= 10) {
        siprintf(buffer, "%d.%d", num_entero, entero_frac_num);
    } else {
        siprintf(buffer, "%d.%d", num_entero, entero_frac_num);
    }
}

float absolute_substract(float num1, float num2) {
	if (num1 > num2) return num1 - num2;
	else if (num1 < num2) return num2 - num1;
	else return 0;

	// Tecnicamente la ultima condicion no hace falta,
	// pero para evitar conflictos la agregue
}
// Funcion que calcula el valor absoluto sin producir overflow
// de la variable
