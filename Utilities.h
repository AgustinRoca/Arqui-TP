#ifndef _Utilities_H_
#define _Utilities_H_

#include "Arduino.h"

void bubbleSort(void* start, uint32_t lenght, uint8_t elementSize, int8_t(*compareFunction)(const void* a, const void* b));
void swap(void* a, void* b, uint8_t elementSize);

#endif
