#include "HighscoreHandler.h"

#define BYTE_MASK 0xFF
#define BYTE_SIZE 8
#define END_BYTE -1LL // Long long para evitar problemas con operadores bitwise
#define CHUNK 5

// Funcion de comparacion utilizada para ordenacion (descendente) en RAM de scores
int8_t descendingCompareFunction(const void *a, const void *b) {
  int64_t result = *((uint64_t*) b) - *((uint64_t*) a);
  return result > 0 ? 1 : result < 0 ? -1 : result;
}


// ---- Private ----
void HighscoreHandler::registerScoreEEPROM(uint64_t score, uint64_t scoreToRemove) {
  uint32_t address = findEEPROMAddress(scoreToRemove);
  
  // Pisa valor anterior
  writeEEPROM(address, score);
}

void HighscoreHandler::writeEEPROM(uint32_t address, uint64_t data) {
  // Guarda LSB primero (similar a como lo realiza un sistema Little Endian)
  // Uso de sizeof permite no dejar espacios sin escribir en ROM en sistemas que
  // no soportan enteros de 64 bits.
  for (uint8_t i = 0; i < sizeof(data); i++) {
    // Nos quedamos con el byte menos significativo
    uint8_t b = data & BYTE_MASK;
    
    // Debido a que la cantidad de escrituras en las memorias EEPROM suele ser
    // limitada (entre 10.000 y 100.000 escrituras para un Arduino UNO (Atmega 328p))
    // y suele tardar un tiempo considerable (3.3ms para el dispositivo recien 
    // mencionado), primero confirmamos que no sobreescribiremos datos que ya se
    // encuentran guardados.
    if (EEPROM.read(address + i) != b) {
      EEPROM.write(address + i, b);
    }
    
    // Shifteamos a la derecha un byte (MSB a LSB) para procesarlo en la proxima
    // iteracion
    data >>= BYTE_SIZE;
  }
}

uint32_t HighscoreHandler::findPositionInScores(uint64_t score) {
  uint32_t ix;
  
  // Chequeo de igualdad con el fin de mantener estabilidad en orden de llegada
  for (ix = 0; ix < count && scores[ix] >= score; ix++);
  
  return ix;
}

uint32_t HighscoreHandler::findEEPROMAddress(uint64_t score) {
  // Asume que score existe y esta en la EEPROM! (Y las funciones que la llaman
  // se aseguran de esto pues solo se busca en EEPROM para eliminar). Notemos 
  // que el retorno de parametro es mas sencillo de esta manera
  uint32_t i, current;
  
  for (current = 0, i = startingAddress; current < count; current++) {
    uint64_t romScore = readEEPROM(i);
    
    if (romScore == score) {
      break;
    }
    
    i += sizeof(score) ;
  }
  
  return i;
}

uint64_t HighscoreHandler::readEEPROM(uint32_t address) {
  uint64_t ret = 0;
  
  // Little Endian
  for (uint8_t i = 0; i < sizeof(ret); i++) {
    ret |= EEPROM.read(address + i);
    
    // Si no chequeamos eliminamos el MSB!
    if (i != sizeof(ret) - 1) {
      ret <<= BYTE_SIZE;
    }
  }
  
  return ret;
}

void HighscoreHandler::shiftScores(uint32_t from) {
  // from invalido o count esta en la ultima posicion (y, por lo tanto, no
  // hay que shiftear el array)
  if (from >= count - 1)
    return;
  
  // Evita buffer overflow
  // Si hay espacio disponible copia el ultimo valor. En caso contrario,
  // se pisara adelante (si es que ya se alcanzo el numero maximo de 
  // scores a almacenar)
  if (count < currentLength) {
    scores[count] = scores[count - 1];
  }
  
  for (uint32_t i = count - 2; i > from; i--) {
    scores[i] = scores[i - 1];
  }
}

bool HighscoreHandler::endReached(uint64_t data) {
  // Chequea que el bit de signo (el MSb) este prendido, indicando si es
  // o no el final del array almacenado en la EEPROM.
  return data >> (sizeof(data) * BYTE_SIZE - 1);
}

void HighscoreHandler::load() {
  if (scores != NULL) {
    free(scores);
    scores = NULL;
    currentLength = count = 0;
  }
  
  if (maxScores <= CHUNK) {
    currentLength = maxScores;
    scores = (uint64_t*) malloc(currentLength * sizeof(*scores));
  }
  
  uint64_t address, data;
  for (count = 0; count < maxScores; count++) {
    address = startingAddress + (count * sizeof(*scores));
    data = readEEPROM(address);
    
    if (endReached(data))
      break;
    
    // Agranda el vector si es necesario
    if (maxScores > CHUNK && count % CHUNK == 0) {
      currentLength = count + CHUNK;
      scores = (uint64_t*) realloc(scores, currentLength * sizeof(*scores));
    }
    
    scores[count] = data;
  }
  
  bubbleSort(scores, count, sizeof(data), descendingCompareFunction);
}


// ---- Public ----
HighscoreHandler::HighscoreHandler(uint32_t _startingAddress, uint32_t _maxScores) {
  startingAddress = _startingAddress;
  maxScores = _maxScores;
  currentLength = 0;
  scores = NULL;
  count = 0;
  load();
}

uint32_t HighscoreHandler::registerScore(uint64_t score){
  // Guardamos el ultimo elemento (que, como el array se encuentra ordenado de manera
  // descendente, es el numero mas chico o el ultimo por orden de llegada si coincide
  // con el inmediato anterior) para luego poder eliminarlo de la EEPROM si es necesario.
  uint64_t lastElement = scores[count-1];
  if (score <= lastElement) {
    return 0;
  }
  
  // Si es posible agrandar el array (no se alcanzo a registrar la cantidad
  // maxima de scores) y va a ser necesario luego de agregar el score, lo agrandamos
  if (count < maxScores && count + 1 == currentLength) {
    if (count + 1 + CHUNK >= maxScores) {
      currentLength = maxScores;
    } else {
      currentLength = count + 1 + CHUNK;
    }
    
    scores = (uint64_t*) realloc(scores, currentLength * sizeof(*scores));
  }
  
  // Buscamos posicion de insercion en el array, lo movemos (para hacer un lugar)
  // y lo almacenamos
  uint32_t ix = findPositionInScores(score);
  shiftScores(ix);
  scores[ix] = score;
  
  if (count < maxScores) {
    // Como no se alcanzo la cantidad maxima de scores, podemos guardarlo en EEPROM
    // en la primera direccion disponible desde el offset "startingAddress"
    count++;
    writeEEPROM(startingAddress + (count * sizeof(score)), score);
  } else {
    // Pisamos valor previamente eliminado por el nuevo score.
    registerScoreEEPROM(score, lastElement);
  }
  
  return ix + 1; // Ranking empieza en 1
}

uint32_t HighscoreHandler::getScoresAmmount() {
  return count;
}

const uint64_t* HighscoreHandler::getScores() {
  return scores;
}

void HighscoreHandler::resetEEPROM() {
  if (scores != NULL) {
    free(scores);
    scores = NULL;
    currentLength = count = 0;
  }

  writeEEPROM(startingAddress, END_BYTE);
  load();
}
