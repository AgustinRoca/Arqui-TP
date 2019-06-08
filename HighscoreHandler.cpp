#include "HighscoreHandler.h"

#define MAX_SCORES 3
#define BYTE_SIZE 8
#define BYTE_MASK 0xFF
#define END_BYTE -1
#define CHUNK 5

HighscoreHandler::HighscoreHandler(uint64_t _startAddress) {
  startAddress = startAddress;
  load();
}

void HighscoreHandler::write64(uint64_t address, uint64_t data) {
  // MSB first
  for (uint8_t i = 0; i < sizeof(address); i++) {
    EEPROM.write(address + i, data & BYTE_MASK);
    data >>= BYTE_SIZE;
  }
}


uint8_t HighscoreHandler::findPosition(uint64_t score) {
  uint8_t ix;
  
  // Check for eq to make sure that, if the same score is registered,
  // then the new rank (or position) is not the first one (preserve order)
  for (ix = 0; ix < count && scores[ix] >= score; ix++);
  
  return ix;
}

uint64_t HighscoreHandler::read64(uint64_t address) {
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

bool HighscoreHandler::endReached(uint64_t data) {
  return data == END_BYTE;
}

void HighscoreHandler::shiftScores(uint8_t from) {
  if (from >= count)
    return; // Nothing to shift.
  
  // Avoid buffer overflow
  if (count < currentSize) {
    scores[count] = scores[count - 1];
  }
  
  for (uint8_t i = count - 2; i >= from; i--) {
    scores[i + 1] = scores[i];
  }
}

void HighscoreHandler::load() {
  scores = NULL;
  
  if (MAX_SCORES <= CHUNK) {
    currentSize = MAX_SCORES;
    scores = (uint64_t*) malloc(currentSize * sizeof(scores));
  }
  
  for (count = 0; count < MAX_SCORES; count++) {
    uint64_t address = startAddress + (count * sizeof(scores));
    uint64_t data = read64(data);
    
    if (endReached(data))
      break;
    
    if (MAX_SCORES > CHUNK && count > 0 && count % CHUNK == 0) {
      currentSize = count + CHUNK;
      scores = (uint64_t*) realloc(scores, currentSize * sizeof(scores));
    }
    
    scores[count] = data;
  }
}

uint8_t HighscoreHandler::registerScore(uint64_t score){
  uint8_t ix;
  
  if (count < MAX_SCORES && count + 1 == currentSize) {
    if (count + 1 + CHUNK >= MAX_SCORES) {
      currentSize = MAX_SCORES;
    } else {
      currentSize = count + 1 + CHUNK;
    }
    
    scores = (uint64_t*) realloc(scores, currentSize * sizeof(scores));
  }
  
  ix = findPosition(score);
  shiftScores(ix);
  scores[ix] = score;
  
  if (count < MAX_SCORES) {
    count++;
  }
  
  return ix;
}

uint8_t HighscoreHandler::getScoresAmmount() {
  return count;
}

uint64_t* HighscoreHandler::getScores() {
  return scores;
}

void HighscoreHandler::initEEPROM() {
  count = 0;
  write64(startAddress, END_BYTE);
}

void HighscoreHandler::reset() {
  if (scores != NULL)
    free(scores);

  initEEPROM();
  load();
}
