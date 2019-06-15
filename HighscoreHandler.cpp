#include "HighscoreHandler.h"

#define BYTE_SIZE 8

/* Constructores */
HighscoreHandler::HighscoreHandler(){
}

HighscoreHandler::HighscoreHandler(uint32_t startingAddress,uint32_t maxScores){
  this->startingAddress = startingAddress;
  this->maxScores = maxScores;
  currentLoadedScores = 0;
  scores = (uint64_t*)malloc(maxScores * sizeof(*scores)); // + 1 porque en la primera posicion se guarda la cantidad de highscores
  initializeScores();
}

/* Getters */
uint32_t HighscoreHandler::getScoresAmmount(){
  return currentLoadedScores;
}

uint64_t * HighscoreHandler::getScores(){
  return scores;
}

/* Se fija si hay que almacenar el score, en caso de hacerlo, lo guarda tanto en RAM como en EEPROM */
void HighscoreHandler::registerScore(uint64_t score){
    uint32_t scoreRanking = rankScore(score); // Posicion dentro de scores en la que se deberia insertar el score
    
    if(scoreRanking <= maxScores){ // Si verdaderamente lo tengo que almacenar
      
      // Lo escribo en RAM para que quede ordenado
      for(int64_t i=(int64_t)currentLoadedScores-1; i>=scoreRanking; i--){ // Muevo todo el array para dejar espacio al nuevo
        scores[i+1] = scores[i];
      }
      scores[scoreRanking] = score; // Guardo el score en el lugar reservado para el
      if(currentLoadedScores < maxScores) // Si tengo mas elementos que antes
        currentLoadedScores++; 

      // Lo escribo en EEPROM ordenado, aprovechando que ya lo ordene en RAM
      for(int64_t i=currentLoadedScores-1; i>=scoreRanking; i--){ // Sobreescribo solamente los valores cambiados
        writeInEEPROM(startingAddress + (i+1)*sizeof(*scores), scores[i]); // Corro uno en la EEPROM por el valor de la cantidad de scores ( i+1 porque en la posicion 0 esta la cantidad de scores almacenados )
        writeInEEPROM(startingAddress, currentLoadedScores);
      }

    }
}

/* Elimina todos los puntajes almacenados */
void HighscoreHandler::resetScores(){
  writeInEEPROM(startingAddress, 0);
  currentLoadedScores = 0;
}

/* Libera el espacio reservado en heap para el arreglo de scores */
void HighscoreHandler::freeScores(){
  free(scores);
}

/* ------------------------------------------------------------------------------------------------------------- */

/* FUNCIONES AUXILIARES */

/* AUXILIAR: Escribe una variable de 64 bits en la ROM, supone que hay 64 bits libres desde la posicion address */
void HighscoreHandler::writeInEEPROM(uint64_t address, uint64_t data){
  uint64_t byteToWrite = 0;
  for(int j=sizeof(data)-1; j>=0; j--){
    byteToWrite = data & 0xFF; //Agarro el byte menos significativo
    //Serial.print("Writing in ROM: ");
    //Serial.println((long) byteToWrite, DEC);

    EEPROM.write(address + j, byteToWrite); //Lo meto en la posicion del final reservada para ese numero
    data >>= BYTE_SIZE; //Me voy al proximo byte
  }
}

/* AUXILIAR: Devuelve 1 si b>a, -1 si a<b y 0 si a=b */
int8_t HighscoreHandler::descendingCompareFunction(const void *a, const void *b) {
  int64_t result = *((uint64_t*) b) - *((uint64_t*) a);
  return result > 0 ? 1 : result < 0 ? -1 : result;
}

/* AUXILIAR: Lee de la EEPROM los puntajes almacenados y los guarda en el array de scores */
void HighscoreHandler::initializeScores(){
  // En los primeros 8 bytes se guarda la cantidad de scores que se guardaron en EEPROM
  uint64_t aux = 0;
  for(int j=0; j<sizeof(aux) - 1; j++){
      aux |= EEPROM.read(startingAddress + j);
      aux <<= BYTE_SIZE;
  }
  aux |= EEPROM.read(startingAddress + (sizeof(aux) - 1));
  currentLoadedScores = aux;

  // Carga de los scores
  for(int i=0; i<currentLoadedScores; i++){ 
    aux = 0;
    for(int j=0; j<sizeof(aux) - 1; j++){
      aux |= EEPROM.read(startingAddress + (i+1)*sizeof(*scores) + j);//Corro 1 porque la primera posicion es la cantidad de scores en la ROM
      aux <<= BYTE_SIZE;
    }
    aux |= EEPROM.read(startingAddress + (i+1)*sizeof(*scores) + (sizeof(aux) -1));
    scores[i] = aux;
  }
}

/* AUXILIAR: Devuelve la posicion dentro del array de RAM que deberia estar este score */
uint32_t HighscoreHandler::rankScore(uint64_t score){
  for(uint32_t i=currentLoadedScores - 1; i>=0; i--){
    if(scores[i] > score){
      return i+1;
    }
  }
  return 0;
}