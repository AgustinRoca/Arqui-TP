#include "LCD.h"

#include "Utilities.h"

#define DEFAULT_TIMEOUT  500
#define BLANK ' '

void LCD::initializeVariables() {
  timeout = DEFAULT_TIMEOUT;
  lastPrinted = 0;
  startingCol = 0;
  textLenght = 0;
  rows = 0;
  cols = 0;
  line = 0;
  
  continuous = false;
  finished = true;
  
  current = NULL;
  text = NULL;
}

void LCD::clearLine() {
  this->setCursor(0, line);
  for (uint8_t i = 0; i < cols; i++) {
    this->print(BLANK);
  }
}

void LCD::printInitialText() {
  if (current != NULL) {
    uint8_t cursor = startingCol;
    this->setCursor(cursor, line);
    while (cursor < cols && *current != '\0') {
      this->print(*(current++));
      cursor++;
    }
  }
}


LCD::LCD(uint8_t rs, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
  uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) : LiquidCrystal (rs, enable, 
                                                    d0, d1, d2, d3,
                                                    d4, d5, d6, d7) {
  initializeVariables();
}
  
LCD::LCD(uint8_t rs, uint8_t rw, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
  uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) : LiquidCrystal (rs, rw, enable, 
                                                    d0, d1, d2, d3,
                                                    d4, d5, d6, d7) {
  initializeVariables();
}

LCD::LCD(uint8_t rs, uint8_t rw, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) : LiquidCrystal (rs, rw, enable, 
                                                    d0, d1, d2, d3) {
  initializeVariables();
}

LCD::LCD(uint8_t rs, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) : LiquidCrystal (rs, enable, 
                                                    d0, d1, d2, d3) {
  initializeVariables();
}

void LCD::setCharacterTimeout(uint32_t _timeout) {
  timeout = _timeout;
}

void LCD::setText(char* _text, uint8_t _line) {
  text = _text;
  
  if (text != NULL)
    textLenght = strlen(text);
  
  line = _line < rows ? line : rows - 1;
}

void LCD::setContinuous(bool _continuous) {
  continuous = _continuous;
}

void LCD::setBrightnessPin(int16_t pin) {
  brightnessPin = pin;
  if (brightnessPin >= 0) {
    pinMode(pin, OUTPUT);
    analogWrite(pin, 0);
  }
}

void LCD::setContrastPin(int16_t pin) {
  contrastPin = pin;
  if (contrastPin >= 0) {
    pinMode(pin, OUTPUT);
    analogWrite(pin, 0);
  }
}

void LCD::setStartingCol(uint8_t col) {
  startingCol = col < cols ? col : cols - 1;
}

void LCD::setRows(uint8_t _rows) {
  rows = _rows;
}

void LCD::setCols(uint8_t _cols) {
  startingCol = cols - 1;
  cols = _cols;
}

void LCD::setBrightness(uint8_t intensity) {
  if (brightnessPin >= 0) {
    analogWrite(brightnessPin, intensity);
  }
}

void LCD::setContrast(uint8_t intensity) {
  if (contrastPin >= 0) {
    analogWrite(contrastPin, intensity);
  }
}

bool LCD::getFinished() {
  return finished;
}


void LCD::printScrollableText() {
  if (line < rows && text != NULL) {
    finished = false;
    lastPrinted = 0;
    current = text;
    clearLine();
    printInitialText();
    this->setCursor(cols - 1, line);
  } else {
    finished = true;
  }
}

void LCD::refresh() {
  if ((!finished || (finished && continuous && textLenght > cols - startingCol)) && millis2() - lastPrinted > timeout) {
    if (current == NULL || *current == '\0') {
      finished = true;
      if (continuous) {
        current = text;
        
        clearLine();
        printInitialText();
        this->setCursor(cols - 1, line);
        lastPrinted = millis2();
      }
    } else {
      uint8_t cursor = current - text < cols ? cols - (current - text) - 1 : 0;
      
      this->setCursor(cursor, line);
      while (cursor < cols - 1) {
        this->print(*(current - (cols - cursor) + 1));
        cursor++;
      }
      
      this->print(*(current++));
      lastPrinted = millis2();
    }
  }
}
