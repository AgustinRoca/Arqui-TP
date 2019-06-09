#ifndef _HighscoreHandler_H_
#define _HighscoreHandler_H_

#include "Arduino.h"
#include <EEPROM.h>
#include "Utilities.h"

class HighscoreHandler {
  private:
    // ---- Variables de instancia ----
    // Direccion de comienzo de EEPROM
    uint32_t startingAddress;
    
    // Almacena el tamano real del vector de scores
    uint32_t currentLength;
    uint32_t maxScores;
    
    // Puntero a primer score
    uint64_t* scores;
    
    // Cantidad efectiva de scores almacenados
    uint32_t count;
    
    
    // ---- Metodos de instancia ----
    void registerScoreEEPROM(uint64_t score, uint64_t scoreToRemove);
    void writeEEPROM(uint32_t address, uint64_t data);
    
    // Retorna indice donde insertar score en el array.
    uint32_t findPositionInScores(uint64_t score);
    uint32_t findEEPROMAddress(uint64_t score);
    
    // Lee 64 bits de EEPROM
    uint64_t readEEPROM(uint32_t address);
    
    // Desplaza los scores en el array desde la posicion from copiandolos
    // desde este indice hasta el final del array + 1
    void shiftScores(uint32_t from);
    
    // Comprueba si el dato levantado de EEPROM es de finalizacion (< 0)
    bool endReached(uint64_t data);
    
    // Lee los scores guardados en EEPROM y los ordena (en RAM) al finalizar
    void load();
    
  public:
    // "_startingAddress" tiene que ser una direccion de EEPROM valida (dependiente de board).
    HighscoreHandler(uint32_t _startingAddress, uint32_t _maxScores);
    
    // Registra un score y la guarda tanto en RAM (ordenado descendentemente) 
    // como en EEPROM (sin ordenar). El algoritmo utilizado para ordenacion (en RAM) 
    // es estable, por lo que, bajo el mismo score, mantiene el orden de llegada.
    // Devuelve el ranking del score si es un nuevo highscore, o 0 en caso contrario.
    uint32_t registerScore(uint64_t score);
    
    // Devuelve la cantidad de scores almacenado.
    uint32_t getScoresAmmount();
    
    // Devuelve un puntero a la direccion del primer score.
    const uint64_t* getScores();
    
    // Resetea zona de EEPROM utilizada para almacenaje de scores.
    // Setea un byte especial en la direccion de memoria "startingAddress"
    void reset();
};

#endif
