#include "Snake.h"

// TODO: Chequear constraints iniciales
Snake::Snake(int _initialLength, int _initialDirection, int _initialSpeed, int _initialRow, int _initialColumn) {
  currentDirection = _initialDirection;
  currentLength = _initialLength;
  currentSpeed = _initialSpeed;
  initialColumn = _initialColumn;
  initialRow = _initialRow;
  alive = true;
  head = 0;
}

/* Crea el cuerpo de la serpiente y lo posiciona en las matrices */
void Snake::initialize() {
  // TODO: Corregir esto para soportar diferentes initial directions
  //La serpiente empieza en linea recta mirando para la derecha
  for(int i=0; i < currentLength; i++){
    body[i].y = initialRow;
    body[i].x = initialColumn - currentLength + i;
  }
  
  head = currentLength - 1;
}

/* Getters */
long long Snake::getCurrentSpeed() {
  return currentSpeed;
}

bool Snake::isAlive() {
  return alive;
}

Direction Snake::getCurrentDirection() {
  return currentDirection;
}

Position* Snake::getBody() {
  return body;
}

int Snake::getCurrentLength() {
  return currentLength;
}

int Snake::getHead() {
  return head;
}

/* Setters */
void Snake::setCurrentSpeed(long long newSpeed) {
  currentSpeed = newSpeed;
}

bool Snake::moveSnake(Direction newDirection, bool enlarge) {
  if (!alive) {
    return false;
  }

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
    
  if((body[head].x >= HORIZONTAL_MATRIXES_QTY * MATRIX_COLUMNS) || (body[head].x < 0) || (body[head].y >= VERTICAL_MATRIXES_QTY * MATRIX_ROWS) || (body[head].y < 0)) //si se choco contra algun borde
    return alive = false;
    
  for(int i=1; i<currentLength; i++){
    if((body[head].x == body[(MAX_LENGTH + head - i)% MAX_LENGTH].x) && (body[head].y == body[(MAX_LENGTH + head - i)% MAX_LENGTH].y)) //si se choco con alguna parte de su cuerpo
      return alive = false;
  }
  
  return alive;
}
