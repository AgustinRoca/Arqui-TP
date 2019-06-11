#include "HighscoreHandler.h"

#define BYTE_SIZE 8

//Escribe una variable de 64 bits en la ROM, supone que hay 64 bits libres desde la posicion address
void writeInEEPROM(uint64_t address, uint64_t data){
  Serial.println("Registering in ROM...");
  Serial.println("Data: ");
  Serial.println((long) data, DEC);

  uint64_t byteToWrite = 0;
  for(int j=sizeof(data)-1; j<=0; j--){
    byteToWrite = data & 0xFF; //Agarro el byte menos significativo
    EEPROM.write(address, byteToWrite); //Lo meto en la posicion del final reservada para ese numero
    data >>= BYTE_SIZE; //Me voy al proximo byte
  }
}

int8_t descendingCompareFunction(const void *a, const void *b) {
  int64_t result = *((uint64_t*) b) - *((uint64_t*) a);
  return result > 0 ? 1 : result < 0 ? -1 : result;
}

HighscoreHandler::HighscoreHandler(){
}

HighscoreHandler::HighscoreHandler(uint32_t startingAddress,uint32_t maxScores){
  this->startingAddress = startingAddress;
  this->maxScores = maxScores;
  currentLoadedScores = 0;
  scores = (uint64_t*)malloc((maxScores+1) * sizeof(*scores)); // + 1 porque en la primera posicion se guarda la cantidad de highscores
  initializeScores();
}

void HighscoreHandler::initializeScores(){
  //En los primeros 8 bytes se guarda la cantidad de scores que se guardaron en EEPROM
  uint64_t aux = 0;
  for(int j=0; j<sizeof(aux); j++){
      aux |= EEPROM.read(startingAddress + j);
      Serial.println((long) aux, DEC);
      aux <<= BYTE_SIZE;
   }
   currentLoadedScores = aux;
   Serial.print("Current Loaded Scores: ");
   Serial.println((long) aux, DEC);
    
  for(int i=1; i<currentLoadedScores + 1; i++){ //Corro 1 porque la primera posicion es la cantidad de scores en la ROM
    aux = 0;
    for(int j=0; j<sizeof(aux); j++){
      aux |= EEPROM.read(startingAddress + i*sizeof(*scores) + j);
      aux <<= BYTE_SIZE;
    }
    scores[i] = aux;
  }
}

void HighscoreHandler::registerScore(uint64_t score){
    int scoreRanking = rankScore(score); //Posicion en la que se deberia insertar el score
    
    if(scoreRanking < maxScores){ //si lo tengo que registrar
      //lo escribo en RAM para que quede ordenado
      Serial.println((long)scoreRanking, DEC);
      Serial.println((long)maxScores, DEC);
      for(int i=currentLoadedScores-1; i>=scoreRanking; i--){ //Muevo todo el array para dejar espacio al nuevo
        Serial.println("Llegue");
        scores[i] = scores[i-1];
      }
      scores[scoreRanking] = score;
      Serial.println("Llegue 2");
      currentLoadedScores++;

      //lo escribo en ROM ordenado, aprovechando que ya lo ordene en RAM
      for(int i=currentLoadedScores-1; i>=scoreRanking; i--){ //Muevo todo el array para dejar espacio al nuevo
        writeInEEPROM(startingAddress + (i+1)*sizeof(*scores), scores[i]); //corro uno en la ROM por el valor de la cantidad de scores
        Serial.println((long)currentLoadedScores + 1, DEC);
        writeInEEPROM(startingAddress, currentLoadedScores);
      }

    }    
}

int HighscoreHandler::rankScore(uint64_t score){ //Devuelve la posicion dentro del array de ram que deberia estar este score
  for(int i=currentLoadedScores - 1; i>=0; i--){
    if(scores[i] > score){
      return i+1;
    }
  }
  return 0;
}

uint32_t HighscoreHandler::getScoresAmmount(){
  return currentLoadedScores;
}

uint64_t * HighscoreHandler::getScores(){
  return scores;
}

void HighscoreHandler::resetScores(){
  writeInEEPROM(startingAddress, 0);
  currentLoadedScores = 0;
}
