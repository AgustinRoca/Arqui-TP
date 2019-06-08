#ifndef _Snake_H_
#define _Snake_H_

#include "Arduino.h"
#include "Direction.h"
#include "Position.h"
#include <EEPROM.h>

#define HORIZONTAL_MATRIXES_QTY 1
#define VERTICAL_MATRIXES_QTY 1
#define MATRIX_COLUMNS 8
#define MAX_LENGTH 256
#define MATRIX_ROWS 8

class Snake {
  private:
    Direction currentDirection; //En la direccion que esta yendo actualmente la serpiente
    uint16_t currentLength; //Tamanio de la serpiente
    uint64_t currentSpeed; //cantidad de ms que se espera hasta el siguiente movimiento
    uint64_t startTime;
    uint64_t stopTime;
    Position body[]; // Arreglo que marca en que posicion de la pantalla esta cada parte del cuerpo de la serpiente
    uint16_t head; //posicion dentro del array en la que esta la cabeza
    bool alive;
  
  public:
    /* Crea el cuerpo de la serpiente y lo posiciona en las matrices */
    Snake(uint16_t _initialLength, Direction _initialDirection, uint64_t _initialSpeed, uint16_t _initialRow, uint16_t _initialColumn);
    
    /* Getters */
    Direction getCurrentDirection();
    uint16_t getCurrentLength();
    uint64_t getCurrentSpeed();
    uint64_t getAliveTime();
    Position* getBody();
    uint16_t getHead();
    bool isAlive();

    /* Setters */
    bool moveSnake(Direction newDirection, bool enlarge);
    void setCurrentSpeed(uint64_t newSpeed);
};

#endif
