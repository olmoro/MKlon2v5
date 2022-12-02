#ifndef _MDISPLAY_H_
#define _MDISPLAY_H_

#include <TFT_eSPI.h>
#include "stdio.h"
#include "stdint.h"

class TFT_eSPI;

class MDisplay
{
    public:
        MDisplay();
        ~MDisplay();

        // declare size of working string buffers.
        static constexpr size_t MaxString = 22;   // Max 21 + 1 for TextSize 1 TFT 1.8" 128*160

        static constexpr uint16_t SEC   = 0;
        static constexpr uint16_t HOUR  = 1;

        void initLCD();
        //void runDisplay(float u, float i, float celsius, int time, float ah, int percent, bool ap);
        void runDisplay(float celsius, bool ap);

        void showVolt(float volt, uint8_t pls);    // showVoltage(...) new name
        void showAmp(float amp, uint8_t pls);
        void showMode(char *s);
        void showHelp(char *s);
        void showDuration( int time, int plan );
        void showAh( float ah );
        void showLabel(char *s);

        void barOff();
        void initBar( uint16_t color );
        void barStop();


    private:
        TFT_eSPI * tft = nullptr;

        const int TFT_GREY = 0xBDF7;

        void initTFT();

        void fulfill( int percent, uint16_t rgb );

        void displayVoltage();
        void displayCurrent();
        void displayMode();
        void displayHelp();
        void displayFulfill();
        void displayDuration();
        void displayAmphours();
        void displayHeap();
        void displayCelsius( float celsius );
        void displayLabel();

        struct MVoltage
        {
            static constexpr int8_t  po_x   =  12;
            static constexpr int8_t  po_y   =  02;
            static constexpr int8_t  font   =   4;
            static constexpr uint16_t fgcolor = TFT_CYAN;   // foreground colour
            static constexpr uint16_t bgcolor = TFT_BLACK;  // backgorund colour
        };

        struct MCurrent
        {
            static constexpr int8_t  po_x   =  12;
            static constexpr int8_t  po_y   =  24;
            static constexpr int8_t  font   =   4;
            static constexpr uint16_t fgcolor = TFT_CYAN;   // foreground colour
            static constexpr uint16_t bgcolor = TFT_BLACK;  // backgorund colour
        };

        struct MMode
        {
            static constexpr int8_t  po_x   =  64;
            static constexpr int8_t  po_y   =  46;
            static constexpr int8_t  font   =   2;
            static constexpr uint16_t fgcolor = TFT_GREEN;  // foreground colour
            static constexpr uint16_t bgcolor = TFT_BLACK;  // backgorund colour
        };

        struct MHelp
        {
            static constexpr int8_t  po_x   =   0;
            static constexpr int8_t  po_y   =  68;
            static constexpr int8_t  font   =   2;
            static constexpr uint16_t fgcolor = TFT_GREEN;  // foreground colour
            static constexpr uint16_t bgcolor = TFT_BLACK;  // backgorund colour
        };

        struct MBar
        {
            // Параметры отображения %% выполнения
            static constexpr int32_t  min_x      =  0;  //14;
            static constexpr int32_t  max_x      = 100/2; // nu
            static constexpr int32_t  po_y   =  90;
        };

        struct MDuration
        {
            static constexpr int8_t  po_x   =  36;
            static constexpr int8_t  po_y   = 100;
            static constexpr int8_t  font   =   2;
            static constexpr uint16_t fgcolor = TFT_YELLOW;   // foreground colour
            static constexpr uint16_t bgcolor = TFT_BLACK;  // backgorund colour
        };

        struct MAmpHours
        {
            static constexpr int8_t  po_x   =  36;
            static constexpr int8_t  po_y   = 115;
            static constexpr int8_t  font   =   2;
            static constexpr uint16_t fgcolor = TFT_CYAN;   // foreground colour
            static constexpr uint16_t bgcolor = TFT_BLACK;  // backgorund colour
        };

        struct MHeap
        {
            static constexpr int8_t  po_x   =  10;  //36;
            static constexpr int8_t  po_y   = 130;
            static constexpr int8_t  font   =   2;
            static constexpr uint16_t fgcolor = TFT_ORANGE;   // foreground colour
            static constexpr uint16_t bgcolor = TFT_BLACK;  // backgorund colour
        };

        struct MCelsius
        {
            static constexpr int8_t  po_x   =  77;  //36;
            static constexpr int8_t  po_y   = 130;
            static constexpr int8_t  font   =   2;
            static constexpr uint16_t fgcolor = TFT_GREEN;   // foreground colour
            static constexpr uint16_t bgcolor = TFT_BLACK;  // backgorund colour
        };

        struct MLabel
        {
            static constexpr int8_t  po_x   =  64;
            static constexpr int8_t  po_y   = 145;
            static constexpr int8_t  font   =   2;
            static constexpr uint16_t fgcolor = TFT_CYAN;   // foreground colour
            static constexpr uint16_t bgcolor = TFT_BLACK;  // backgorund colour
        };

        char newVoltageString[ MaxString ]  = { 0 };    // nu
        char newCurrentString[ MaxString ]  = { 0 };    // nu
        char modeString[ MaxString ]     = { 0 };
        char helpString[ MaxString ]     = { 0 };

        int  percent                        =  0;   //-1;
        uint16_t color                      = TFT_GREEN;

        float volt                          = 12.6;     // вольты для отображения
        float amp                           =  5.5;     // амперы для отображения
        uint8_t placesV                     =  2;       // знаков после зпт
        uint8_t placesI                     =  2;       // знаков после зпт

        uint16_t plan                       = SEC;
        unsigned long upSeconds             =   0;

        float ah                            =   0;

        char labelString[ MaxString ]    = { 0 };

};

#endif  // !_MDISPLAY_H_
