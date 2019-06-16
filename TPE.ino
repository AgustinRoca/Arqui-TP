#include <MaxMatrix.h>

#include "HighscoreHandler.h"
#include "Direction.h"
#include "Position.h"
#include "Snake.h"
#include "InputHandler.h"
#include "LCD.h"

/* Constantes relacionadas con el tamanio/cantidad de las matrices*/
#define MATRIX_COLUMNS 8 // Cantidad de columnas de LEDs que tiene UNA matriz
#define MATRIX_ROWS 8 // Cantidad de filas de LEDs que tiene UNA matriz
#define MATRIX_SIZE (MATRIX_ROWS * MATRIX_COLUMNS) // Cantidad total de LEDs que tiene UNA matriz
#define MAX_LENGTH (MATRIX_SIZE * VERTICAL_MATRIXES_QTY * HORIZONTAL_MATRIXES_QTY) // Cantidad total de LEDs totals que tiene la pantalla completa (la vibora no puede ser mas grande que eso tampoco)
#define HORIZONTAL_MATRIXES_QTY 2
#define VERTICAL_MATRIXES_QTY 2

#define LCD_COLS 20
#define LCD_ROWS 4

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
#define DIFICULTY_INTERVAL 0.3

/* Constantes utilizadas para el almacenamiento de maximas puntuaciones*/
#define INITIAL_EEPROM_ADDRESS 0 // Posicion de la EEPROM desde la que se van a guardar los highscores
#define MAX_HIGHSCORES 3 // Cantidad maxima de maximas puntuaciones que se almacenan


/* Constantes relacionadas a los pins usados para conectar las componentes a la placa*/
#define LEFT_BUTTON_PIN 2 // Pin en la que se conecta la flechita izquierda
#define RIGHT_BUTTON_PIN 4 // Pin en la que se conecta la flechita derecha
#define SELECT_BUTTON_PIN 6
#define DATA_PIN 11 // Pin que se conecta al DIN de la matriz
#define CLK_PIN 10 // Pin que se conecta al CLK de la matriz
#define CS_PIN 13 // Pin que se conecta al CS de la matriz

#define CONTRAST_PIN 3
#define BRIGHTNESS_PIN 5
/* PROBABLEMENTE SE ELIMINE CUANDO HAYA BOTONES */
/* Limpia el serial */
void cleanSerial(){
  while(Serial.available()){
    Serial.read();
  }
}

/* SE CAMBIA CUANDO HAYA BOTONES */
/* Traduce el boton apretado y devuelve la direccion en la que deberia seguir la vibora en base a ese boton, si no hubo boton apretado, sigue en la misma direccion que estaba */
Direction translateInput(Direction currentDir, LCD * lcd, HighscoreHandler * highscore, InputHandler input, MaxMatrix screen, int * intensity){
  if(Serial.available()){
    char c = Serial.read(); //Deberiamos agregar un while(Serial.available()) para que lea todo el buffer por si se insertaron mas de una letra? Nos quedamos con la primera o la ultima?
    cleanSerial();
    switch(toupper(c)){
      case 'A': // Flecha izquierda
        return currentDir = (Direction) ((4 + currentDir - 1) % 4); // Giro en sentido antihorario (sumo 4 para que quede positivo, para que el % me de positivo)
      case 'D': // Flecha derecha
        return currentDir = (Direction) ((currentDir + 1) % 4);  // Giro en sentido horario
      case 'P': // 'P'rint
        printHighscores(*highscore, lcd);
        return currentDir;
      case 'R': // 'R'eset
        highscore->resetScores();
        return currentDir;
      case 'I':
        *intensity = Serial.read() - '0';
        screen.setIntensity(*intensity);
        return currentDir;
      default:
        return currentDir;
    }
  }
  else
    return currentDir;
}


/* SE CAMBIA CUANDO HAYA LCD */
/* Imprime los puntajes maximos que se almacenaron (en Serial por ahora) */
void printHighscores(HighscoreHandler highscore, LCD * lcd) {
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
  Serial.println("--------------------------------------------");
}

/* Traduce la posicion pos a pixeles de la screen y la imprime con intensidad intensity,si se le manda una posicion valida la ignora */
void setDotInScreen(Position pos, MaxMatrix * screen, int intensity){
  if(pos.x < MATRIX_COLUMNS){ // Matrices izquierdas
    if(pos.y < MATRIX_ROWS){ // Matriz abajo-izquierda (1era matriz de la cascada)
      screen->setDot(7-pos.y, 7-pos.x, intensity);
    }
    else{ // Matriz arriba-izquierda (3era matriz de la cascada)
      screen->setDot(7-(pos.y % MATRIX_ROWS), 7-(pos.x % MATRIX_COLUMNS) + 2*MATRIX_COLUMNS, intensity);
    }
  }
  else{ // Matrices Derechas
    if(pos.y < MATRIX_ROWS){ // Matriz abajo-derecha (2da matriz de la cascada). Como esta invertida se le saca el 7- a las posiciones
      screen->setDot(pos.y, (pos.x % MATRIX_COLUMNS) + MATRIX_COLUMNS, intensity);
    }
    else{ // Matriz arriba-derecha (4ta matriz de la cascada). Como esta invertida se le saca el 7- a las posiciones
      screen->setDot(pos.y % MATRIX_ROWS, (pos.x % MATRIX_COLUMNS) + 3*MATRIX_COLUMNS, intensity);
    }
  }
}

