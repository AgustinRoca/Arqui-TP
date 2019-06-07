#include <MaxMatrix.h>

#define HORIZONTAL_MATRIXES_QTY 1
#define VERTICAL_MATRIXES_QTY 1
#define MATRIX_ROWS 8
#define MATRIX_COLUMNS 8
#define MATRIX_SIZE (MATRIX_ROWS * MATRIX_COLUMNS)
#define MAX_LENGTH (MATRIX_SIZE * VERTICAL_MATRIXES_QTY * HORIZONTAL_MATRIXES_QTY)
#define INIT_LENGTH 2
#define INIT_ROW_POS 2 // La cabeza de la serpiente comienza en "el medio de la pantalla" (esquina superior derecha de la matriz de abajo a la izquierda)
#define INIT_COL_POS 3
#define INIT_WAIT 500 //cantidad de ms que espera inicialmente hasta el siguiente movimiento de la serpiente (velocidad)
#define WAIT_DECREASE_RATIO 0.9 // cambio de velocidad de la serpiente
#define SPEED_INCREASE_TIME 2000


/* Definicion de tipos */

enum dir{UP, DOWN, LEFT, RIGHT}; // Direcciones posibles

typedef struct{
  int x; //Horizontal 0=izquierda
  int y; //Vertical 0=abajo
} Position;


/* Definicion de clases */

class Snake {
  private:
    Position body[MAX_LENGTH]; // Arreglo que marca en que posicion de la pantalla esta cada parte del cuerpo de la serpiente
    int head; //posicion dentro del array en la que esta la cabeza
    int currentLength; //Tamanio de la serpiente
    bool alive;
    dir currentDirection; //En la direccion que esta yendo actualmente la serpiente
    long long currentSpeed; //cantidad de ms que se espera hasta el siguiente movimiento
  
  public:
    Snake() {
      head = 0;
      currentLength = INIT_LENGTH;
      alive = true;
      currentDirection = RIGHT;
      currentSpeed = INIT_WAIT;
    }
    /* Getters */
    long long getCurrentSpeed(){
      return currentSpeed;
    }
    
    bool isAlive() {
      return alive;
    }

    dir getCurrentDirection(){
      return currentDirection;
    }

    Position * getBody(){
      return body;
    }

    int getCurrentLength(){
      return currentLength;
    }

    int getHead(){
      return head;
    }

    /* Setters */
    void setCurrentSpeed(long long newSpeed){
      currentSpeed = newSpeed;
    }

    /* Crea el cuerpo de la serpiente y lo posiciona en las matrices */
    void initialize(){
      //La serpiente empieza en linea recta mirando para la derecha
      for(int i=0; i<currentLength; i++){
        body[i].y = INIT_ROW_POS;
        body[i].x = INIT_COL_POS-currentLength + i;
      }
      head = currentLength - 1;
    }
    
    bool moveSnake(dir newDirection, bool enlarge) {
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
};

dir translateInput(dir currentDir){
  if(Serial.available()){
    char c;
    c = Serial.read();
    switch(toupper(c)){
      case 'W':
        return UP;
      break;
      case 'A':
        return LEFT;
      break;
      case 'S':
        return DOWN;
      break;
      case 'D':
        return RIGHT;
      break;
      default:
        return currentDir;
    }
  }
  else
    return currentDir;
}

void printWholeBody(Position body[MAX_LENGTH], int currentLength, int head, MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY]){
  for(int i=0; i<currentLength; i++){
    screen[0][0].setDot(body[(head-i) % MAX_LENGTH].y, body[(head-i) % MAX_LENGTH].x, 8);
  }
}

void printMove(Position head, Position tail, MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY]){
  screen[0][0].setDot(tail.y, tail.x, 0);
  screen[0][0].setDot(head.y, head.x, 8);
}

void printSkull(MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY]){
  byte sf[8]= {B00111100,B01000010,B10100101,B10000001,B10011001,B10100101,B01000010,B00111100};
  for(int i=0; i<MATRIX_COLUMNS * HORIZONTAL_MATRIXES_QTY; i++){
    screen[0][0].setColumn(7-i, sf[i]);
  }
}

/* Creacion de variables globales */
Snake snake;
MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY] = {{MaxMatrix(11,13,10,1)}}; //{{MaxMatrix(1,2,3,4), MaxMatrix(5,6,7,8)}, {MaxMatrix(11,13,10,1), MaxMatrix(13,14,15,0)}}; //0,0 = Arriba izquierda; 0,1 = Arriba derecha; 1,0 = Abajo izquierda; 1,1 = Arriba derecha
dir input = RIGHT;
long long lastMovedMillis = 0;
long long lastUpdatedMillis = 0;
bool enlarge = false;


void setup() {
  //Inicializacion de las matrices
  for(int i=0; i<VERTICAL_MATRIXES_QTY; i++){
    for(int j=0; j<HORIZONTAL_MATRIXES_QTY; j++){
      screen[i][j].init();
      screen[i][j].setIntensity(8);
      screen[i][j].clear();
    }
  }
  snake.initialize();
  printWholeBody(snake.getBody(), snake.getCurrentLength(), snake.getHead(), screen);
  
  //inicializar botones
  Serial.begin(115200);
}

void loop() {
  if(millis() - lastUpdatedMillis > SPEED_INCREASE_TIME){
    lastUpdatedMillis = millis();
    snake.setCurrentSpeed(snake.getCurrentSpeed() * WAIT_DECREASE_RATIO);
    enlarge = true;
  }
  
  if(snake.isAlive()){

    //leer tecla, traducirla a dir y meterla en variable input (si no hubo tecla dejar input como estaba)
    input = translateInput(input);
    
    if(millis() - lastMovedMillis > snake.getCurrentSpeed()){ //si es tiempo de moverse
      lastMovedMillis = millis();
      snake.moveSnake(input, enlarge);
      if(snake.isAlive())
        printMove(snake.getBody()[snake.getHead()], snake.getBody()[(MAX_LENGTH + snake.getHead() - snake.getCurrentLength()) % MAX_LENGTH], screen);
      else
        printSkull(screen);
      enlarge = false;
    }
  }
}
