#include <MaxMatrix.h>

#define MAX_LENGTH 256 //16x16
#define INIT_LENGTH 2
#define INIT_ROW_POS 7
#define INIT_COL_POS 7
#define INIT_SPEED 5 //no tengo ni idea

MaxMatrix screen[2][2] = {{MaxMatrix(1,2,3,4), MaxMatrix(5,6,7,8)}, {MaxMatrix(9,10,11,12), MaxMatrix(13,14,15,0)}};

typedef struct{
  int x;
  int y;
}Position;

Position body[MAX_LENGTH];
int currentLength = INIT_LENGTH;
bool gameover = false;
int snakeSpeed=INIT_SPEED;

long timeElapsed;
enum dir{UP, DOWN, LEFT, RIGHT};
dir currentDir = RIGHT;
dir input = RIGHT;

void setup() {
  for(int i=0; i<2; i++){
    for(int j=0; j<2; j++){
      screen[i][j].init();
      screen[i][j].setIntensity(8);
    }
  }
  for(int i=0; i<currentLength; i++){ //Faltaria chequear que INIT_ROW_POS-i sea >=0
    body[i].y = INIT_ROW_POS;
    body[i].x = INIT_COL_POS-i;
  }
  
  //inicializar botones
}

void loop() {
  
  //LEER INPUT DE BOTON
  
  switch(input){
    case UP:
      if(currentDir != DOWN){
        currentDir = input;
        //funcion que mueva todo en el array uno para la derecha
        body[0].x = body[1].x;
        body[0].y = body[1].y+1;
      }
    break;
    case DOWN:
      if(currentDir != UP){
        currentDir = input;
        //funcion que mueva todo en el array uno para la derecha
        body[0].x = body[1].x;
        body[0].y = body[1].y-1;
      }
    break;
    case LEFT:
      if(currentDir != RIGHT){
        currentDir = input;
        //funcion que mueva todo en el array uno para la derecha
        body[0].x = body[1].x-1;
        body[0].y = body[1].y;
      }
    break;
    case RIGHT:
      if(currentDir != LEFT){
        currentDir = input;
        //funcion que mueva todo en el array uno para la derecha
        body[0].x = body[1].x+1;
        body[0].y = body[1].y;
        //chequear si se choco 
      }
    break;
  }
}
