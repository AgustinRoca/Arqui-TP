#include "LCD.h"

#define DEFAULT_TIMEOUT  500
#define UPDATE_BRIGHTNESS_EVERY_MILLIS 10
#define UPDATE_CONTRAST_EVERY_MILLIS 10
#define BLANK ' '
#define BLANK_S " "
#define MIN(x, y) (x > y ? y : x)

#define NEGATIVE 0
#define POSITIVE 1
#define INITIAL_FADING_PRINT_DELAY 10

#define CHUNK 5

#define INITIAL_BRIGHTNESS 125
#define INITIAL_CONTRAST 125

// ------ LCD TEXT ------
LCD::LCDText::LCDText(const char* text, uint8_t startingCol, bool continuous, uint8_t index) {
  this->text = text;
  this->beingPrinted = false;
  this->continuous = continuous;

  if (text != NULL) {
    this->index = index;
    this->length = strlen(text);
    this->startingCol = startingCol;
  } else {
    this->index = 0;
    this->length = 0;
    this->startingCol = 0;
  }
}

void LCD::LCDText::setBeingPrinted(bool state) {
  this->beingPrinted = state;
}

uint8_t LCD::LCDText::getStartingCol() {
  return startingCol;
}

bool LCD::LCDText::isBeingPrinted() {
  return beingPrinted;
}

uint16_t LCD::LCDText::getLength() {
  return length;
}

uint8_t LCD::LCDText::getIndex() {
  return index;
}

const char* LCD::LCDText::getText() {
  return text;
}

bool LCD::LCDText::isContinuous() {
  return continuous;
}

void LCD::LCDText::setLCDLine(LCDLine* lcdLine) {
  beingPrinted = false;
  this->lcdLine = lcdLine;
}


// ------ LCD LINE ------
LCD::LCDLine::LCDLine(uint8_t _maxCols) {
  maxCols = _maxCols;
}

bool LCD::LCDLine::isContinuous() {
  return lcdText != NULL && lcdText->isContinuous();
}

bool LCD::LCDLine::hasFinishedCurrentLine() {
  return finishedCurrentLine;
}

bool LCD::LCDLine::hasFinished() {
  if (finished || lcdText == NULL)
    return true;

  return current == NULL || finished;
}

bool LCD::LCDLine::shouldPrint(uint32_t delay) {
  if (lcdText == NULL)
    return false;

  if (millis() - lastPrinted > delay) {
    return !finished || (lcdText->isContinuous() && lcdText->getLength() > maxCols - lcdText->getStartingCol());
  } 
  
  return false;
}

const char* LCD::LCDLine::getCurrent() {
  if (lcdText == NULL)
    return NULL;

  if (hasFinished()) {
    if (lcdText->isContinuous()) {
      uint16_t alreadyPrinted = current - lcdText->getText();
      
      if (lcdText->getLength() <= maxCols + alreadyPrinted - 1) {
        current = lcdText->getText();
      }
    } else {
      return NULL;
    }
  }

  lastPrinted = millis();
  
  if (shouldClear) {
    currentClearedCol++;
    if (currentClearedCol >= maxCols)
      shouldClear = false;

    return BLANK_S;
  }
  
  const char* ret = current;
  if (started && hasFinishedCurrentLine()) {
    current++;
  }

  if (!started) {
    started = true;
  }

  return ret;
}

uint8_t LCD::LCDLine::getColToPrint() {
  if (shouldClear)
    return currentClearedCol;

  uint8_t col;
  uint16_t alreadyPrinted = current - lcdText->getText();
  if (alreadyPrinted < maxCols) {
    col = MIN(lcdText->getStartingCol(), maxCols - alreadyPrinted - 1);
  } else {
    col = 0;
  }

  if (lcdText->getLength() <= maxCols + alreadyPrinted - 1) {
    finished = true;
    finishedCurrentLine = true;
  } else {
    if (col == 0 || hasFinished()) {
      finishedCurrentLine = true;
    } else if (finishedCurrentLine) {
      finishedCurrentLine = false;
    }
  }

  return col;
}

