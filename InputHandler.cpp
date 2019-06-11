#include "InputHandler.h"

#define CHUNK 5

InputHandler::InputHandler() {
  activeTypes = (uint8_t*) malloc(CHUNK * (sizeof(*pins)));
  pins = (uint8_t*) malloc(CHUNK * (sizeof(*pins)));
  currentLenght = CHUNK;
  activePin = -1;
  count = 0;
}

void InputHandler::registerPin(uint8_t pin, uint8_t activeType, bool internalPullup) {
  pinMode(pin, INPUT);
  
  if (activeType == HIGH && internalPullup) {
    digitalWrite(pin, HIGH);
  }
  
  count++;
  if (count == currentLenght) {
    activeTypes = (uint8_t*) realloc(activeTypes, sizeof(*pins) * (CHUNK + count));
    pins = (uint8_t*) realloc(pins, sizeof(*pins) * (CHUNK + count));
    currentLenght = CHUNK;
  }
}

void InputHandler::registerPin(uint8_t pin, uint8_t activeType) {
  registerPin(pin, activeType, false);
}

int16_t InputHandler::getActivePin() {
  return activePin;
}

int16_t InputHandler::readInputs() {
  bool found = false;
  
  for (uint8_t ix = 0; ix < count && !found; ix++) {
    Serial.print("Read: ");
    Serial.print(digitalRead(pins[ix]));
    Serial.print(" ");
    Serial.println(activeTypes[ix]);
    if (digitalRead(pins[ix]) == activeTypes[ix]) {
      activePin = pins[ix];
      found = true;
    }
  }
  
  if (!found) {
    activePin = -1;
  }
  
  return activePin;
}
