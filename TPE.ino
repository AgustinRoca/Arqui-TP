#include <MaxMatrix.h>

#include "HighscoreHandler.h"
#include "Direction.h"
#include "Position.h"
#include "Snake.h"javascript:Announcement.searchAnnouncements('3')
#include "InputHandler.h"
#include "LCD.h"

/* Constantes relacionadas con el tamanio/cantidad de las matrices*/
#define MATRIX_COLUMNS 8 // Cantidad de columnas de LEDs que tiene UNA matriz
#define MATRIX_ROWS 8 // Cantidad de filas de LEDs que tiene UNA matriz
#define MATRIX_SIZE (MATRIX_ROWS * MATRIX_COLUMNS) // Cantidad total de LEDs que tiene UNA matriz
#define HORIZONTAL_MATRIXES_QTY 2
#define VERTICAL_MATRIXES_QTY 2
#define MATRIX_QTY (VERTICAL_MATRIXES_QTY * HORIZONTAL_MATRIXES_QTY)
#define MAX_LENGTH (MATRIX_SIZE * MATRIX_QTY) // Cantidad total de LEDs totals que tiene la pantalla completa (la vibora no puede ser mas grande que eso tampoco)

#define LCD_COLS 20
#define LCD_ROWS 4
#define DEFAULT_CONTRAST 150

/* Constantes que definen la intensidad de un LED para que se considere prendido o apagado */
#define DEFAULT_LED_INTENSITY 8 //intensidad de led cuando se prende
#define GET_LCD_INTENSITY(x) (x > 0 ? x * 32 - 1 : 0)
#define OFF 0 //intensidad de led cuando se apaga

/* Constantes relacionadas con las condiciones iniciales de la vibora*/
#define INIT_LENGTH 2 // Cantidad de LEDs que ocupa la vibora inicialmente
#define INIT_ROW_POS 2 // La cabeza de la vibora comienza en "el medio de la pantalla" (esquina superior derecha de la matriz de abajo a la izquierda)
#define INIT_COL_POS 3
#define INIT_WAIT 500 // Cantidad de ms que espera inicialmente hasta el siguiente movimiento de la vibora (velocidad)
#define WAIT_DECREASE_RATIO 0.5 // Cambio de velocidad de la vibora (newWait = INIT_WAIT * WAIT_DECREASE_RATIO)
#define SPEED_INCREASE_TIME 15000 // Tiempo en ms entre aumento de velocidades y aumento del tamanio de la vibora
#define INIT_DIR RIGHT
#define DIFFICULTY_INTERVAL 0.3
#define DIFFICULTY_LEVELS 3

/* Constantes utilizadas para el almacenamiento de maximas puntuaciones*/
#define INITIAL_EEPROM_ADDRESS 0 // Posicion de la EEPROM desde la que se van a guardar los highscores
#define MAX_HIGHSCORES 3 // Cantidad maxima de maximas puntuaciones que se almacenan


/* Constantes relacionadas a los pins usados para conectar las componentes a la placa*/
#define LEFT_BUTTON_PIN 9 // Pin en la que se conecta la flechita izquierda
#define RIGHT_BUTTON_PIN 4 // Pin en la que se conecta la flechita derecha
#define SELECT_BUTTON_PIN 7
#define DATA_PIN 2 // Pin que se conecta al DIN de la matriz
#define CLK_PIN 3 // Pin que se conecta al CLK de la matriz
#define CS_PIN 8 // Pin que se conecta al CS de la matriz

#define CONTRAST_PIN 10
#define BRIGHTNESS_PIN 5

#define LCD_CHARACTER_TIMEOUT 1000

#define NO_BUTTON -1


/* PROBABLEMENTE SE ELIMINE CUANDO HAYA BOTONES */
/* Limpia el serial */
void cleanSerial(); 

/* SE CAMBIA CUANDO HAYA BOTONES */
/* Traduce el boton apretado y devuelve la direccion en la que deberia seguir la vibora en base a ese boton, si no hubo boton apretado, sigue en la misma direccion que estaba */
Direction translateInput(Direction currentDir, LCD * lcd, HighscoreHandler * highscore, InputHandler input, MaxMatrix screen, uint8_t * intensity);

