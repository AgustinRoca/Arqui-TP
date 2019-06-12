#include <MaxMatrix.h>

#include "HighscoreHandler.h"
#include "Direction.h"
#include "Position.h"
#include "Snake.h"
#include "InputHandler.h"

/* Constantes relacionadas con el tamanio/cantidad de las matrices*/
#define MATRIX_COLUMNS 8 // Cantidad de columnas de LEDs que tiene UNA matriz
#define MATRIX_ROWS 8 // Cantidad de filas de LEDs que tiene UNA matriz
#define MATRIX_SIZE (MATRIX_ROWS * MATRIX_COLUMNS) // Cantidad total de LEDs que tiene UNA matriz
#define MAX_LENGTH (MATRIX_SIZE * VERTICAL_MATRIXES_QTY * HORIZONTAL_MATRIXES_QTY) // Cantidad total de LEDs totals que tiene la pantalla completa (la vibora no puede ser mas grande que eso tampoco)
#define HORIZONTAL_MATRIXES_QTY 1
#define VERTICAL_MATRIXES_QTY 1

/* Constantes que definen la intensidad de un LED para que se considere prendido o apagado */
#define ON 1 //intensidad de led cuando se prende
#define OFF 0 //intensidad de led cuando se apaga

/* Constantes relacionadas con las condiciones iniciales de la vibora*/
#define INIT_LENGTH 2 // Cantidad de LEDs que ocupa la vibora inicialmente
#define INIT_ROW_POS 2 // La cabeza de la vibora comienza en "el medio de la pantalla" (esquina superior derecha de la matriz de abajo a la izquierda)
#define INIT_COL_POS 3
#define INIT_WAIT 500 // Cantidad de ms que espera inicialmente hasta el siguiente movimiento de la vibora (velocidad)
#define WAIT_DECREASE_RATIO 0.5 // Cambio de velocidad de la vibora (newWait = INIT_WAIT * WAIT_DECREASE_RATIO)
#define SPEED_INCREASE_TIME 15000 // Tiempo en ms entre aumento de velocidades y aumento del tamanio de la vibora
#define INIT_DIR RIGHT

/* Constantes utilizadas para el almacenamiento de maximas puntuaciones*/
#define INITIAL_EEPROM_ADDRESS 0 // Posicion de la EEPROM desde la que se van a guardar los highscores
#define MAX_HIGHSCORES 3 // Cantidad maxima de maximas puntuaciones que se almacenan


/* Constantes relacionadas a los pins usados para conectar las componentes a la placa*/
#define LEFT_BUTTON_PIN 5 // Pin en la que se conecta la flechita izquierda
#define RIGHT_BUTTON_PIN 3 // Pin en la que se conecta la flechita derecha
#define DATA_PIN 11 // DIN
#define CLK_PIN 10 // Pin que se conecta al CLK de la matriz
#define CS_PIN 13 // Pin que se conecta al CS de la matriz

/* Traduce el boton apretado y devuelve la direccion en la que deberia seguir la vibora en base a ese boton, si no hubo boton apretado, sigue en la misma direccion que estaba */
Direction translateInput(Direction currentDir, HighscoreHandler * highscore, InputHandler input){
  if(Serial.available()){
    char c = Serial.read(); //Deberiamos agregar un while(Serial.available()) para que lea todo el buffer por si se insertaron mas de una letra? Nos quedamos con la primera o la ultima?
    
    switch(toupper(c)){
      case 'A': // Flecha izquierda
        return currentDir = (Direction) ((4 + currentDir - 1) % 4); // Giro en sentido antihorario (sumo 4 para que quede positivo, para que el % me de positivo)
      break;
      case 'D': // Flecha derecha
        return currentDir = (Direction) ((currentDir + 1) % 4);  // Giro en sentido horario
      break;
      case 'P': // 'P'rint
        printHighscores(*highscore);
        return currentDir;
        break;
      case 'R': // 'R'eset
        highscore->resetScores();
        return currentDir;
        break;
      default:
        return currentDir;
    }
  }
  else
    return currentDir;
}

/* Imprime los puntajes maximos que se almacenaron (en Serial por ahora) */
void printHighscores(HighscoreHandler highscore) {
  Serial.println("Puntajes Maximos:");
  if (highscore.getScoresAmmount() > 0) {
    for(int i=0; i<highscore.getScoresAmmount(); i++){
      Serial.print(i+1, DEC);
      Serial.print(". ");
      Serial.println((long)highscore.getScores()[i], DEC);
    }
  } else {
    Serial.println("No hay puntajes registrados");
  }
}