void LCD::LCDLine::setLCDText(LCDText* lcdText) {
  if (this->lcdText != NULL) {
    this->lcdText->setBeingPrinted(false);
    this->lcdText->setLCDLine(NULL);
    clear();
  }

  started = false;
  finished = false;
  finishedCurrentLine = false;
  lastPrinted = 0;
  this->lcdText = lcdText;
  if (lcdText != NULL) {
    current = lcdText->getText();
  } else {
    current = NULL;
  }
}

void LCD::LCDLine::clear() {
  if (this->lcdText != NULL) 
    this->lcdText->setBeingPrinted(false);
  currentClearedCol = 0;
  shouldClear = true;
}

// ------ LCD ------
int32_t LCD::getLinePrintingTextIndex(uint16_t index) {
  if (!indexBeingPrinted(index)) {
    return -1;
  }

  return index - (currentPage * rows);
}

bool LCD::indexBeingPrinted(uint16_t index) {
  return currentPage * rows <= index && index < (currentPage + 1) * rows;
}

 void LCD::initializeVariables(uint8_t _cols, uint8_t _rows) {
  timeout = DEFAULT_TIMEOUT;
  rows = _rows;
  cols = _cols;
  
  fadingBrightnessOperation = POSITIVE;
  fadingContrastOperation = POSITIVE;
  lastBrightnessMillis = 0;
  lastContrastMillis = 0;
  currentBrightness = INITIAL_BRIGHTNESS;
  targetBrightness = INITIAL_BRIGHTNESS;
  currentContrast = INITIAL_CONTRAST;
  targetContrast = INITIAL_CONTRAST;
  brightnessPin = -1;
  contrastPin = -1;

  fadingBrightness = false;
  fadingContrast = false;

  lines = (LCDLine**) malloc(sizeof(LCDLine*) * rows);
  for (uint8_t i = 0; i < rows; i++) {
    lines[i] = new LCDLine(cols);
  }
}

