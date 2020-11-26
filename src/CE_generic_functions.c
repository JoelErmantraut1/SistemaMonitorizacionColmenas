#include "CE_generic_functions.h"

void CE_print(char *buffer, const char* text, float num, const char *simbolo) {
    int num_entero = (int) num;                         // Asigno la parte entera del total
    float frac_num = num - num_entero;                  // Resto el total menos la parte entera y obtengo los decimales
    int entero_frac_num = trunc(frac_num * 10);        // Obtengo los decimales como un entero para mostrarlos en el display
    // Lo multiplico por 100 para tener un decimal

    if (entero_frac_num >= 10) {
        siprintf(buffer, "%s %i.%i %s", text, num_entero, entero_frac_num, simbolo);
    } else {
        siprintf(buffer, "%s %i.%i %s", text, num_entero, entero_frac_num, simbolo);
    }
}

void CE_format_float(float num, char *buffer) {
    int num_entero = (int) num;                         // Asigno la parte entera del total
    float frac_num = num - num_entero;                  // Resto el total menos la parte entera y obtengo los decimales
    int entero_frac_num = trunc(frac_num * 100);        // Obtengo los decimales como un entero para mostrarlos en el display
    // Lo multiplico por 100 para tener dos decimales

    if (entero_frac_num >= 10) {
        siprintf(buffer, "%i.%i", num_entero, entero_frac_num);
    } else {
        siprintf(buffer, "%i.%i", num_entero, entero_frac_num);
    }
}