/* SE CAMBIA CUANDO HAYA LCD */
/* Imprime los puntajes maximos que se almacenaron (en Serial por ahora) */
void printHighscores(HighscoreHandler highscore, LCD * lcd);

/* Traduce la posicion pos a pixeles de la screen y la imprime con intensidad intensity,si se le manda una posicion valida la ignora */
void setDotInScreen(Position pos, MaxMatrix * screen, uint8_t intensity);

/* Prende todo el array de body en las matrices (asume que la pantalla esta limpia antes) */
void printWholeBody(Position body[MAX_LENGTH], int currentLength, int head, MaxMatrix * screen, uint8_t intensity);

/* Prende el LED de la nueva cabeza de la vibora, apaga el LED de la vieja cola de la vibora */
void printMove(Position newHead, Position oldTail, MaxMatrix * screen, int intensity);

/* Imprime la cruz cuando se pierde */
void printSkull(MaxMatrix * screen, LCD * lcd, uint32_t score);

/* SE CAMBIA CUANDO HAYA LCD */
void printMenu(HighscoreHandler * highscore, uint8_t * intensity, LCD * lcd);


/* SE CAMBIA CUANDO HAYA BOTONES */
/* Lee la opcion del menu que se selecciono, y se ejecuta, si se le manda solo 4 o solo 5 se rompe (igual esto cambia cuando haya botones) */
void readMenuInput(Snake * snake, InputHandler * inputHandler, LCD * lcd, HighscoreHandler * highscore,  MaxMatrix * screen,uint8_t * intensity, uint64_t * lastUpdatedMillis, uint64_t * lastMovedMillis, Direction * input, double * waitTimeFactor, double * waitDecreaseRatioFactor, uint8_t * lcdIntensity);

/* Creacion de variables globales */
Snake snake(INIT_LENGTH, INIT_DIR, INIT_WAIT, INIT_ROW_POS, INIT_COL_POS, HORIZONTAL_MATRIXES_QTY * MATRIX_COLUMNS, VERTICAL_MATRIXES_QTY * MATRIX_ROWS); // La parte logica de la viborita
InputHandler inputHandler;
HighscoreHandler highscore;

bool enlarge = false; // True si hay que agrandar la vibora en el siguiente turno
Direction input = INIT_DIR; // La direccion que empieza la vibora (lo inicializo en eso porque si no hay boton devuelve eso)
uint64_t lastMovedMillis = 0; // El tiempo (en ms) en el que se movio la vibora la ultima vez
uint64_t lastUpdatedMillis = 0; // El tiempo (en ms) en los que se agrando la vibora la ultima vez
double waitTimeFactor = 1;
double waitDecreaseRatioFactor = 1;
int16_t lastButtonRead = NO_BUTTON;

LCD lcd(A5, A4, A3, A2, A1, A0, LCD_COLS, LCD_ROWS);
MaxMatrix screen(DATA_PIN,CS_PIN,CLK_PIN, MATRIX_QTY);
uint8_t contrast = DEFAULT_CONTRAST;
uint8_t intensity = DEFAULT_LED_INTENSITY;
uint8_t lcdIntensity = GET_LCD_INTENSITY(DEFAULT_LED_INTENSITY);

