#include "HighscoresHandler.h"

#define MAX_SCORES 3
#define BYTE_SIZE 8
#define BYTE_MASK 0xFF
#define END_BYTE -1
#define CHUNK 5

HighscoresHandler::HighscoresHandler(uint64_t _startAddress) {
  startAddress = startAddress;
  load();
}

uint64_t HighscoresHandler::read64(uint64_t address) {
  uint64_t ret = 0;
  
  // MSB first
  for (uint8_t i = 0; i < sizeof(address); i++) {
    ret |= EEPROM.read(address + i);
    
    if (i != sizeof(address) - 1) {
      ret <<= BYTE_SIZE;
    }
  }
  
  return ret;
}

bool HighscoresHandler::endReached(uint64_t address) {
  return read64(address) == END_BYTE;
}

void HighscoresHandler::write64(uint64_t address, uint64_t data) {
  // MSB first
  for (uint8_t i = 0; i < sizeof(address); i++) {
    EEPROM.write(address + i, data & BYTE_MASK);
    data >>= BYTE_SIZE;
  }
}

void HighscoresHandler::load() {
  scores = NULL;
  
  if (MAX_SCORES <= CHUNK) {
    currentSize = MAX_SCORES;
    scores = malloc(currentSize * sizeof(scores));
  }
  
  for (count = 0; count < MAX_SCORES; count++) {
    uint64_t address = startAddress + (count * sizeof(scores));
    if (endReached(address))
      break;
    
    if (MAX_SCORES > CHUNK && count > 0 && count % CHUNK == 0) {
      currentSize = count + CHUNK;
      scores = realloc(scores, currentSize * sizeof(scores));
    }
    
    scores[count] = read64(address);
  }
}

// TODO: Terminar
uint8_t HighscoresHandler::registerScore(uint64_t score){
  uint8_t ix;
  
  if (count < MAX_SCORES && count + 1 == currentSize) {
    currentSize = count + CHUNK;
    scores = realloc(scores, currentSize * sizeof(scores));
  }
  
  return ix;
}

uint8_t HighscoresHandler::getScoresAmmount() {
  return count;
}

uint64_t* HighscoresHandler::getScores() {
  return scores;
}

void HighscoresHandler::initEEPROM() {
  count = 0;
  write64(startAddress, END_BYTE);
}

void HighscoresHandler::reset() {
  if (scores != NULL)
    free(scores);

  initEEPROM();
  load();
}