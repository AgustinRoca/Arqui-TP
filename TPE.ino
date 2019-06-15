#include <MaxMatrix.h>

#include "HighscoreHandler.h"
#include "Direction.h"
#include "Position.h"
#include "Snake.h"
#include "InputHandler.h"
#include "SnakeFront.h"

#define __DEBUG__ 1

/* Constantes relacionadas con el tamanio/cantidad de las matrices*/
#define MATRIX_COLUMNS 8 // Cantidad de columnas de LEDs que tiene UNA matriz
#define MATRIX_ROWS 8 // Cantidad de filas de LEDs que tiene UNA matriz
#define MATRIX_SIZE (MATRIX_ROWS * MATRIX_COLUMNS) // Cantidad total de LEDs que tiene UNA matriz
#define MAX_LENGTH (MATRIX_SIZE * VERTICAL_MATRIXES_QTY * HORIZONTAL_MATRIXES_QTY) // Cantidad total de LEDs totals que tiene la pantalla completa (la vibora no puede ser mas grande que eso tampoco)
#define HORIZONTAL_MATRIXES_QTY 2
#define VERTICAL_MATRIXES_QTY 2

/* Constantes relacionadas con el tamanio del LCD */
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

#define MATRIX_CS_PIN 13 // Pin que se conecta al CS de la matriz
#define MATRIC_CLK_PIN 10 // Pin que se conecta al CLK de la matriz
#define MATRIX_DATA_PIN 11 // Pin que se conecta al DIN de la matriz

#define LCD_D0_PIN A3
#define LCD_D1_PIN A2
#define LCD_D2_PIN A1
#define LCD_D3_PIN A0
#define LCD_RS_PIN A5
#define LCD_ENABLE_PIN A4
#define LCD_CONTRAST_PIN 3
#define LCD_BRIGHTNESS_PIN 5

/* PROBABLEMENTE SE ELIMINE CUANDO HAYA BOTONES */
/* Limpia el serial */
void cleanSerial(){
  while(Serial.available()){
    Serial.read();
  }
}

SnakeFront snakeFront;

/* SE CAMBIA CUANDO HAYA BOTONES */
/* Traduce el boton apretado y devuelve la direccion en la que deberia seguir la vibora en base a ese boton, si no hubo boton apretado, sigue en la misma direccion que estaba */
Direction translateInput(Direction currentDir, HighscoreHandler * highscore, InputHandler input, MaxMatrix screen, int * intensity){
  if(Serial.available()){
    char c = Serial.read(); //Deberiamos agregar un while(Serial.available()) para que lea todo el buffer por si se insertaron mas de una letra? Nos quedamos con la primera o la ultima?
    cleanSerial();
    switch(toupper(c)){
      case 'A': // Flecha izquierda
        return currentDir = (Direction) ((4 + currentDir - 1) % 4); // Giro en sentido antihorario (sumo 4 para que quede positivo, para que el % me de positivo)
      case 'D': // Flecha derecha
        return currentDir = (Direction) ((currentDir + 1) % 4);  // Giro en sentido horario
      case 'P': // 'P'rint
        snakeFront.printHighscores();
        return currentDir;
      case 'R': // 'R'eset
        highscore->resetScores();
        return currentDir;
      case 'I':
        *intensity = Serial.read() - '0';
        snakeFront.setMatrixIntensity(*intensity);
        return currentDir;
      default:
        return currentDir;
    }
  }
  else
    return currentDir;
}

/* SE CAMBIA CUANDO HAYA BOTONES */
/* Lee la opcion del menu que se selecciono, y se ejecuta, si se le manda solo 4 o solo 5 se rompe (igual esto cambia cuando haya botones) */
void readMenuInput(Snake * snake, HighscoreHandler * highscore,  MaxMatrix * screen,int * intensity, uint64_t * lastUpdatedMillis, uint64_t * lastMovedMillis, Direction * input, double * waitTimeFactor, double * waitDecreaseRatioFactor){
  if(Serial.available()){
    char c = Serial.read(); //Deberiamos agregar un while(Serial.available()) para que lea todo el buffer por si se insertaron mas de una letra? Nos quedamos con la primera o la ultima?
    switch(toupper(c)){
      case '1': // Play
        snake->revive(INIT_LENGTH, INIT_DIR, INIT_WAIT, INIT_ROW_POS, INIT_COL_POS);
        screen->clear();
        snakeFront.printWholeBody();
        *lastUpdatedMillis = *lastMovedMillis = millis();
        *input = INIT_DIR;
      break;
      case '2': // Show Highscores
        snakeFront.printHighscores();
      break;
      case '3': // Reset Highscores
        highscore->resetScores();
        break;
      case '4': // Set intensity
        *intensity = Serial.read() - '0';
        snakeFront.setMatrixIntensity(*intensity);
        snakeFront.setLCDIntensity(*intensity);
        Serial.println(*intensity);
        break;
      case '5': // Dificulty
        *waitDecreaseRatioFactor = *waitTimeFactor = 1;
        int dificulty = Serial.read() - '0' - 1;
        for(int i=0; i<dificulty; i++){
          *waitDecreaseRatioFactor = (*waitTimeFactor -= DIFICULTY_INTERVAL);
        }
      break;
      case '6': // Set contrast
        uint8_t contrast = Serial.read() - '0';
        Serial.println(contrast);
        snakeFront.setLCDContrast(0);
        break;
    }
    cleanSerial();
    if(toupper(c) != '1' && c!='\r'){
      snakeFront.printMenu();
    }
  }
}

