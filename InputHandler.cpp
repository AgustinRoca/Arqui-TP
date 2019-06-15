#include "InputHandler.h"

#define CHUNK 5

InputHandler::InputHandler() {
  activeTypes = (uint8_t*) malloc(CHUNK * (sizeof(*activeTypes)));
  activePins = (uint8_t*) malloc(CHUNK * (sizeof(*activePins)));
  pins = (uint8_t*) malloc(CHUNK * (sizeof(*pins)));
  currentLenght = CHUNK;
  count = 0;
}

void InputHandler::registerPin(uint8_t pin, uint8_t activeType, bool internalPullup) {
  pinMode(pin, INPUT);
  
  if (activeType == HIGH && internalPullup) {
    digitalWrite(pin, HIGH);
  }
  
  count++;
  if (count == currentLenght) {
    activeTypes = (uint8_t*) realloc(activeTypes, sizeof(*activeTypes) * (CHUNK + currentLenght));
    activePins = (uint8_t*) realloc(activePins, sizeof(*activePins) * (CHUNK + currentLenght));
    pins = (uint8_t*) realloc(pins, sizeof(*pins) * (CHUNK + currentLenght));
    currentLenght += CHUNK;
  }
  
  activeTypes[count - 1] = activeType;
  pins[count - 1] = pin;
}

void InputHandler::registerPin(uint8_t pin, uint8_t activeType) {
  registerPin(pin, activeType, false);
}

uint8_t InputHandler::getActivePinsCount() {
  return activePinsCount;
}

const uint8_t* InputHandler::readInputs() {
  activePinsCount = 0;
  
  for (uint8_t ix = 0; ix < count; ix++) {
    if (digitalRead(pins[ix]) == activeTypes[ix]) {
      activePins[activePinsCount++] = pins[ix];
    }
  }
  
  return activePins;
}
