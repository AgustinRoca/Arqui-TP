#ifndef _Snake_H_
#define _Snake_H_

#include "Direction.h"
#include "Position.h"

#define MAX_LENGTH 256
#define HORIZONTAL_MATRIXES_QTY 1
#define VERTICAL_MATRIXES_QTY 1
#define MATRIX_ROWS 8
#define MATRIX_COLUMNS 8

class Snake {
  private:
    Position body[MAX_LENGTH]; // Arreglo que marca en que posicion de la pantalla esta cada parte del cuerpo de la serpiente
    int head; //posicion dentro del array en la que esta la cabeza
    int currentLength; //Tamanio de la serpiente
    bool alive;
    Direction currentDirection; //En la direccion que esta yendo actualmente la serpiente
    long long currentSpeed; //cantidad de ms que se espera hasta el siguiente movimiento
    int initialRow;
    int initialColumn;
  
  public:
    Snake(int _initialLength, int _initialDirection, int _initialSpeed, int _initialRow, int _initialColumn);
    
    /* Getters */
    long long getCurrentSpeed();
    bool isAlive();
    Direction getCurrentDirection();
    Position * getBody();
    int getCurrentLength();
    int getHead();

    /* Setters */
    void setCurrentSpeed(long long newSpeed);
    void initialize();
    bool moveSnake(Direction newDirection, bool enlarge);
};

#endif
