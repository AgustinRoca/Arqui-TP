#include "SnakeFront.h"

#define OFF 0

// ----- PRIVATE
/* Traduce la posicion pos a pixeles de la screen y la imprime con intensidad intensity,si se le manda una posicion valida la ignora */
void SnakeFront::setDotInScreen(Position pos, uint8_t intensity) {
  if(pos.x < matrixCols){ // Matrices izquierdas
    if(pos.y < matrixRows){ // Matriz abajo-izquierda (1era matriz de la cascada)
      screen->setDot(7-pos.y, 7-pos.x, intensity);
    }
    else{ // Matriz arriba-izquierda (3era matriz de la cascada)
      screen->setDot(7-(pos.y % matrixRows), 7-(pos.x % matrixCols) + 2*matrixCols, intensity);
    }
  }
  else{ // Matrices Derechas
    if(pos.y < matrixRows){ // Matriz abajo-derecha (2da matriz de la cascada). Como esta invertida se le saca el 7- a las posiciones
      screen->setDot(pos.y, (pos.x % matrixCols) + matrixCols, intensity);
    }
    else{ // Matriz arriba-derecha (4ta matriz de la cascada). Como esta invertida se le saca el 7- a las posiciones
      screen->setDot(pos.y % matrixRows, (pos.x % matrixCols) + 3*matrixCols, intensity);
    }
  }
}

SnakeFront::SnakeFront() {
  
}

void SnakeFront::initialize(HighscoreHandler* _highscoreHandler,
                    LCD* _lcd, 
                    MaxMatrix* _screen, 
                    Snake* _snake,
                    uint8_t _matrixRows,
                    uint8_t _matrixCols,
                    uint8_t _lcdRows,
                    uint8_t _lcdCols,
                    uint8_t _snakeMaxLength) {
  highscoreHandler = _highscoreHandler;
  screen = _screen;
  snake = _snake;
  lcd = _lcd;
  
  snakeMaxLength = _snakeMaxLength;
  matrixRows = _matrixRows;
  matrixCols = _matrixCols;
  matrixIntensity = 0;
  lcdRows = _lcdRows;
  lcdCols = _lcdCols;
}


/* Imprime los puntajes maximos que se almacenaron (en Serial por ahora) */
void SnakeFront::printHighscores() {
  Serial.println("Puntajes Maximos:");
  
  lcd->setCursor(0, 0);
  lcd->print("Puntajes maximos: ");
  
  if (highscoreHandler->getScoresAmmount() > 0) {
    lcd->print("Puntajes maximos: ");
    for(int i=0; i < highscoreHandler->getScoresAmmount(); i++){
      Serial.print(i + 1, DEC);
      Serial.print(". ");
      Serial.println((long) highscoreHandler->getScores()[i], DEC);
      
      if (i + 1 < lcdRows) {
        lcd->setCursor(0, i + 1);
        lcd->print((uint32_t) highscoreHandler->getScores()[i]);
      }
    }
  } else {
    lcd->setCursor(0, 1);
    lcd->print("No hay puntajes registrados");
    Serial.println("No hay puntajes registrados");
  }
  
  Serial.println("--------------------------------------------");
}

void SnakeFront::setMatrixIntensity(uint8_t _matrixIntensity) {
  screen->setIntensity(_matrixIntensity);
  matrixIntensity = _matrixIntensity;
}

void SnakeFront::setLCDIntensity(uint8_t lcdIntensity) {
  lcd->setBrightness(lcdIntensity);
}

void SnakeFront::setLCDContrast(uint8_t contrast) {
  lcd->setContrast(contrast);
}

/* Traduce la posicion pos a pixeles de la screen y la imprime con intensidad intensity,si se le manda una posicion valida la ignora */
void SnakeFront::setDotInScreen(Position pos) {
  setDotInScreen(pos, matrixIntensity);
}

/* Prende todo el array de body en las matrices (asume que la pantalla esta limpia antes) */
void SnakeFront::printWholeBody() {
  for(uint16_t i = 0; i < snake->getCurrentLength(); i++){
    setDotInScreen(snake->getBody()[(snakeMaxLength + snake->getHead() - i) % snakeMaxLength], matrixIntensity);
  }
}

/* Prende el LED de la nueva cabeza de la vibora, apaga el LED de la vieja cola de la vibora */
void SnakeFront::printMove(Position newHead, Position oldTail){
  setDotInScreen(oldTail, OFF);
  setDotInScreen(newHead, matrixIntensity);
}

/* Imprime la cruz cuando se pierde */
void SnakeFront::printSkull() { //TODO: ACTUALIZAR PARA 4 MATRICES
  byte skull1[8]= {B00011001,B00001111,B00001111,B00000011,B00110010,B00010000,B00000000,B00000000}; //Parte que deberia ir en la matriz abajo-izquierda
  byte skull2[8]= {B00000000,B00000000,B00100000,B01100101,B00000111,B00011110,B00011111,B00110011}; //Parte que deberia ir en la matriz abajo-derecha
  byte skull3[8]= {B00000000,B00010000,B00110000,B00000111,B00001111,B00001111,B00011100,B00011000}; //Parte que deberia ir en la 3era matriz arriba-izquierda
  byte skull4[8]= {B00110001,B00111001,B00011111,B00011111,B00001111,B01100000,B00100000,B00000000}; //Parte que deberia ir en la 4ta matriz arriba-derecha
  
  for(int i=0; i<8; i++){
    screen->setColumn(i+2*matrixCols, skull3[i]);
    screen->setColumn(i, skull1[i]);
  }
  for(int i=0; i<8; i++){
    screen->setColumn(i+3*matrixCols, skull4[i]);
    screen->setColumn(i+matrixCols, skull2[i]);
  }
}


/* SE CAMBIA CUANDO HAYA LCD */
void SnakeFront::printMenu(){
  Serial.println("\nSNAKE\n");
  Serial.println("1.P");
  Serial.println("2.ShowHighss");
  Serial.println("3.ResHighss");
  Serial.println("4x.Intensity in x(x=(1..8))"); //Ejemplo: 41 = set Intensity in 1
  Serial.println("5x.Dif in x(x=1,2,3)"); //Ejemplo: 53 = set Dificulty in 3
  Serial.println("6x.Contrast in x(x=(1..8))"); //Ejemplo: 61 = set Contrast in 1
  Serial.println("---");
  lcd->clear();
  lcd->print("M");
}

