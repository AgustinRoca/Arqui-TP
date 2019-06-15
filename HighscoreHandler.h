#ifndef _HighscoreHandler_H_
#define _HighscoreHandler_H_

#include "Arduino.h"
#include <EEPROM.h>
#include "Utilities.h"

class HighscoreHandler {
  private:
    uint32_t startingAddress; // Posicion inicial de la EEPROM que se utiliza
    uint32_t maxScores; // Cantidad maxima de puntajes almacenados
    uint32_t currentLoadedScores; // Cantidad total de puntajes almacenados
    uint64_t * scores; // Arreglo de los puntajes almacenados

    /* Funciones auxiliares */
    
    /* Escribe una variable de 64 bits en la ROM, supone que hay 64 bits libres desde la posicion address */
    void writeInEEPROM(uint64_t address, uint64_t data);

    /* Devuelve 1 si b>a, -1 si a<b y 0 si a=b */
    int8_t descendingCompareFunction(const void *a, const void *b);

    /*Devuelve la posicion dentro del array de RAM que deberia estar este score */
    uint32_t rankScore(uint64_t score);

    /* Lee de la EEPROM los puntajes almacenados y los guarda en el array de scores */
    void initializeScores();
    
  public:
    /* Constructor */
    HighscoreHandler();
    
    /* Inicializador */
    void initialize(uint32_t startingAddress,uint32_t maxScores);

    /* Getters */
    uint32_t getScoresAmmount();
    uint64_t * getScores();

    /* Se fija si hay que almacenar el score, en caso de hacerlo, lo guarda tanto en RAM como en EEPROM */
    void registerScore(uint64_t score);
    
    /* Elimina todos los puntajes almacenados */
    void resetScores();

    /* Libera el espacio reservado en heap para el arreglo de scores */
    void freeScores();
};

#endif