/* Prende todo el array de body en las matrices (asume que la pantalla esta limpia antes) */
void printWholeBody(Position body[MAX_LENGTH], int currentLength, int head, MaxMatrix * screen, int intensity){
  for(int i=0; i<currentLength; i++){
    setDotInScreen(body[(MAX_LENGTH + head-i)%MAX_LENGTH], screen, intensity);
  }
}

/* Prende el LED de la nueva cabeza de la vibora, apaga el LED de la vieja cola de la vibora */
void printMove(Position newHead, Position oldTail, MaxMatrix * screen, int intensity){
  setDotInScreen(oldTail, screen, OFF);
  setDotInScreen(newHead, screen, intensity);
}

/* Imprime la cruz cuando se pierde */
void printSkull(MaxMatrix * screen, LCD * lcd){ //TODO: ACTUALIZAR PARA 4 MATRICES
  byte skull1[8]= {B00011001,B00001111,B00001111,B00000011,B00110010,B00010000,B00000000,B00000000}; //Parte que deberia ir en la matriz abajo-izquierda
  byte skull2[8]= {B00000000,B00000000,B00100000,B01100101,B00000111,B00011110,B00011111,B00110011}; //Parte que deberia ir en la matriz abajo-derecha
  byte skull3[8]= {B00000000,B00010000,B00110000,B00000111,B00001111,B00001111,B00011100,B00011000}; //Parte que deberia ir en la 3era matriz arriba-izquierda
  byte skull4[8]= {B00110001,B00111001,B00011111,B00011111,B00001111,B01100000,B00100000,B00000000}; //Parte que deberia ir en la 4ta matriz arriba-derecha
  for(int i=0; i<8; i++){
    screen->setColumn(i+2*MATRIX_COLUMNS, skull3[i]);
    screen->setColumn(i, skull1[i]);
  }
  for(int i=0; i<8; i++){
    screen->setColumn(i+3*MATRIX_COLUMNS, skull4[i]);
    screen->setColumn(i+MATRIX_COLUMNS, skull2[i]);
  }
}


/* SE CAMBIA CUANDO HAYA LCD */
void printMenu(HighscoreHandler * highscore, int * intensity, LCD * lcd){
  Serial.println("\nSNAKE\n");
  Serial.println("1. Play");
  Serial.println("2. Show Highscores");
  Serial.println("3. Reset Highscores");
  Serial.println("4x. Set Intensity in x (x = (1..8))"); //Ejemplo: 41 = set Intensity in 1
  Serial.println("5x. Dificulty in x ( x = 1, 2 or 3)"); //Ejemplo: 53 = set Dificulty in 3
  Serial.println("--------------------------------------------");
}


/* SE CAMBIA CUANDO HAYA BOTONES */
/* Lee la opcion del menu que se selecciono, y se ejecuta, si se le manda solo 4 o solo 5 se rompe (igual esto cambia cuando haya botones) */
void readMenuInput(Snake * snake, InputHandler * inputHandler, LCD * lcd, HighscoreHandler * highscore,  MaxMatrix * screen,int * intensity, uint64_t * lastUpdatedMillis, uint64_t * lastMovedMillis, Direction * input, double * waitTimeFactor, double * waitDecreaseRatioFactor){
  if(Serial.available()){
    char c = Serial.read(); //Deberiamos agregar un while(Serial.available()) para que lea todo el buffer por si se insertaron mas de una letra? Nos quedamos con la primera o la ultima?
    switch(toupper(c)){
      case '1': // Play
        snake->revive(INIT_LENGTH, INIT_DIR, INIT_WAIT, INIT_ROW_POS, INIT_COL_POS);
        screen->clear();
        printWholeBody(snake->getBody(), snake->getCurrentLength(), snake->getHead(), screen, *intensity);
        *lastUpdatedMillis = *lastMovedMillis = millis();
        *input = INIT_DIR;
      break;
      case '2': // Show Highscores
        printHighscores(*highscore, lcd);
        delay(2000);
      break;
      case '3': // Reset Highscores
        highscore->resetScores();
        break;
      case '4': // Set intensity
        *intensity = Serial.read() - '0';
        screen->setIntensity(*intensity);
        break;
      case '5': // Dificulty
        *waitDecreaseRatioFactor = *waitTimeFactor = 1;
        int dificulty = Serial.read() - '0' - 1;
        for(int i=0; i<dificulty; i++){
          *waitDecreaseRatioFactor = (*waitTimeFactor -= DIFICULTY_INTERVAL);
        }
      break;
    }
    cleanSerial();
    if(toupper(c) != '1' && c!='\r'){
      printMenu(highscore, intensity, lcd);
    }
  }
}

