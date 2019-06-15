#ifndef _LCD_H_
#define _LCD_H_

#include "Arduino.h"
#include <LiquidCrystal.h>

class LCD : public LiquidCrystal {
  private:
    uint64_t lastPrinted;
    uint16_t textLenght;
    uint8_t startingCol;
    uint32_t timeout;
    bool continuous;
    bool finished;
    char* current;
    uint8_t rows;
    uint8_t cols;
    uint8_t line;
    char* text;
    
    int16_t brightnessPin;
    int16_t contrastPin;
    
    void initializeVariables();
    void printInitialText();
    void clearLine();
  
  public:
    LCD(uint8_t rs, uint8_t enable,
      uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
      uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
    LCD(uint8_t rs, uint8_t rw, uint8_t enable,
      uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
      uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
    LCD(uint8_t rs, uint8_t rw, uint8_t enable,
      uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
    LCD(uint8_t rs, uint8_t enable,
      uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
  
    void setCharacterTimeout(uint32_t _timeout);
    void setText(char* _text, uint8_t _line);
    void setContinuous(bool _continuous);
    void setBrightnessPin(int16_t pin);
    void setContrastPin(int16_t pin);
    void setStartingCol(uint8_t col);
    void setBrightness(uint8_t intensity);
    void setContrast(uint8_t intensity);
    void setRows(uint8_t _rows);
    void setCols(uint8_t _cols);
    
    bool getFinished();
    
    void printScrollableText();
    void refresh();
};

#endif
