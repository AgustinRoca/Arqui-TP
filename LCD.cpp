#include "LCD.h"

#define DEFAULT_TIMEOUT  500
#define UPDATE_BRIGHTNESS_EVERY_MILLIS 10
#define UPDATE_CONTRAST_EVERY_MILLIS 10
#define BLANK ' '

#define NEGATIVE 0
#define POSITIVE 1

 void LCD::initializeVariables() {
  timeout = DEFAULT_TIMEOUT;
  lastPrinted = 0;
  startingCol = 0;
  textLenght = 0;
  rows = 0;
  cols = 0;
  line = 0;
  
  fadingBrightnessOperation = POSITIVE;
  fadingContrastOperation = POSITIVE;
  lastBrightnessMillis = 0;
  lastContrastMillis = 0;
  currentBrightness = 0;
  targetBrightness = 0;
  currentContrast = 0;
  targetContrast = 0;
  brightnessPin = -1;
  contrastPin = -1;

  fadingBrightness = false;
  fadingContrast = false;
  printingText = false;
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
  uint8_t cursor = startingCol;

   this->setCursor(cursor, line);
  while (cursor < cols && *current != '\0') {
    this->print(*(current++));
    cursor++;
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

   if (_line < rows)
    line = _line;
}

 void LCD::setContinuous(bool _continuous) {
  continuous = _continuous;
}

 void LCD::setBrightnessPin(int16_t pin) {
  brightnessPin = pin;
  if (brightnessPin >= 0) {
    pinMode(pin, OUTPUT);
  }
}

 void LCD::setContrastPin(int16_t pin) {
  contrastPin = pin;
  if (contrastPin >= 0) {
    pinMode(pin, OUTPUT);
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
  if (brightnessPin >= 0 && intensity != currentBrightness) {
    fadingBrightness = true;
    targetBrightness = intensity;
    fadingBrightnessOperation = intensity > currentBrightness ? POSITIVE : NEGATIVE;
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
  if (fadingBrightness && millis() - lastBrightnessMillis > UPDATE_BRIGHTNESS_EVERY_MILLIS) {
    analogWrite(brightnessPin, (pow(256, (currentBrightness / 255.0)) - 1) + 0.5);
    
    if (fadingBrightnessOperation == NEGATIVE)
      currentBrightness--;
    else
      currentBrightness++;

    if (currentBrightness == targetBrightness) {
      fadingBrightness = false;
    } else {
      lastBrightnessMillis = millis();
    }
  }

  if (fadingContrast && millis() - lastContrastMillis > UPDATE_CONTRAST_EVERY_MILLIS) {
    analogWrite(contrastPin, (pow(256, (currentContrast / 255.0)) - 1) + 0.5);
    
    currentContrast++;
    if (currentContrast == targetContrast) {
      fadingContrast = false;
    } else {
      lastContrastMillis = millis();
    }
  }

   if (printingText) {
    if ((!finished || (finished && continuous && textLenght > cols - startingCol)) && millis() - lastPrinted > timeout) {
      if (current == NULL || *current == '\0') {
        finished = true;
        if (continuous) {
          current = text;

          clearLine();
          printInitialText();
          this->setCursor(cols - 1, line);
          lastPrinted = millis();
        }
      } else {
        uint8_t cursor = current - text < cols ? cols - (current - text) - 1 : 0;

        this->setCursor(cursor, line);
        while (cursor < cols - 1) {
          this->print(*(current - (cols - cursor) + 1));
          cursor++;
        }

        this->print(*(current++));
        lastPrinted = millis();
      }
    }
   }
}