void LCD::refreshBrightness() {
  if (millis() - lastBrightnessMillis > UPDATE_BRIGHTNESS_EVERY_MILLIS) {
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
}

void LCD::refreshContrast() {
  if (millis() - lastContrastMillis > UPDATE_CONTRAST_EVERY_MILLIS) {
    analogWrite(contrastPin, (pow(256, (currentContrast / 255.0)) - 1) + 0.5);
    
    if (fadingContrastOperation == NEGATIVE)
      currentContrast--;
    else
      currentContrast++;
    
    if (currentContrast == targetContrast) {
      fadingContrast = false;
    } else {
      lastContrastMillis = millis();
    }
  }
}

void LCD::setPreviousPage() {
  if (currentPage > 1) {
    currentPage++;
    
    for (uint8_t i = 0, j = (currentPage - 1) * rows; i < rows && j < lcdTextsCount; i++, j++) {
      lines[i]->setLCDText(lcdTexts[j]);
    }
  }
}

void LCD::setFollowingPage() {
  if (currentPage * rows > lcdTextsCount) {
    currentPage++;
    
    for (uint8_t i = 0, j = currentPage * rows; i < rows && j < lcdTextsCount; i++, j++) {
      lines[i]->setLCDText(lcdTexts[j]);
    }
  }
}


 LCD::LCD(uint8_t rs, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
  uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
  uint8_t _cols, uint8_t _rows) {
  lcd = new LiquidCrystal(rs, enable, d0, d1, d2, d3, d4, d5, d6, d7);
  initializeVariables(_cols, _rows);
}

 LCD::LCD(uint8_t rs, uint8_t rw, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
  uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
  uint8_t _cols, uint8_t _rows) {
  lcd = new LiquidCrystal(rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
  initializeVariables(_cols, _rows);
}

 LCD::LCD(uint8_t rs, uint8_t rw, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
  uint8_t _cols, uint8_t _rows) {
  lcd = new LiquidCrystal(rs, rw, enable, d0, d1, d2, d3);
  initializeVariables(_cols, _rows);
}

 LCD::LCD(uint8_t rs, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
  uint8_t _cols, uint8_t _rows) {
  lcd = new LiquidCrystal(rs, enable, d0, d1, d2, d3);
  initializeVariables(_cols, _rows);
}

 void LCD::addText(const char* text, uint8_t startingCol, bool continuous) {
  LCDLine* lcdLine;

  LCDText* lcdText = new LCDText(text, startingCol, continuous, lcdTextsCount);
  if (indexBeingPrinted(lcdTextsCount)) {
    lcdLine = lines[getLinePrintingTextIndex(lcdTextsCount)];
    lcdLine->setLCDText(lcdText);
  }

  if (lcdTextsCount == lcdTextsTotalLength) {
    lcdTextsTotalLength += CHUNK;

    lcdTexts = (LCDText**) realloc(lcdTexts, sizeof(LCDText*) * lcdTextsTotalLength);
  }

  lcdTexts[lcdTextsCount++] = lcdText;
}

 void LCD::addText(const char* text, uint8_t startingCol) {
   addText(text, startingCol, true);
}

void LCD::clear() {
  for (uint8_t i = 0; i < rows; i++) {
    lines[i]->clear();
    lines[i]->setLCDText(NULL);
  }

  for (uint16_t i = 0; i < lcdTextsCount; i++) {
    free(lcdTexts[i]);
  }

  free(lcdTexts);
  lcdTexts = NULL;
  lcdTextsTotalLength = 0;
  lcdTextsCount = 0;
  currentPage = 0;

  setShowCursor(false);
  lcd->clear();
}

 void LCD::setCharacterTimeout(uint32_t _timeout) {
  timeout = _timeout;
}

 void LCD::setBrightnessPin(int16_t pin) {
  brightnessPin = pin;
  if (brightnessPin >= 0) {
    pinMode(pin, OUTPUT);
    analogWrite(pin, currentBrightness);
  }
}

 void LCD::setContrastPin(int16_t pin) {
  contrastPin = pin;
  if (contrastPin >= 0) {
    pinMode(pin, OUTPUT);
    analogWrite(pin, currentContrast);
  }
}

 void LCD::setBrightness(uint8_t intensity) {
  if (brightnessPin >= 0 && intensity != currentBrightness) {
    fadingBrightness = true;
    targetBrightness = intensity;
    fadingBrightnessOperation = intensity > currentBrightness ? POSITIVE : NEGATIVE;
  }
}

 void LCD::setContrast(uint8_t intensity) {
  if (contrastPin >= 0 && intensity != currentContrast) {
    fadingContrast = true;
    targetContrast = intensity;
    fadingContrastOperation = intensity > currentContrast ? POSITIVE : NEGATIVE;
  }
}

void LCD::setShowCursor(bool showCursor) {
  if (this->showCursor != showCursor) {
    this->showCursor = showCursor;
    if (!showCursor) {
      highlightedLine = 0;
      if (isShowingCursor) {
        lcd->noBlink();
      }
    }
  }
}

void LCD::upButtonPressed() {
  if (lcdTextsCount == 0)
    return;

  if (highlightedLine == 0 && currentPage > 0) {
    setPreviousPage();
    highlightedLine = rows - 1;
  } else if (highlightedLine > 0) {
    highlightedLine--;
  }
}

void LCD::downButtonPressed() {
  if (lcdTextsCount == 0)
    return;

  if (highlightedLine == rows - 1 && lcdTextsCount / rows > currentPage) {
    setFollowingPage();
    highlightedLine = 0;
  } else if (highlightedLine < rows - 1 && highlightedLine + 1 < lcdTextsCount - (currentPage * rows)) {
    highlightedLine++;
  }
}

uint8_t LCD::selectButtonPressed() {
  return highlightedLine;
}

void LCD::refresh() {
  if (fadingBrightness)
    refreshBrightness();
  if (fadingContrast)
    refreshContrast();

  for (uint8_t i = 0; i < rows; i++) {
    LCDLine* lcdLine = lines[i];

    if (lcdLine->shouldPrint(lcdLine->hasFinishedCurrentLine() ? timeout : 0)) {
      uint8_t col = lcdLine->getColToPrint();
      const char* current = lcdLine->getCurrent(); 

      char c;
      uint8_t start = col;
      lcd->setCursor(col, i);

      if (showCursor && highlightedLine == i && !isShowingCursor) {
        lcd->blink();
        isShowingCursor = true;
      } else if (isShowingCursor) {
        isShowingCursor = false;
      }

      while (start++ < cols && (c = *current++) != '\0') {
        lcd->print(c);
      }

      if (showCursor && highlightedLine == i) {
        lcd->setCursor(0, i);
      }
    }
  }
}

void LCD::begin() {
  lcd->begin(cols, rows);
  lcd->clear();
  lcd->cursor();
}