void setup() {
  //Serial1.begin(230400);
  Serial.begin(115200);

  // Inicializacion de botones
  inputHandler.registerPin(RIGHT_BUTTON_PIN, LOW);
  inputHandler.registerPin(LEFT_BUTTON_PIN, LOW);
  inputHandler.registerPin(SELECT_BUTTON_PIN, LOW);

  //INICIALIZACION DE LCD
  lcd.begin();

  lcd.setContrastPin(CONTRAST_PIN);
  lcd.setBrightnessPin(BRIGHTNESS_PIN);
  lcd.setCharacterTimeout(LCD_CHARACTER_TIMEOUT);

  lcd.setContrast(DEFAULT_CONTRAST);
  lcd.setBrightness(lcdIntensity);
  
  /* SE ELIMINA CUANDO HAYA BOTONES */
  
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
  //Serial1.println("ASD");
  if(snake.isAlive()){
    if(millis() - lastUpdatedMillis > SPEED_INCREASE_TIME * waitTimeFactor){ // Si tengo que aumentar la velocidad y agrandar la snake
      snake.setCurrentSpeed(snake.getCurrentSpeed() * WAIT_DECREASE_RATIO * waitDecreaseRatioFactor);
      enlarge = true;
      
      lastUpdatedMillis = millis();
    }
  
    // Si hubo tecla, devuelve la direccion en la que deberia ir la vibora ahora
    input = translateInput(input, &lcd, &highscore, inputHandler, screen, &intensity);
    
    if(millis() - lastMovedMillis > snake.getCurrentSpeed()){ //Si es tiempo de moverse
      char stringBuffer[20] = {0};
      sprintf(stringBuffer, "%ld", snake.getAliveTime());
      lcd.clear();
      lcd.addText(stringBuffer, 0);
      Position oldTail = snake.getBody()[(MAX_LENGTH + snake.getHead() - (snake.getCurrentLength() - 1) ) % MAX_LENGTH]; // Guardo la cola de la vibora para apagar despues (La guardo por si estamos en el caso limite en que el head pise a la cola en el array de body)
      snake.moveSnake(input, enlarge);
      enlarge = false; // Si lo agrandaba, ya no lo tengo que agrandar
    
      if(snake.isAlive()){ // Si sigue viva despues de ese movimiento
        printMove(snake.getBody()[snake.getHead()], oldTail, &screen, intensity); // Actualizo la pantalla 
      }
      else{
        printSkull(&screen, &lcd, snake.getAliveTime()); // Imprimo la pantalla de GAME OVER
        snake.freeSnake(); // Libera solo el body, no es un destructor
        highscore.registerScore(snake.getAliveTime()); // Si fue un highscore, lo guarda
        delay(1000); // Para que se vea la pantalla de GAME OVER
        screen.clear();
        printMenu(&highscore, &intensity, &lcd);
      }
      
      lastMovedMillis = millis();
    }
  }
  else{
    readMenuInput(&snake, &inputHandler, &lcd, &highscore, &screen, &intensity, &lastUpdatedMillis, &lastMovedMillis, &input, &waitTimeFactor, &waitDecreaseRatioFactor, &lcdIntensity, &lastButtonRead);
  }
  
  lcd.refresh();
}

void cleanSerial(){
  while(Serial.available()){
    Serial.read();
  }
}

/* SE CAMBIA CUANDO HAYA BOTONES */
/* Traduce el boton apretado y devuelve la direccion en la que deberia seguir la vibora en base a ese boton, si no hubo boton apretado, sigue en la misma direccion que estaba */
Direction translateInput(Direction currentDir, LCD * lcd, HighscoreHandler * highscore, InputHandler input, MaxMatrix screen, uint8_t * intensity){
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
  else {
    const uint8_t* activeButtons = input.readInputs();
    uint8_t activeButtonsCount = input.getActivePinsCount();
    if (activeButtonsCount > 0) {
      if (activeButtons[0] == LEFT_BUTTON_PIN) {
        currentDir = (Direction) ((4 + currentDir - 1) % 4);
      } else if (activeButtons[0] == RIGHT_BUTTON_PIN) {
        currentDir = (Direction) ((currentDir + 1) % 4);
      }
    }
        
    return currentDir;
  }
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

  // lcd->setCursor(0,0);
  lcd->addText("Puntajes Maximos:",0);
  if (highscore.getScoresAmmount() > 0) {
    for(int i=0; i<highscore.getScoresAmmount(); i++){
      char * buff = (char *)malloc(LCD_COLS * sizeof(buff));
      sprintf(buff, "%d. %ld", i+1, (long)highscore.getScores()[i]);
      lcd->addText(buff, 0);
      free(buff);
    }
  } else {
    lcd->addText("No hay puntajes registrados",0);
  }
  
}

