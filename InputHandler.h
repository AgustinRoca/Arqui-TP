#ifndef _InputHandler_H_
#define _InputHandler_H_

#include "Arduino.h"

class InputHandler {
  private:
    uint8_t activePinsCount;
    uint8_t currentLenght;
    uint8_t* activeTypes;
    uint8_t * activePins;
    uint8_t* pins;
    uint8_t count;
  
  public:
    InputHandler();
    
    void registerPin(uint8_t pin, uint8_t activeType, bool internalPullup);
    void registerPin(uint8_t pin, uint8_t activeType);
    int16_t getActivePin();
    const uint8_t * readInputs();
    uint8_t getActivePinsCount();
};
#endif
