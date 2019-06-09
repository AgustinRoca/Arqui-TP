#include "HighscoreHandler.h"

#define BYTE_SIZE 8
#define BYTE_MASK 0xFF
#define END_BYTE -1
#define CHUNK 5

int8_t descendingCompareFunction(const void *a, const void *b);

HighscoreHandler::HighscoreHandler(uint32_t _startingAddress, uint32_t _maxScores) {
  startingAddress = _startingAddress;
  maxScores = _maxScores;
  scores = NULL;
  load();
}

void HighscoreHandler::registerScoreEEPROM(uint64_t score, uint64_t scoreToRemove) {
  uint32_t address = findEEPROMAddress(scoreToRemove);
  write64(address, score);
}

void HighscoreHandler::write64(uint32_t address, uint64_t data) {
  // MSB first
  for (uint32_t i = 0; i < sizeof(data); i++) {
    if (EEPROM.read(address + i) != (data & BYTE_MASK)) {
      EEPROM.write(address + i, data & BYTE_MASK);
    }
    data >>= BYTE_SIZE;
  }
}

uint32_t HighscoreHandler::findEEPROMAddress(uint64_t score) {
  uint32_t i, latest, current;
  
  for (current = 0, latest = i = startingAddress; current < count; current++) {
    uint64_t romScore = read64(i);
    
    if (romScore == score) {
      latest = i;
    }
    i += 4;
  }
  
  return latest;
}

uint32_t HighscoreHandler::findPosition(uint64_t score) {
  uint32_t ix;
  
  // Check for eq to make sure that, if the same score is registered,
  // then the new rank (or position) is not the first one (preserve order)
  for (ix = 0; ix < count && scores[ix] >= score; ix++);
  
  return ix;
}

uint64_t HighscoreHandler::read64(uint32_t address) {
  uint64_t ret = 0;
  
  // MSB first
  for (uint32_t i = 0; i < sizeof(ret); i++) {
    ret |= EEPROM.read(address + i);
    
    if (i != sizeof(ret) - 1) {
      ret <<= BYTE_SIZE;
    }
  }
  
  return ret;
}

void HighscoreHandler::shiftScores(uint32_t from) {
  if (from >= count)
    return; // Nothing to shift.
  
  // Avoid buffer overflow
  if (count < currentSize) {
    scores[count] = scores[count - 1];
  }
  
  for (uint32_t i = count - 2; i >= from; i--) {
    scores[i + 1] = scores[i];
  }
}

bool HighscoreHandler::endReached(uint64_t data) {
  return data == END_BYTE;
}

void HighscoreHandler::initEEPROM() {
  count = 0;
  write64(startingAddress, END_BYTE);
}

void HighscoreHandler::load() {
  if (scores != NULL) {
    free(scores);
    scores = NULL;
  }
  
  if (maxScores <= CHUNK) {
    currentSize = maxScores;
    scores = (uint64_t*) malloc(currentSize * sizeof(*scores));
  }
  
  uint64_t address, data;
  for (count = 0; count < maxScores; count++) {
    address = startingAddress + (count * sizeof(*scores));
    data = read64(address);
    
    if (endReached(data))
      break;
    
    if (maxScores > CHUNK && count > 0 && count % CHUNK == 0) {
      currentSize = count + CHUNK;
      scores = (uint64_t*) realloc(scores, currentSize * sizeof(*scores));
    }
    
    scores[count] = data;
  }
  
  bubbleSort(scores, count, sizeof(data), descendingCompareFunction);
}



uint32_t HighscoreHandler::registerScore(uint64_t score){
  if (count < maxScores && count + 1 == currentSize) {
    if (count + 1 + CHUNK >= maxScores) {
      currentSize = maxScores;
    } else {
      currentSize = count + 1 + CHUNK;
    }
    
    scores = (uint64_t*) realloc(scores, currentSize * sizeof(*scores));
  }
  
  uint64_t lastElement = scores[count-1];
  
  uint32_t ix = findPosition(score);
  shiftScores(ix);
  scores[ix] = score;
  
  if (count < maxScores) {
    count++;
    write64(startingAddress + (count * sizeof(count)), score);
  } else {
    registerScoreEEPROM(score, lastElement);
  }
  
  return ix;
}

uint32_t HighscoreHandler::getScoresAmmount() {
  return count;
}

uint64_t* HighscoreHandler::getScores() {
  return scores;
}

void HighscoreHandler::reset() {
  if (scores != NULL) {
    free(scores);
    scores = NULL;
  }

  initEEPROM();
  load();
}


int8_t descendingCompareFunction(const void *a, const void *b) {
  int64_t result = *((uint64_t*) b) - *((uint64_t*) a);
  return result > 0L ? 1L : result < 0L ? -1L : result;
}