/* Traduce la posicion pos a pixeles de la screen y la imprime con intensidad intensity,si se le manda una posicion valida la ignora */
void setDotInScreen(Position pos, MaxMatrix * screen, uint8_t intensity){
  if(pos.x < MATRIX_COLUMNS){ // Matrices izquierdas
    if(pos.y < MATRIX_ROWS){ // Matriz abajo-izquierda (1era matriz de la cascada)
      screen->setDot(7-pos.y, 7-pos.x, intensity);
    }
    else{ // Matriz arriba-izquierda (3era matriz de la cascada)
      screen->setDot(7-(pos.y % MATRIX_ROWS) + 2*MATRIX_COLUMNS, 7-(pos.x % MATRIX_COLUMNS), intensity);
    }
  }
  else{ // Matrices Derechas
    if(pos.y < MATRIX_ROWS){ // Matriz abajo-derecha (2da matriz de la cascada). Como esta invertida se le saca el 7- a las posiciones
      screen->setDot(pos.y + MATRIX_COLUMNS, (pos.x % MATRIX_COLUMNS) , intensity);
    }
    else{ // Matriz arriba-derecha (4ta matriz de la cascada). Como esta invertida se le saca el 7- a las posiciones
      screen->setDot(pos.y % MATRIX_ROWS + 3*MATRIX_COLUMNS, (pos.x % MATRIX_COLUMNS) , intensity);
    }
  }
}

/* Prende todo el array de body en las matrices (asume que la pantalla esta limpia antes) */
void printWholeBody(Position body[MAX_LENGTH], int currentLength, int head, MaxMatrix * screen, uint8_t intensity){
  for(int i=0; i<currentLength; i++){
    setDotInScreen(body[(MAX_LENGTH + head-i)%MAX_LENGTH], screen, intensity);
  }
}

/* Prende el LED de la nueva cabeza de la vibora, apaga el LED de la vieja cola de la vibora */
void printMove(Position newHead, Position oldTail, MaxMatrix * screen, uint8_t intensity){
  setDotInScreen(oldTail, screen, OFF);
  setDotInScreen(newHead, screen, intensity);
}

/* Imprime la cruz cuando se pierde */
void printSkull(MaxMatrix * screen, LCD * lcd, uint32_t score){ //TODO: ACTUALIZAR PARA 4 MATRICES
  byte skull1[8]= {B00011001,B00001111,B00001111,B00000011,B00110010,B00010000,B00000000,B00000000}; //Parte que deberia ir en la matriz abajo-izquierda
  byte skull2[8]= {B00000000,B00000000,B00100000,B01100101,B00000111,B00011110,B00011111,B00110011}; //Parte que deberia ir en la matriz abajo-derecha
  byte skull3[8]= {B00000000,B00010000,B00110000,B00000111,B00001111,B00001111,B00011100,B00011000}; //Parte que deberia ir en la 3era matriz arriba-izquierda
  byte skull4[8]= {B00110001,B00111001,B00011111,B00011111,B00001111,B01100000,B00100000,B00000000}; //Parte que deberia ir en la 4ta matriz arriba-derecha\
  
  char stringBuffer[30] = {0};
  sprintf(stringBuffer, "Score: %ld", score);
  lcd->clear();
  lcd->addText("GAME OVER", 0);
  lcd->addText(stringBuffer, 0);
  
  for(int i=0; i<8; i++){
    screen->setColumn(7-i, skull1[7-i]);
    screen->setColumn(i+MATRIX_COLUMNS, skull2[i]);
  }
  for(int i=0; i<8; i++){
    screen->setColumn((7-i)+2*MATRIX_COLUMNS, skull3[7-i]);
    screen->setColumn(i+3*MATRIX_COLUMNS, skull4[i]);
  }
}


/* SE CAMBIA CUANDO HAYA LCD */
void printMenu(HighscoreHandler * highscore, uint8_t * intensity, LCD * lcd){
  Serial.println("\nSNAKE\n");
  Serial.println("1. Play");
  Serial.println("2. Show Highscores");
  Serial.println("3. Reset Highscores");
  Serial.println("4x. Set Intensity in x (x = (1..8))"); //Ejemplo: 41 = set Intensity in 1
  Serial.println("5x. Difficulty in x ( x = 1, 2 or 3)"); //Ejemplo: 53 = set Difficulty in 3
  Serial.println("--------------------------------------------");

  lcd->clear();
  lcd->addText("1. Play", 0);
  lcd->addText("2. Show Highscores", 0);
  lcd->addText("3. Reset Highscores", 0);
  lcd->addText("4. Intensity", 0);
  lcd->addText("5. Difficulty", 0);
  lcd->setShowCursor(true);
}


