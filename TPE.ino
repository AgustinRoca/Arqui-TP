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
#define ON 8 //intensidad de led cuando se prende
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
#define DIFICULTY_INTERVAL 0.3

/* Traduce el boton apretado y devuelve la direccion en la que deberia seguir la vibora en base a ese boton, si no hubo boton apretado, sigue en la misma direccion que estaba */
Direction translateInput(Direction currentDir, HighscoreHandler * highscore, InputHandler input, MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY], int * intensity){
  if(Serial.available()){
    char c = Serial.read(); //Deberiamos agregar un while(Serial.available()) para que lea todo el buffer por si se insertaron mas de una letra? Nos quedamos con la primera o la ultima?
    
    switch(toupper(c)){
      case 'A': // Flecha izquierda
        return currentDir = (Direction) ((4 + currentDir - 1) % 4); // Giro en sentido antihorario (sumo 4 para que quede positivo, para que el % me de positivo)
      case 'D': // Flecha derecha
        return currentDir = (Direction) ((currentDir + 1) % 4);  // Giro en sentido horario
      case 'P': // 'P'rint
        printHighscores(*highscore);
        return currentDir;
      case 'R': // 'R'eset
        highscore->resetScores();
        return currentDir;
      case 'I':
        *intensity = Serial.read() - '0';
        screen[0][0].setIntensity(*intensity);
        return currentDir;
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
void printWholeBody(Position body[MAX_LENGTH], int currentLength, int head, MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY], int intensity){
  for(int i=0; i<currentLength; i++){
    screen[0][0].setDot(body[(MAX_LENGTH + head-i) % MAX_LENGTH].y, body[(MAX_LENGTH + head-i) % MAX_LENGTH].x, intensity); //Sumo MAX_LENGTH para que el indice quede positivo
  }
}

/* Prende el LED de la nueva cabeza de la vibora, apaga el LED de la vieja cola de la snake */
void printMove(Position newHead, Position oldTail, MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY], int intensity){
  screen[0][0].setDot(oldTail.y, oldTail.x, OFF);
  screen[0][0].setDot(newHead.y, newHead.x, intensity);
}

/* Imprime la cruz cuando se pierde */
void printSkull(MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY]){
  byte sf[8]= {B00111100,B01000010,B10100101,B10011001,B10011001,B10100101,B01000010,B00111100};
  for(int i=0; i<MATRIX_COLUMNS * HORIZONTAL_MATRIXES_QTY; i++){
    screen[0][0].setColumn(7-i, sf[i]);
  }
}

void printMenu(HighscoreHandler * highscore,  MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY],int * intensity){
  Serial.println("SNAKE\n");
  Serial.println("1. Play");
  Serial.println("2. Show Highscores");
  Serial.println("3. Reset Highscores");
  Serial.println("4x. Set Intensity in x (x = (1..8))");
  Serial.println("5x. Dificulty in x ( x = 1, 2 or 3)");
}

