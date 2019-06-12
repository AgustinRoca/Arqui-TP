#ifndef _Utilities_H_
#define _Utilities_H_

#include "Arduino.h"

/* Ordena un arreglo con posicion inicial start, de length cantidad de elementos, donde cada elemento tiene un tamnio de elementSize bytes, 
**  segun la funcion de comparacion compareFunction, usando el metodo de burbujeo
*/
void bubbleSort(void* start, uint32_t lenght, uint8_t elementSize, int8_t(*compareFunction)(const void* a, const void* b));

/* Intercambia elementSize bytes de la posicion a y  la posicion b */
void swap(void* a, void* b, uint8_t elementSize);

#endif
