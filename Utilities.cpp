#include "Utilities.h"

void bubbleSort(void* start, uint32_t lenght, uint8_t elementSize, int8_t(*compareFunction)(const void* a, const void* b)) {
  for (uint32_t i = 0; i < lenght; i++) {
    for (uint32_t j = 0; j < lenght - 1; j++) {
      void* left = start + (i * elementSize);
      void* right = start + ((j + 1) * elementSize);
      
      if (compareFunction(left, right) > 0) {
        swap(left, right, elementSize);
      }
    }
  }
}

void swap(void* a, void* b, uint8_t elementSize) {
  uint8_t aux;
  uint8_t* pointerA = (uint8_t*) a;
  uint8_t* pointerB = (uint8_t*) b;
  
  for (uint32_t i = 0; i < elementSize; i++) {
    aux = pointerA[i];
    
    pointerA[i] = pointerB[i];
    pointerB[i] = aux;
  }
}

uint32_t millis2() {
  static uint32_t time = 1;
  time += 500;
  return time;
}