/* Creacion de variables globales */
Snake snake(INIT_LENGTH, INIT_DIR, INIT_WAIT, INIT_ROW_POS, INIT_COL_POS, HORIZONTAL_MATRIXES_QTY * MATRIX_COLUMNS, VERTICAL_MATRIXES_QTY * MATRIX_ROWS); // La parte logica de la viborita
MaxMatrix screen(MATRIX_DATA_PIN,MATRIX_CS_PIN,MATRIC_CLK_PIN,1);  //0,0 = Arriba izquierda; 0,1 = Arriba derecha; 1,0 = Abajo izquierda; 1,1 = Arriba derecha
Direction input = INIT_DIR; // La direccion que empieza la vibora (lo inicializo en eso porque si no hay boton devuelve eso)
uint64_t lastMovedMillis = 0; // El tiempo (en ms) en el que se movio la vibora la ultima vez
uint64_t lastUpdatedMillis = 0; // El tiempo (en ms) en los que se agrando la vibora la ultima vez
bool enlarge = false; // True si hay que agrandar la vibora en el siguiente turno
HighscoreHandler highscore;
InputHandler inputHandler;
int intensity = ON;
double waitTimeFactor = 1;
double waitDecreaseRatioFactor = 1;

LCD lcd(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D0_PIN, LCD_D1_PIN, LCD_D2_PIN, LCD_D3_PIN);

void setup() {
  // Inicializacion de botones
  // Los botones son Active Low ya que, si bien estan conectados
  // con una resistencia en formato PullDown, el Schmitt Trigger
  // utilizado invierte la senial.
  inputHandler.registerPin(LEFT_BUTTON_PIN, LOW);
  inputHandler.registerPin(RIGHT_BUTTON_PIN, LOW);
  inputHandler.registerPin(SELECT_BUTTON_PIN, LOW);

  // Inicializacion LCD
  lcd.begin(LCD_ROWS, LCD_COLS);
  lcd.clear();

  lcd.setRows(LCD_ROWS);
  lcd.setCols(LCD_COLS);
  lcd.setStartingCol(0);
  lcd.setContinuous(false);
  lcd.setCharacterTimeout(500);

  lcd.setContrastPin(LCD_CONTRAST_PIN);
  lcd.setBrightnessPin(LCD_BRIGHTNESS_PIN);

  lcd.setContrast(50);
  lcd.setBrightness(255);
  
  #ifdef __DEBUG__
  Serial.begin(115200);
  #endif

  // Inicializacion de las matrices de LEDs
  screen.init();
  screen.clear();

  // Inicializacion del manejador de puntajes maximos
  highscore = HighscoreHandler(INITIAL_EEPROM_ADDRESS, MAX_HIGHSCORES);
  
  // Inicializacion FrontEnd
  snakeFront.initialize(&highscore, &lcd, &screen, &snake, MATRIX_ROWS, MATRIX_ROWS, LCD_ROWS, LCD_COLS, MAX_LENGTH);
  snakeFront.setMatrixIntensity(intensity);
  
  // Se imprime el menu
  snakeFront.printMenu();
}

void loop() {
  if(snake.isAlive()){
    if(millis() - lastUpdatedMillis > SPEED_INCREASE_TIME * waitTimeFactor){ // Si tengo que aumentar la velocidad y agrandar la snake
      snake.setCurrentSpeed(snake.getCurrentSpeed() * WAIT_DECREASE_RATIO * waitDecreaseRatioFactor);
      enlarge = true;

      lastUpdatedMillis = millis();
    }
  
    // Si hubo tecla, devuelve la direccion en la que deberia ir la vibora ahora
    input = translateInput(input, &highscore, inputHandler, screen, &intensity);
    
    if(millis() - lastMovedMillis > snake.getCurrentSpeed()){ //Si es tiempo de moverse
      Position oldTail = snake.getBody()[(MAX_LENGTH + snake.getHead() - (snake.getCurrentLength() - 1) ) % MAX_LENGTH]; // Guardo la cola de la vibora para apagar despues (La guardo por si estamos en el caso limite en que el head pise a la cola en el array de body)
      snake.moveSnake(input, enlarge);
      enlarge = false; // Si lo agrandaba, ya no lo tengo que agrandar
    
      if(snake.isAlive()){ // Si sigue viva despues de ese movimiento
        snakeFront.printMove(snake.getBody()[snake.getHead()], oldTail); // Actualizo la pantalla 
      }
      else{
        snakeFront.printSkull(); // Imprimo la pantalla de GAME OVER
        snake.freeSnake(); // Libera solo el body, no es un destructor
        highscore.registerScore(snake.getAliveTime()); // Si fue un highscore, lo guarda
        delay(1000); // Para que se vea la pantalla de GAME OVER
        snakeFront.printMenu();
      }

      Serial.println("Alvie");
      lastMovedMillis = millis();
    }
  }
  else{
    readMenuInput(&snake, &highscore, &screen, &intensity, &lastUpdatedMillis, &lastMovedMillis, &input, &waitTimeFactor, &waitDecreaseRatioFactor);
  }
}
  
