#ifndef _HighscoreHandler_H_
#define _HighscoreHandler_H_

#include "Arduino.h"
#include <EEPROM.h>

class HighscoreHandler {
  private:
    uint64_t startAddress;
    uint8_t currentSize;
    uint64_t* scores;
    uint8_t count;
	
    void write64(uint64_t address, uint64_t data);
    uint8_t findPosition(uint64_t score);
    uint64_t read64(uint64_t address);
    bool endReached(uint64_t data);
    void shiftScores(uint8_t from);
    void load();
    
  public:
    HighscoreHandler(uint64_t _startAddress);
    uint8_t registerScore(uint64_t score);
    uint8_t getScoresAmmount();
    uint64_t* getScores();
    void initEEPROM();
    void reset();
};

#endif
