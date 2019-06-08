#include "Snake.h"

#define HORIZONTAL_MATRIXES_QTY 1
#define VERTICAL_MATRIXES_QTY 1
#define MATRIX_COLUMNS 8
#define MAX_LENGTH 256
#define MATRIX_ROWS 8

// TODO: Chequear constraints iniciales
Snake::Snake(uint16_t _initialLength, Direction _initialDirection, uint64_t _initialSpeed, uint16_t _initialRow, uint16_t _initialColumn) {
  currentDirection = _initialDirection;
  currentLength = _initialLength;
  currentSpeed = _initialSpeed;
  alive = true;
  
  // TODO: Corregir esto para soportar diferentes initial directions
  //La serpiente empieza en linea recta mirando para la derecha
  for(uint16_t i=0; i < currentLength; i++){
    body[i].y = initialRow;
    body[i].x = initialColumn - currentLength + i;
  }
  
  head = currentLength - 1;
}

/* Getters */
Direction Snake::getCurrentDirection() {
  return currentDirection;
}

uint16_t Snake::getCurrentLength() {
  return currentLength;
}

uint64_t Snake::getCurrentSpeed() {
  return currentSpeed;
}

uint64_t Snake::getAliveTime() {
  return alive ? stopTime - startTime : millis() - startTime;
}

Position* Snake::getBody() {
  return body;
}

uint16_t Snake::getHead() {
  return head;
}

bool Snake::isAlive() {
  return alive;
}

/* Setters */
bool Snake::moveSnake(Direction newDirection, bool enlarge) {
  if (!alive) {
    return false;
  }

  uint64_t possibleStopTime = millis();  // Don't account for time spent on routine
  
  switch(newDirection){
    case UP:
      if(currentDirection != DOWN){
        body[(head+1) % MAX_LENGTH].x = body[head].x;
        body[(head+1) % MAX_LENGTH].y = body[head].y + 1;
        currentDirection = newDirection;
      }
      else{
        body[(head+1) % MAX_LENGTH].x = body[head].x;
        body[(head+1) % MAX_LENGTH].y = body[head].y - 1;
      }
    break;
    case DOWN:
      if(currentDirection != UP){
        body[(head+1) % MAX_LENGTH].x = body[head].x;
        body[(head+1) % MAX_LENGTH].y = body[head].y - 1;
        currentDirection = newDirection;
      }
      else{
        body[(head+1) % MAX_LENGTH].x = body[head].x;
        body[(head+1) % MAX_LENGTH].y = body[head].y + 1;
      }
    break;
    case LEFT:
      if(currentDirection != RIGHT){
        body[(head+1) % MAX_LENGTH].x = body[head].x - 1;
        body[(head+1) % MAX_LENGTH].y = body[head].y;
        currentDirection = newDirection;
      }
      else{
        body[(head+1) % MAX_LENGTH].x = body[head].x + 1;
        body[(head+1) % MAX_LENGTH].y = body[head].y;
      }
    break;
    case RIGHT:
      if(currentDirection != LEFT){
        body[(head+1) % MAX_LENGTH].x = body[head].x + 1;
        body[(head+1) % MAX_LENGTH].y = body[head].y;
        currentDirection = newDirection;
      }
      else{
        body[(head+1) % MAX_LENGTH].x = body[head].x - 1;
        body[(head+1) % MAX_LENGTH].y = body[head].y;
      }
    break;
  }
  
  head = (head + 1) % MAX_LENGTH;
  
  if(enlarge){
    currentLength++;
  }
    
  if((body[head].x >= HORIZONTAL_MATRIXES_QTY * MATRIX_COLUMNS) || (body[head].x < 0) || (body[head].y >= VERTICAL_MATRIXES_QTY * MATRIX_ROWS) || (body[head].y < 0)) { //si se choco contra algun borde
    stopTime = possibleStopTime;
    return alive = false;
  }
  
  for(uint16_t i=1; i < currentLength; i++){
    if((body[head].x == body[(MAX_LENGTH + head - i)% MAX_LENGTH].x) && (body[head].y == body[(MAX_LENGTH + head - i)% MAX_LENGTH].y)) { //si se choco con alguna parte de su cuerpo
      stopTime = possibleStopTime;
      return alive = false;
    }
  }
  
  return alive;
}

void Snake::setCurrentSpeed(uint64_t newSpeed) {
  currentSpeed = newSpeed;
}
