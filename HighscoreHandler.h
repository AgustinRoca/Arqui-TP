#ifndef _HighscoreHandler_H_
#define _HighscoreHandler_H_

#include "Arduino.h"
#include <EEPROM.h>
#include "Utilities.h"

class HighscoreHandler {
  private:
    uint32_t startingAddress;
    uint32_t maxScores;
    uint32_t currentLoadedScores;
    uint64_t * scores;
  public:
    HighscoreHandler();
    HighscoreHandler(uint32_t startingAddress,uint32_t maxScores);
    void initializeScores();
    void registerScore(uint64_t score);
    int rankScore(uint64_t score);
    uint32_t getScoresAmmount();
    uint64_t *getScores();
    void resetScores();
};

#endif