/* Prende todo el array de body en las matrices (hacer un clear antes de esto) */
void printWholeBody(Position body[MAX_LENGTH], int currentLength, int head, MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY]){
  for(int i=0; i<currentLength; i++){
    screen[0][0].setDot(body[(MAX_LENGTH + head-i) % MAX_LENGTH].y, body[(MAX_LENGTH + head-i) % MAX_LENGTH].x, ON); //Sumo MAX_LENGTH para que el indice quede positivo
  }
}

/* Prende el LED de la nueva cabeza de la vibora, apaga el LED de la vieja cola de la snake */
void printMove(Position newHead, Position oldTail, MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY]){
  screen[0][0].setDot(oldTail.y, oldTail.x, OFF);
  screen[0][0].setDot(newHead.y, newHead.x, ON);
}

/* Imprime la cruz cuando se pierde */
void printSkull(MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY]){
  byte sf[8]= {B00111100,B01000010,B10100101,B10011001,B10011001,B10100101,B01000010,B00111100};
  for(int i=0; i<MATRIX_COLUMNS * HORIZONTAL_MATRIXES_QTY; i++){
    screen[0][0].setColumn(7-i, sf[i]);
  }
}

/* Creacion de variables globales */
Snake snake(INIT_LENGTH, INIT_DIR, INIT_WAIT, INIT_ROW_POS, INIT_COL_POS, HORIZONTAL_MATRIXES_QTY * MATRIX_COLUMNS, VERTICAL_MATRIXES_QTY * MATRIX_ROWS); // La parte logica de la viborita
MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY] = {{MaxMatrix(DATA_PIN,CS_PIN,CLK_PIN,1)}}; //{{MaxMatrix(1,2,3,4), MaxMatrix(5,6,7,8)}, {MaxMatrix(11,13,10,1), MaxMatrix(13,14,15,0)}}; //0,0 = Arriba izquierda; 0,1 = Arriba derecha; 1,0 = Abajo izquierda; 1,1 = Arriba derecha
Direction input = INIT_DIR; // La direccion que empieza la vibora (lo inicializo en eso porque si no hay boton devuelve eso)
uint64_t lastMovedMillis = 0; // El tiempo (en ms) en el que se movio la vibora la ultima vez
uint64_t lastUpdatedMillis = 0; // El tiempo (en ms) en los que se agrando la vibora la ultima vez
bool enlarge = false; // True si hay que agrandar la vibora en el siguiente turno
HighscoreHandler highscore;
InputHandler inputHandler;

void setup() {
  // Inicializacion de botones
  inputHandler.registerPin(RIGHT_BUTTON_PIN, HIGH);
  inputHandler.registerPin(LEFT_BUTTON_PIN, HIGH);

  Serial.begin(115200);
  
  //Inicializacion de las matrices de LEDs
  for(int i=0; i<VERTICAL_MATRIXES_QTY; i++){
    for(int j=0; j<HORIZONTAL_MATRIXES_QTY; j++){
      screen[i][j].init();
      screen[i][j].setIntensity(ON);
      screen[i][j].clear();
    }
  }

  highscore = HighscoreHandler(INITIAL_EEPROM_ADDRESS, MAX_HIGHSCORES);
  printWholeBody(snake.getBody(), snake.getCurrentLength(), snake.getHead(), screen); 
  lastUpdatedMillis = lastMovedMillis = millis();
}

void loop() {
  if(millis() - lastUpdatedMillis > SPEED_INCREASE_TIME){ // Si tengo que aumentar la velocidad y agrandar la snake
    snake.setCurrentSpeed(snake.getCurrentSpeed() * WAIT_DECREASE_RATIO);
    enlarge = true;
    
    lastUpdatedMillis = millis();
  }

  if(snake.isAlive()){

    // Si hubo tecla, devuelve la direccion en la que deberia ir la vibora ahora
    input = translateInput(input, &highscore, inputHandler);
    
    if(millis() - lastMovedMillis > snake.getCurrentSpeed()){ //Si es tiempo de moverse
      Position oldTail = snake.getBody()[(MAX_LENGTH + snake.getHead() - (snake.getCurrentLength() - 1) ) % MAX_LENGTH]; // Guardo la cola de la vibora para apagar despues (La guardo por si estamos en el caso limite en que el head pise a la cola en el array de body)
      snake.moveSnake(input, enlarge);
    
      if(snake.isAlive()) // Si sigue viva despues de ese movimiento
        printMove(snake.getBody()[snake.getHead()], oldTail, screen); // Actualizo la pantalla
      else{
        printSkull(screen); // Imprimo la pantalla de GAME OVER
        snake.freeSnake(); // Libera solo el body, no es un destructor
        highscore.registerScore(snake.getAliveTime()); // Si fue un highscore, lo guarda
      }
      enlarge = false; // Si lo agrandaba, ya no lo tengo que agrandar
      
      lastMovedMillis = millis();
    }
  } else{
    input = translateInput(input, &highscore, inputHandler); // Por si quiero hacer un print o reset
  }
}
