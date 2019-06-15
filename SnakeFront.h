#ifndef _SnakeFront_H_
#define _SnakeFront_H_

#include <MaxMatrix.h>

#include "HighscoreHandler.h"
#include "Snake.h"
#include "LCD.h"

class SnakeFront {
  private:
    HighscoreHandler* highscoreHandler;
    MaxMatrix* screen;
    Snake* snake;
    LCD* lcd;
    
    uint8_t matrixIntensity;
    uint8_t snakeMaxLength;
    uint8_t matrixRows;
    uint8_t matrixCols;
    uint8_t lcdRows;
    uint8_t lcdCols;
    
    void setDotInScreen(Position pos, uint8_t intensity);
  
  public:
    SnakeFront();
    
    void initialize(HighscoreHandler* _highscoreHandler,
                    LCD* _lcd, 
                    MaxMatrix* _screen, 
                    Snake* _snake,
                    uint8_t _matrixRows,
                    uint8_t _matrixCols,
                    uint8_t _lcdRows,
                    uint8_t _lcdCols,
                    uint8_t _snakeMaxLength);
    void printWholeBody(Position* body, int currentLength, int head);
    void printMove(Position newHead, Position oldTail);
    void setMatrixIntensity(uint8_t intensity);
    void setLCDIntensity(uint8_t intensity);
    void setLCDContrast(uint8_t contrast);
    void setDotInScreen(Position pos);
    void printHighscores();
    void printSkull();
    void printMenu();
};

#endif