/* SE CAMBIA CUANDO HAYA BOTONES */
/* Lee la opcion del menu que se selecciono, y se ejecuta, si se le manda solo 4 o solo 5 se rompe (igual esto cambia cuando haya botones) */
void readMenuInput(Snake * snake, InputHandler * inputHandler, LCD * lcd, HighscoreHandler * highscore,  MaxMatrix * screen,uint8_t * intensity, uint64_t * lastUpdatedMillis, uint64_t * lastMovedMillis, Direction * input, double * waitTimeFactor, double * waitDecreaseRatioFactor, uint8_t * lcdIntensity, int16_t * lastButtonRead){
  if(Serial.available()){
    char c = Serial.read(); //Deberiamos agregar un while(Serial.available()) para que lea todo el buffer por si se insertaron mas de una letra? Nos quedamos con la primera o la ultima?
    switch(toupper(c)){
      case '1': // Play
        snake->revive(INIT_LENGTH, INIT_DIR, INIT_WAIT, INIT_ROW_POS, INIT_COL_POS);
        screen->clear();
        printWholeBody(snake->getBody(), snake->getCurrentLength(), snake->getHead(), screen, *intensity);
        *lastUpdatedMillis = *lastMovedMillis = millis();
        *input = INIT_DIR;
        lcd->clear();
      break;
      case '2': // Show Highscores
        lcd->clear();
        printHighscores(*highscore, lcd);
        delay(2000);
        lcd->clear();
      break;
      case '3': // Reset Highscores
        highscore->resetScores();
      break;
      case '4': // Set intensity
        *intensity = Serial.read() - '0';
        screen->setIntensity(*intensity);
        *lcdIntensity = GET_LCD_INTENSITY(*intensity);
        lcd->setBrightness(*lcdIntensity);
      break;
      case '5': // Difficulty
      {
        *waitDecreaseRatioFactor = *waitTimeFactor = 1;
        int difficulty = Serial.read() - '0' - 1;
        for(int i=0; i<difficulty; i++){
          *waitDecreaseRatioFactor = (*waitTimeFactor -= DIFFICULTY_INTERVAL);
        }
      }
      break;
      case 'A':
        lcd->upButtonPressed();
      break;
      case 'D':
        lcd->downButtonPressed();
      break;
      case 'W':
        {
          const uint8_t* activeButtons = inputHandler->readInputs();
          uint8_t activeButtonsCount = inputHandler->getActivePinsCount();
          uint8_t item = lcd->selectButtonPressed() + 1;
          switch(item){
            case 1: // Play
              snake->revive(INIT_LENGTH, INIT_DIR, INIT_WAIT, INIT_ROW_POS, INIT_COL_POS);
              screen->clear();
              printWholeBody(snake->getBody(), snake->getCurrentLength(), snake->getHead(), screen, *intensity);
              *lastUpdatedMillis = *lastMovedMillis = millis();
              *input = INIT_DIR;
            break;
            case 2: // Show Highscores
              c = 0;
              lcd->clear();
              printHighscores(*highscore, lcd);
              lcd->setShowCursor(false);
              do{
                lcd->refresh();
                if(Serial.available()){
                  c = Serial.read();
                  cleanSerial();
                }
              }while(toupper(c) != 'W');
              lcd->clear();
              printMenu(highscore, intensity,lcd);
              lcd->setShowCursor(true);
            break;
            case 3: // Reset Highscores
              highscore->resetScores();
              break;
            case 4: // Set intensity
            {
              lcd->clear();
              bool done = false;
              char * buff = (char*)malloc(1 * sizeof(*buff));
              buff[0] = *intensity + '0';
              lcd->addText("Intensity:",0);
              lcd->addText(buff,0);
              do{
                buff[0] = *intensity + '0';
                lcd->refresh();
                if(Serial.available()){
                  char but = Serial.read();
                  switch(toupper(but)){
                    case 'A':
                      lcd->clear();
                      *intensity = ((DEFAULT_LED_INTENSITY+1)+*intensity - 1)%(DEFAULT_LED_INTENSITY+1);
                      buff[0] = *intensity + '0';
                      lcd->addText("Intensity:",0);
                      lcd->addText(buff,0);
                      screen->setIntensity(*intensity);
                      *lcdIntensity = GET_LCD_INTENSITY(*intensity);
                      lcd->setBrightness(*lcdIntensity);
                    break;
                    case 'D':
                      lcd->clear();
                      *intensity = ((DEFAULT_LED_INTENSITY+1)+*intensity + 1)%(DEFAULT_LED_INTENSITY+1);
                      buff[0] = *intensity + '0';
                      lcd->addText("Intensity:",0);
                      lcd->addText(buff,0);
                      screen->setIntensity(*intensity);
                      *lcdIntensity = GET_LCD_INTENSITY(*intensity);
                      lcd->setBrightness(*lcdIntensity);
                    break;
                    case 'W':
                      done = true;
                    break;
                  }
                }
              }while(!done);
              free(buff);
              printMenu(highscore, intensity, lcd);
            }
              break;
            case 5: // Difficulty
            {
              *waitDecreaseRatioFactor = *waitTimeFactor = 1;
              int difficulty = 0;
              lcd->clear();
              bool done = false;
              char * buff = (char*)malloc(1 * sizeof(*buff));
              buff[0] = difficulty + '0';
              lcd->addText("Difficulty:",0);
              lcd->addText(buff,0);
              do{
                buff[0] = difficulty + '0';
                lcd->refresh();
                if(Serial.available()){
                  char but = Serial.read();
                  switch(toupper(but)){
                    case 'A':
                      lcd->clear();
                      difficulty = ((DIFFICULTY_LEVELS+1)+difficulty - 1)%(DIFFICULTY_LEVELS+1);
                      buff[0] = difficulty + '0';
                      lcd->addText("Difficulty:",0);
                      lcd->addText(buff,0);
                    break;
                    case 'D':
                      lcd->clear();
                      difficulty = ((DIFFICULTY_LEVELS+1)+difficulty + 1)%(DIFFICULTY_LEVELS+1);
                      buff[0] = difficulty + '0';
                      lcd->addText("Difficulty:",0);
                      lcd->addText(buff,0);
                    break;
                    case 'W':
                      done = true;
                      for(int i=0; i<difficulty; i++){
                        *waitDecreaseRatioFactor = (*waitTimeFactor -= DIFFICULTY_INTERVAL);
                      }
                    break;
                  }
                }
              }while(!done);
              free(buff);
              printMenu(highscore, intensity, lcd);
            }
            break;
          }
        }
      break;
    }
    cleanSerial();
    if((c > '1') && (c <= '5') && (c!='\r')){
      printMenu(highscore, intensity, lcd);
    }
  } else {
    const uint8_t* activeButtons = inputHandler->readInputs();
    uint8_t activeButtonsCount = inputHandler->getActivePinsCount();
    if (activeButtonsCount > 0) {
      if (*lastButtonRead != activeButtons[0]) {
        *lastButtonRead = activeButtons[0];

        switch(activeButtons[0]) {
          case LEFT_BUTTON_PIN:
            lcd->upButtonPressed();
            //Serial.println("Button left");
            break;
          case RIGHT_BUTTON_PIN:
            lcd->downButtonPressed();
            //Serial.println("Button right");
            break;
          case SELECT_BUTTON_PIN:
             {
                  const uint8_t* activeButtons = inputHandler->readInputs();
                  uint8_t activeButtonsCount = inputHandler->getActivePinsCount();
                  uint8_t item = lcd->selectButtonPressed() + 1;
                  switch(item){
                    case 1: // Play
                      snake->revive(INIT_LENGTH, INIT_DIR, INIT_WAIT, INIT_ROW_POS, INIT_COL_POS);
                      screen->clear();
                      printWholeBody(snake->getBody(), snake->getCurrentLength(), snake->getHead(), screen, *intensity);
                      *lastUpdatedMillis = *lastMovedMillis = millis();
                      *input = INIT_DIR;
                    break;
                    case 2: // Show Highscores
                    {
                      char c = 0;
                      lcd->clear();
                      printHighscores(*highscore, lcd);
                      lcd->setShowCursor(false);
                      do{
                        lcd->refresh();
                        activeButtons = inputHandler->readInputs();
                        activeButtonsCount = inputHandler->getActivePinsCount();
                        if (activeButtonsCount > 0) {
                          if (*lastButtonRead != activeButtons[0]) {
                            *lastButtonRead = activeButtons[0];
                          }
                        }
                      }while(activeButtons[0] != SELECT_BUTTON_PIN);
                      lcd->clear();
                      printMenu(highscore, intensity,lcd);
                      lcd->setShowCursor(true);
                    }
                    break;
                    case 3: // Reset Highscores
                      highscore->resetScores();
                      break;
                    case 4: // Set intensity
                    {
                      lcd->clear();
                      bool done = false;
                      char * buff = (char*)malloc(1 * sizeof(*buff));
                      buff[0] = *intensity + '0';
                      lcd->addText("Intensity:",0);
                      lcd->addText(buff,0);
                      do{
                        buff[0] = *intensity + '0';
                        lcd->refresh();
                        activeButtons = inputHandler->readInputs();
                        activeButtonsCount = inputHandler->getActivePinsCount();
                        if (activeButtonsCount > 0) {
                          if (*lastButtonRead != activeButtons[0]) {
                            *lastButtonRead = activeButtons[0];
                          switch(activeButtons[0]){
                            case LEFT_BUTTON_PIN:
                              lcd->clear();
                              *intensity = ((DEFAULT_LED_INTENSITY+1)+*intensity - 1)%(DEFAULT_LED_INTENSITY+1);
                              buff[0] = *intensity + '0';
                              lcd->addText("Intensity:",0);
                              lcd->addText(buff,0);
                              screen->setIntensity(*intensity);
                              *lcdIntensity = GET_LCD_INTENSITY(*intensity);
                              lcd->setBrightness(*lcdIntensity);
                            break;
                            case RIGHT_BUTTON_PIN:
                              lcd->clear();
                              *intensity = ((DEFAULT_LED_INTENSITY+1)+*intensity + 1)%(DEFAULT_LED_INTENSITY+1);
                              buff[0] = *intensity + '0';
                              lcd->addText("Intensity:",0);
                              lcd->addText(buff,0);
                              screen->setIntensity(*intensity);
                              *lcdIntensity = GET_LCD_INTENSITY(*intensity);
                              lcd->setBrightness(*lcdIntensity);
                            break;
                            case SELECT_BUTTON_PIN:
                              done = true;
                            break;
                          }
                        }
                       }
                      }while(!done);
                      free(buff);
                      printMenu(highscore, intensity, lcd);
                    }
                      break;
                    case 5: // Difficulty
                    {
                      *waitDecreaseRatioFactor = *waitTimeFactor = 1;
                      int difficulty = 0;
                      lcd->clear();
                      bool done = false;
                      char * buff = (char*)malloc(1 * sizeof(*buff));
                      buff[0] = difficulty + '0';
                      lcd->addText("Difficulty:",0);
                      lcd->addText(buff,0);
                      do{
                        buff[0] = difficulty + '0';
                        lcd->refresh();
                        activeButtons = inputHandler->readInputs();
                        activeButtonsCount = inputHandler->getActivePinsCount();
                        if (activeButtonsCount > 0) {
                          if (*lastButtonRead != activeButtons[0]) {
                            *lastButtonRead = activeButtons[0];
                          switch(activeButtons[0]){
                            case LEFT_BUTTON_PIN:
                              lcd->clear();
                              difficulty = ((DIFFICULTY_LEVELS+1)+difficulty - 1)%(DIFFICULTY_LEVELS+1);
                              buff[0] = difficulty + '0';
                              lcd->addText("Difficulty:",0);
                              lcd->addText(buff,0);
                            break;
                            case RIGHT_BUTTON_PIN:
                              lcd->clear();
                              difficulty = ((DIFFICULTY_LEVELS+1)+difficulty + 1)%(DIFFICULTY_LEVELS+1);
                              buff[0] = difficulty + '0';
                              lcd->addText("Difficulty:",0);
                              lcd->addText(buff,0);
                            break;
                            case SELECT_BUTTON_PIN:
                              done = true;
                              for(int i=0; i<difficulty; i++){
                                *waitDecreaseRatioFactor = (*waitTimeFactor -= DIFFICULTY_INTERVAL);
                              }
                            break;
                          }
                        }
                       }
                      }while(!done);
                      free(buff);
                      printMenu(highscore, intensity, lcd);
                    }
                    break;
                  }
              break;
            }
            //Serial.println("Button select");
            break;
        }
      }
    } else {
      *lastButtonRead = NO_BUTTON;
    }
  }
}
  