/* Creacion de variables globales */
Snake snake(INIT_LENGTH, INIT_DIR, INIT_WAIT, INIT_ROW_POS, INIT_COL_POS, HORIZONTAL_MATRIXES_QTY * MATRIX_COLUMNS, VERTICAL_MATRIXES_QTY * MATRIX_ROWS); // La parte logica de la viborita
MaxMatrix screen(DATA_PIN,CS_PIN,CLK_PIN,1);  //0,0 = Arriba izquierda; 0,1 = Arriba derecha; 1,0 = Abajo izquierda; 1,1 = Arriba derecha
Direction input = INIT_DIR; // La direccion que empieza la vibora (lo inicializo en eso porque si no hay boton devuelve eso)
uint64_t lastMovedMillis = 0; // El tiempo (en ms) en el que se movio la vibora la ultima vez
uint64_t lastUpdatedMillis = 0; // El tiempo (en ms) en los que se agrando la vibora la ultima vez
bool enlarge = false; // True si hay que agrandar la vibora en el siguiente turno
HighscoreHandler highscore;
InputHandler inputHandler;
int intensity = ON;
double waitTimeFactor = 1;
double waitDecreaseRatioFactor = 1;
LCD lcd(A5, A4, A3, A2, A1, A0);

void setup() {
  // Inicializacion de botones
  inputHandler.registerPin(RIGHT_BUTTON_PIN, LOW);
  inputHandler.registerPin(LEFT_BUTTON_PIN, LOW);
  inputHandler.registerPin(SELECT_BUTTON_PIN, LOW);

  //INICIALIZACION DE LCD
  lcd.begin(LCD_ROWS, LCD_COLS);
  lcd.clear();
  lcd.setContrastPin(CONTRAST_PIN);
  lcd.setBrightnessPin(BRIGHTNESS_PIN);
  lcd.setRows(LCD_ROWS);
  lcd.setCols(LCD_COLS);
  lcd.setStartingCol(0);
  lcd.setContinuous(false);
  lcd.setCharacterTimeout(500);
  lcd.setContrast(50);
  lcd.setBrightness(255);
  
  /* SE ELIMINA CUANDO HAYA BOTONES */
  Serial.begin(115200);
  
  //Inicializacion de las matrices de LEDs
  screen.init();
  screen.setIntensity(intensity);
  screen.clear();

  //Inicializacion del manejador de puntajes maximos
  highscore.initialize(INITIAL_EEPROM_ADDRESS, MAX_HIGHSCORES);

  //Se imprime el menu
  printMenu(&highscore, &intensity, &lcd);
}

void loop() {
  if(snake.isAlive()){
    if(millis() - lastUpdatedMillis > SPEED_INCREASE_TIME * waitTimeFactor){ // Si tengo que aumentar la velocidad y agrandar la snake
      snake.setCurrentSpeed(snake.getCurrentSpeed() * WAIT_DECREASE_RATIO * waitDecreaseRatioFactor);
      enlarge = true;
      
      lastUpdatedMillis = millis();
    }
  
    // Si hubo tecla, devuelve la direccion en la que deberia ir la vibora ahora
    input = translateInput(input, &lcd, &highscore, inputHandler, screen, &intensity);
    
    if(millis() - lastMovedMillis > snake.getCurrentSpeed()){ //Si es tiempo de moverse
      Position oldTail = snake.getBody()[(MAX_LENGTH + snake.getHead() - (snake.getCurrentLength() - 1) ) % MAX_LENGTH]; // Guardo la cola de la vibora para apagar despues (La guardo por si estamos en el caso limite en que el head pise a la cola en el array de body)
      snake.moveSnake(input, enlarge);
      enlarge = false; // Si lo agrandaba, ya no lo tengo que agrandar
    
      if(snake.isAlive()){ // Si sigue viva despues de ese movimiento
        printMove(snake.getBody()[snake.getHead()], oldTail, &screen, intensity); // Actualizo la pantalla 
      }
      else{
        printSkull(&screen, &lcd); // Imprimo la pantalla de GAME OVER
        snake.freeSnake(); // Libera solo el body, no es un destructor
        highscore.registerScore(snake.getAliveTime()); // Si fue un highscore, lo guarda
        delay(1000); // Para que se vea la pantalla de GAME OVER
        printMenu(&highscore, &intensity, &lcd);
      }
      
      lastMovedMillis = millis();
    }
  }
  else{
    readMenuInput(&snake, &inputHandler, &lcd, &highscore, &screen, &intensity, &lastUpdatedMillis, &lastMovedMillis, &input, &waitTimeFactor, &waitDecreaseRatioFactor);
  }
}
  
