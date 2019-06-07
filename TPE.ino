#include <MaxMatrix.h>

#define HORIZONTAL_MATRIXES_QTY 2
#define VERTICAL_MATRIXES_QTY 2
#define MATRIX_SIZE 64 // 8x8
#define MAX_LENGTH (MATRIX_SIZE * VERTICAL_MATRIXES_QTY * HORIZONTAL_MATRIXES_QTY)
#define INIT_LENGTH 2
#define INIT_ROW_POS 7 // La cabeza de la serpiente comienza en "el medio de la pantalla" (esquina superior derecha de la matriz de abajo a la izquierda)
#define INIT_COL_POS 7
#define INIT_WAIT 500 //cantidad de ms que espera inicialmente hasta el siguiente movimiento de la serpiente (velocidad)
#define WAIT_DECREASE_RATIO 0.5 // cambio de velocidad de la serpiente
#define SPEED_INCREASE_TIME 15000


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
    long currentSpeed; //cantidad de ms que se espera hasta el siguiente movimiento
  
  public:
    Snake() {
      head = 0;
      currentLength = INIT_LENGTH;
      alive = true;
      currentDirection = RIGHT;
      currentSpeed = INIT_WAIT;
    }
    /* Getters */
    long getCurrentSpeed(){
      return currentSpeed;
    }
    
    bool isAlive() {
      return alive;
    }

    /* Setters */
    void setCurrentSpeed(long newSpeed){
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
          }
        break;
        case DOWN:
          if(currentDirection != UP){
            body[(head+1) % MAX_LENGTH].x = body[head].x;
            body[(head+1) % MAX_LENGTH].y = body[head].y - 1;
          }
        break;
        case LEFT:
          if(currentDirection != RIGHT){
            body[(head+1) % MAX_LENGTH].x = body[head].x - 1;
            body[(head+1) % MAX_LENGTH].y = body[head].y;
          }
        break;
        case RIGHT:
          if(currentDirection != LEFT){
            body[(head+1) % MAX_LENGTH].x = body[head].x + 1;
            body[(head+1) % MAX_LENGTH].y = body[head].y;
          }
        break;
      }
      
      currentDirection = newDirection;
      head = (head + 1) % MAX_LENGTH;
      
      if(enlarge)
        currentLength++;
        
      if((body[head].x >= HORIZONTAL_MATRIXES_QTY * 8) || (body[head].x < 0) || (body[head].y >= VERTICAL_MATRIXES_QTY * 8) || (body[head].y < 0)) //si se choco contra algun borde
        return alive = false;
        
      for(int i=1; i<currentLength; i++){
        if((body[head].x == body[(head + i)% MAX_LENGTH].x) && (body[head].y == body[(head + i)% MAX_LENGTH].y)) //si se choco con alguna parte de su cuerpo
          return alive = false;
      }
      return alive;
   }
};


/* Creacion de variables globales */
Snake snake;
MaxMatrix screen[2][2] = {{MaxMatrix(1,2,3,4), MaxMatrix(5,6,7,8)}, {MaxMatrix(11,13,10,1), MaxMatrix(13,14,15,0)}}; //0,0 = Arriba izquierda; 0,1 = Arriba derecha; 1,0 = Abajo izquierda; 1,1 = Arriba derecha
dir input = RIGHT;
long long lastUpdatedMillis = 0;
bool enlarge = false;


void setup() {
  //Inicializacion de las matrices
  for(int i=0; i<VERTICAL_MATRIXES_QTY; i++){
    for(int j=0; j<HORIZONTAL_MATRIXES_QTY; j++){
      screen[i][j].init();
      screen[i][j].setIntensity(8);
    }
  }
  //inicializar botones
}

void loop() {
  if(millis() % SPEED_INCREASE_TIME){
    snake.setCurrentSpeed(snake.getCurrentSpeed() * WAIT_DECREASE_RATIO);
    enlarge = true;
  }
  if(snake.isAlive()){
    
    //leer tecla, traducirla a dir y meterla en variable input (si no hubo tecla dejar input como estaba)
    
    if(millis() - lastUpdatedMillis > snake.getCurrentSpeed()){ //si es tiempo de moverse
      lastUpdatedMillis = millis();
      snake.moveSnake(input, enlarge);
      enlarge = false;
      //if(snake.isAlive()) meter la snake en la pantalla
      //else calavera pixelart
    }
  }
}
