#include <MaxMatrix.h>

#define HORIZONTAL_MATRIXES_QTY 2
#define VERTICAL_MATRIXES_QTY 2
#define MATRIX_SIZE 64 // 8x8
#define MAX_LENGTH (MATRIX_SIZE * VERTICAL_MATRIXES_QTY * HORIZONTAL_MATRIXES_QTY)
#define INIT_LENGTH 2
#define INIT_ROW_POS 7 // La cabeza de la serpiente comienza en "el medio de la pantalla" (esquina superior derecha de la matriz de abajo a la izquierda)
#define INIT_COL_POS 7
#define INIT_SPEED 2000 //cantidad de ms que espera inicialmente hasta el siguiente movimiento de la serpiente 


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
      head = 0; //La cabeza empieza en la posicion 0 del array
      currentLength = INIT_LENGTH;
      alive = true;
      currentDirection = RIGHT;
    }
    /* Getters */
    long getCurrentSpeed(){
      return currentSpeed;
    }
    
    bool isAlive() {
      return alive;
    }

    /* Crea el cuerpo de la serpiente y lo posiciona en las matrices */
    int initialize(){
      if(INIT_ROW_POS - (currentLength - 1) < 0){
        fprintf(stderr, "El tamanio de la serpiente es muy grande para empezar\n");
        return 1;
      }
        
      //La serpiente empieza en linea recta mirando para la derecha
      for(int i=0; i<currentLength; i++){
        body[i].y = INIT_ROW_POS;
        body[i].x = INIT_COL_POS-i;
      }
      return 0;
    }
    
    bool moveSnake(dir newDirection) {
      if (!alive) {
        return false;
      }

      switch(newDirection){
        case UP:
          if(currentDirection != DOWN){
            currentDirection = newDirection;
            body[(head+1) % MAX_LENGTH].x = body[(head+1) % MAX_LENGTH].x;
            body[(head+1) % MAX_LENGTH].y = body[(head) % MAX_LENGTH].y + 1;
          }
        break;
        case DOWN:
          if(currentDirection != UP){
            currentDirection = newDirection;
            body[(head+1) % MAX_LENGTH].x = body[(head+1) % MAX_LENGTH].x;
            body[(head+1) % MAX_LENGTH].y = body[(head) % MAX_LENGTH].y - 1;
          }
        break;
        case LEFT:
            currentDirection = newDirection;
            body[(head+1) % MAX_LENGTH].x = body[(head+1) % MAX_LENGTH].x - 1;
            body[(head+1) % MAX_LENGTH].y = body[(head) % MAX_LENGTH].y;
        break;
        case RIGHT:
          if(currentDirection != LEFT){
            currentDirection = newDirection;
            body[(head+1) % MAX_LENGTH].x = body[(head+1) % MAX_LENGTH].x + 1;
            body[(head+1) % MAX_LENGTH].y = body[(head) % MAX_LENGTH].y;
            //chequear si se choco 
          }
        break;
      }
      //verificar si se choco y en ese caso cambiar alive a false
      return alive;
    }
};


/* Creacion de variables globales */
Snake snake;
MaxMatrix screen[2][2] = {{MaxMatrix(1,2,3,4), MaxMatrix(5,6,7,8)}, {MaxMatrix(9,10,11,12), MaxMatrix(13,14,15,0)}}; //0,0 = Arriba izquierda; 0,1 = Arriba derecha; 1,0 = Abajo izquierda; 1,1 = Arriba derecha
dir input = RIGHT;


void setup() {
  //Inicializacion de las matrices
  for(int i=0; i<VERTICAL_MATRIXES_QTY; i++){
    for(int j=0; j<HORIZONTAL_MATRIXES_QTY; j++){
      screen[i][j].init();
      screen[i][j].setIntensity(8);
    }
  }
  if(snake.initialize() != 0){ // Si hubo error en la creacion inicializacion de la serpiente en la pantalla
    return;
  }
  
  //inicializar botones
}

void loop() {
  if(snake.isAlive()){
    
    //leer tecla y meterlo en variable input
    snake.moveSnake(input);
    //if(snake.isAlive()) meter la snake en la pantalla
  }
  delay(snake.getCurrentSpeed());

}
