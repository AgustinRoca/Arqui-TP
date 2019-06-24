#ifndef _LCD_H_
#define _LCD_H_

#include "Arduino.h"
//#include <new.h>
#include <LiquidCrystal.h>

 class LCD {
  private:
    class LCDLine;
    class LCDText;

    class LCDText {
      private:
        uint8_t startingCol;
        bool beingPrinted;
        LCDLine* lcdLine;
        bool continuous;
        uint16_t length;
        uint8_t index;
        const char* text;

      public:
        LCDText(const char* text, uint8_t startingCol, bool continuous, uint8_t index);
        
        void setBeingPrinted(bool state);
        uint8_t getStartingCol();
        bool isBeingPrinted();
        void setLCDLine(LCDLine* lcdLine);
        LCDLine* getLCDLine();
        uint16_t getLength();
        bool isContinuous();
        uint8_t getIndex();
        const char* getText();
    };

    class LCDLine {
      private:
        uint32_t lastPrinted;
        uint8_t maxCols;
        uint8_t line;
        
        const char* current;
        LCDText* lcdText;

        bool finishedCurrentLine;
        bool finished;
        bool started;

      public:
        LCDLine(uint8_t maxCols);

        bool hasFinishedCurrentLine();
        bool isContinuous();
        bool hasFinished();
        bool hasStarted();
        
        bool shouldPrint(uint32_t delay);
        uint8_t getColToPrint();

        void setLCDText(LCDText* lcdText);

        uint8_t getStartingCol();

        void clear();

        const char* getCurrent();
        const char* getText();
    };

    LiquidCrystal* lcd;

    LCDText** lcdTexts;
    uint16_t lcdTextsCount;
    uint16_t lcdTextsTotalLength;
    
    uint16_t currentPage;

    LCDLine** lines;

    uint32_t lastBrightnessMillis;
    uint32_t lastContrastMillis;
    uint32_t timeout;

    uint8_t fadingBrightnessOperation;
    uint8_t fadingContrastOperation;
    uint8_t highlightedLine;
    
    bool fadingBrightness;
    bool fadingContrast;
    bool isShowingCursor;
    bool showCursor;

    uint8_t rows;
    uint8_t cols;
    uint8_t cursorRow;

    uint8_t currentBrightness;
    uint8_t targetBrightness;
    uint8_t currentContrast;
    uint8_t targetContrast;

    int16_t brightnessPin;
    int16_t contrastPin;

    int32_t getLinePrintingTextIndex(uint16_t index);
    bool indexBeingPrinted(uint16_t index);
    void initializeVariables(uint8_t _cols, uint8_t _rows);
    void refreshBrightness();
    void refreshContrast();
    void setFollowingPage();
    void setPreviousPage();

   public:
    LCD(uint8_t rs, uint8_t enable,
      uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
      uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
      uint8_t _rows, uint8_t _cols);
    LCD(uint8_t rs, uint8_t rw, uint8_t enable,
      uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
      uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
      uint8_t _rows, uint8_t _cols);
    LCD(uint8_t rs, uint8_t rw, uint8_t enable,
      uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
      uint8_t _rows, uint8_t _cols);
    LCD(uint8_t rs, uint8_t enable,
      uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
      uint8_t _rows, uint8_t _cols);

    void addText(const char* text, uint8_t startingCol, bool continuous);
    void addText(const char* text, uint8_t startingCol);
    void setCharacterTimeout(uint32_t _timeout);
    void setBrightnessPin(int16_t pin);
    void setContrastPin(int16_t pin);
    void setStartingCol(uint8_t col);
    void setBrightness(uint8_t intensity);
    void setContrast(uint8_t intensity);

    uint32_t getTimeout();
    uint8_t getCols();

    void refresh();

    void clear();
    void begin();

    void upButtonPressed();
    void downButtonPressed();
    uint8_t selectButtonPressed();
    void setShowCursor(bool showCursor);
};

 #endif
