#ifndef _HighscoreHandler_H_
#define _HighscoreHandler_H_

#include "Arduino.h"
#include <EEPROM.h>
#include "Utilities.h"

class HighscoreHandler {
  private:
    uint32_t startingAddress;
    uint8_t currentSize;
    uint32_t maxScores;
    uint64_t* scores;
    uint8_t count;
    
    void registerScoreEEPROM(uint64_t score, uint64_t scoreToRemove);
    void write64(uint32_t address, uint64_t data);
    uint32_t findEEPROMAddress(uint64_t score);
    uint32_t findPosition(uint64_t score);
    uint64_t read64(uint32_t address);
    void shiftScores(uint32_t from);
    bool endReached(uint64_t data);
    void initEEPROM();
    void load();
    
  public:
    HighscoreHandler(uint32_t _startingAddress, uint32_t _maxScores);
    uint32_t registerScore(uint64_t score);
    uint32_t getScoresAmmount();
    uint64_t* getScores();
    void reset();
};

#endif