void readMenuInput(Snake * snake, HighscoreHandler * highscore,  MaxMatrix screen[VERTICAL_MATRIXES_QTY][HORIZONTAL_MATRIXES_QTY],int * intensity, uint64_t * lastUpdatedMillis, uint64_t * lastMovedMillis, Direction * input, double * waitTimeFactor, double * waitDecreaseRatioFactor){
  if(Serial.available()){
    char c = Serial.read(); //Deberiamos agregar un while(Serial.available()) para que lea todo el buffer por si se insertaron mas de una letra? Nos quedamos con la primera o la ultima?
    
    switch(toupper(c)){
      case '1': // Play
        snake->revive( INIT_LENGTH, INIT_DIR, INIT_WAIT, INIT_ROW_POS, INIT_COL_POS);
        screen[0][0].clear();
        printWholeBody(snake->getBody(), snake->getCurrentLength(), snake->getHead(), screen, *intensity);
        *lastUpdatedMillis = *lastMovedMillis = millis();
        *input = INIT_DIR;
      break;
      case '2': // Show Highscores
        //system("clear");
        printHighscores(*highscore);
        delay(5000);
      break;
      case '3': // Reset Highscores
        highscore->resetScores();
        break;
      case '4': // Set intensity
        *intensity = Serial.read() - '0';
        screen[0][0].setIntensity(*intensity);
        break;
      case '5': // Dificulty
        *waitDecreaseRatioFactor = *waitTimeFactor = 1;
        int dificulty = Serial.read() - '0' - 1;
        for(int i=0; i<dificulty; i++){
          *waitDecreaseRatioFactor = (*waitTimeFactor -= DIFICULTY_INTERVAL);
        }
      break;
    }
    if(toupper(c) != '1' && c!='\r'){
      //system("clear");
      printMenu(highscore, screen, intensity);
    }
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
int intensity = ON;
double waitTimeFactor = 1;
double waitDecreaseRatioFactor = 1;

void setup() {
  // Inicializacion de botones
  inputHandler.registerPin(RIGHT_BUTTON_PIN, HIGH);
  inputHandler.registerPin(LEFT_BUTTON_PIN, HIGH);

  Serial.begin(115200);
  
  //Inicializacion de las matrices de LEDs
  for(int i=0; i<VERTICAL_MATRIXES_QTY; i++){
    for(int j=0; j<HORIZONTAL_MATRIXES_QTY; j++){
      screen[i][j].init();
      screen[i][j].setIntensity(intensity);
      screen[i][j].clear();
    }
  }

  highscore = HighscoreHandler(INITIAL_EEPROM_ADDRESS, MAX_HIGHSCORES);
  lastUpdatedMillis = lastMovedMillis = millis();
  printMenu(&highscore, screen, &intensity);
}

void loop() {
  if(snake.isAlive()){
    if(millis() - lastUpdatedMillis > SPEED_INCREASE_TIME * waitTimeFactor){ // Si tengo que aumentar la velocidad y agrandar la snake
      snake.setCurrentSpeed(snake.getCurrentSpeed() * WAIT_DECREASE_RATIO * waitDecreaseRatioFactor);
      enlarge = true;
      
      lastUpdatedMillis = millis();
    }
  
    if(snake.isAlive()){
  
      // Si hubo tecla, devuelve la direccion en la que deberia ir la vibora ahora
      input = translateInput(input, &highscore, inputHandler, screen, &intensity);
      
      if(millis() - lastMovedMillis > snake.getCurrentSpeed()){ //Si es tiempo de moverse
        Position oldTail = snake.getBody()[(MAX_LENGTH + snake.getHead() - (snake.getCurrentLength() - 1) ) % MAX_LENGTH]; // Guardo la cola de la vibora para apagar despues (La guardo por si estamos en el caso limite en que el head pise a la cola en el array de body)
        snake.moveSnake(input, enlarge);
      
        if(snake.isAlive()) // Si sigue viva despues de ese movimiento
          printMove(snake.getBody()[snake.getHead()], oldTail, screen, intensity); // Actualizo la pantalla
        else{
          printSkull(screen); // Imprimo la pantalla de GAME OVER
          snake.freeSnake(); // Libera solo el body, no es un destructor
          highscore.registerScore(snake.getAliveTime()); // Si fue un highscore, lo guarda
          delay(1000);
          printMenu(&highscore, screen, &intensity);
        }
        enlarge = false; // Si lo agrandaba, ya no lo tengo que agrandar
        
        lastMovedMillis = millis();
      }
    } else{
      input = translateInput(input, &highscore, inputHandler, screen, &intensity); // Por si quiero hacer un print o reset
    }
  }
  else{
    readMenuInput(&snake, &highscore, screen, &intensity, &lastUpdatedMillis, &lastMovedMillis, &input, &waitTimeFactor, &waitDecreaseRatioFactor);
  }
}
  
