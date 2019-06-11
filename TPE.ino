#include <MaxMatrix.h>

#include "HighscoreHandler.h"
#include "Direction.h"
#include "Position.h"
#include "Snake.h"
#include "InputHandler.h"

#define MATRIX_SIZE (MATRIX_ROWS * MATRIX_COLUMNS)
#define MAX_LENGTH (MATRIX_SIZE * VERTICAL_MATRIXES_QTY * HORIZONTAL_MATRIXES_QTY)
#define INIT_LENGTH 2
#define INIT_ROW_POS 2 // La cabeza de la serpiente comienza en "el medio de la pantalla" (esquina superior derecha de la matriz de abajo a la izquierda)
#define INIT_COL_POS 3
#define INIT_WAIT 500 //cantidad de ms que espera inicialmente hasta el siguiente movimiento de la vibora (velocidad)
#define WAIT_DECREASE_RATIO 0.5 // cambio de velocidad de la serpiente
#define SPEED_INCREASE_TIME 2000 //tiempo en ms entre aumento de velocidades y aumento del tamanio de la vibora
#define HORIZONTAL_MATRIXES_QTY 1
#define VERTICAL_MATRIXES_QTY 1
#define MATRIX_COLUMNS 8
#define MATRIX_ROWS 8 
#define INITIAL_EEPROM_ADDRESS 0
#define MAX_HIGHSCORES 3 //cantidad maxima de maximas puntuaciones que se almacenan
#define ON 1 //intensidad de led cuando se prende
#define OFF 0 //intensidad de led cuando se apaga
#define LEFT_BUTTON_PIN 5
#define RIGHT_BUTTON_PIN 3

Direction translateInput(Direction currentDir, HighscoreHandler * highscore, InputHandler input){
  if(Serial.available()){
    char c;
    c = Serial.read();
    
    switch(toupper(c)){
      case 'A': 
        return currentDir = (Direction) ((4 + currentDir - 1) % 4); //giro en sentido antihorario
      break;
      case 'D':
        return currentDir = (Direction) ((currentDir + 1) % 4);  //giro en sentido horario
      break;
      case 'P':
        printRom(*highscore);
        return currentDir;
        break;
      case 'R':
        highscore->resetScores();
        Serial.print("Toy\n");
        return currentDir;
        break;
      case 'Y':
        highscore->initializeScores();
        return currentDir;
        break;
      default:
        return currentDir;
    }
  }
  else
    return currentDir;
}

void printRom(HighscoreHandler highscore) {
  if (highscore.getScoresAmmount() > 0) {
    for(int i=0; i<highscore.getScoresAmmount(); i++){
      Serial.println((long)highscore.getScores()[i], DEC);
    }
  } else {
    Serial.println("Tu vieja");
  }
}

void printWholeBody(Position body[MAX_LENGTH], int currentLength, int head, MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY]){
  for(int i=0; i<currentLength; i++){
    screen[0][0].setDot(body[(head-i) % MAX_LENGTH].y, body[(head-i) % MAX_LENGTH].x, ON);
  }
}

void printMove(Position head, Position tail, MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY]){
  screen[0][0].setDot(tail.y, tail.x, OFF);
  screen[0][0].setDot(head.y, head.x, ON);
}

void printSkull(MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY]){
  byte sf[8]= {B00111100,B01000010,B10100101,B10000001,B10011001,B10100101,B01000010,B00111100};
  for(int i=0; i<MATRIX_COLUMNS * HORIZONTAL_MATRIXES_QTY; i++){
    screen[0][0].setColumn(7-i, sf[i]);
  }
}

/* Creacion de variables globales */
Snake snake(INIT_LENGTH, RIGHT, INIT_WAIT, INIT_ROW_POS, INIT_COL_POS, HORIZONTAL_MATRIXES_QTY * MATRIX_COLUMNS, VERTICAL_MATRIXES_QTY * MATRIX_ROWS);
MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY] = {{MaxMatrix(11,13,10,1)}}; //{{MaxMatrix(1,2,3,4), MaxMatrix(5,6,7,8)}, {MaxMatrix(11,13,10,1), MaxMatrix(13,14,15,0)}}; //0,0 = Arriba izquierda; 0,1 = Arriba derecha; 1,0 = Abajo izquierda; 1,1 = Arriba derecha
Direction input = RIGHT;
uint64_t lastMovedMillis = 0;
uint64_t lastUpdatedMillis = 0;
bool enlarge = false;
HighscoreHandler highscore; 
InputHandler inputHandler;

void setup() {
  //inicializar botones
  inputHandler.registerPin(RIGHT_BUTTON_PIN, HIGH);
  inputHandler.registerPin(LEFT_BUTTON_PIN, HIGH);
  
  Serial.begin(115200);
  
  //Inicializacion de las matrices
  for(int i=0; i<VERTICAL_MATRIXES_QTY; i++){
    for(int j=0; j<HORIZONTAL_MATRIXES_QTY; j++){
      screen[i][j].init();
      screen[i][j].setIntensity(ON);
      screen[i][j].clear();
    }
  }

  highscore = HighscoreHandler(INITIAL_EEPROM_ADDRESS, MAX_HIGHSCORES);
  printWholeBody(snake.getBody(), snake.getCurrentLength(), snake.getHead(), screen); 
  lastUpdatedMillis = millis();
}

void loop() {
  if(millis() - lastUpdatedMillis > SPEED_INCREASE_TIME){
    snake.setCurrentSpeed(snake.getCurrentSpeed() * WAIT_DECREASE_RATIO);
    enlarge = true;
    
    lastUpdatedMillis = millis();
  }

  if(snake.isAlive()){

    //leer tecla, traducirla a dir y meterla en variable input (si no hubo tecla dejar input como estaba)
    input = translateInput(input, &highscore, inputHandler);
    
    if(millis() - lastMovedMillis > snake.getCurrentSpeed()){ //si es tiempo de moverse
      snake.moveSnake(input, enlarge);
    
      if(snake.isAlive())
        printMove(snake.getBody()[snake.getHead()], snake.getBody()[(MAX_LENGTH + snake.getHead() - snake.getCurrentLength()) % MAX_LENGTH], screen);
      else{
        printSkull(screen);
        snake.freeSnake(); //libera solo el body, no es un destructor
        highscore.registerScore(snake.getAliveTime());
      }
      enlarge = false;
      
      lastMovedMillis = millis();
    }
  } else{
    input = translateInput(input, &highscore, inputHandler);
  }
}
