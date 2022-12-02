/*
  Файл   mboard.h
  Проект MESP32v7
  pcb: mesp.v7.1
*/

#ifndef _MBOARD_H_
#define _MBOARD_H_

#include "stdint.h"

class MSupervisor;

class MBoard
{
  public:
    MSupervisor * Supervisor = nullptr;       // local

    static constexpr uint8_t ch_fan = 1;      // PWM channel

  public:
    MBoard();

    void initEsp32();

    void ledROn();	  // Turn the RED LED on
    void ledROff();	  // Turn the RED LED off

    void ledGOn();    // Turn the GREEN LED on
    void ledGOff();	  // Turn the GREEN LED off

    void ledBOn();	  // Turn the BLUE LED on
    void ledBOff();	  // Turn the BLUE LED off

    void ledsOn();	
    void ledsOff();
    void ledsRed();
    void ledsGreen();
    void ledsBlue();
    void ledsYellow();	

    void blinkWhite(short cnt);
    void blinkWhite();
    void blinkRed(short cnt);
    void blinkGreen(short cnt);
    void blinkBlue(short cnt);
    void blinkYellow(short cnt);

    void buzzerOn();
    void buzzerOff();

    void lcdRstOn();
    void lcdRstOff();

    void swOn();
    void swOff();

    void outOn();
    void outOff();

    bool getNeg();
    void  calculateCelsius();
    float getCelsius();
    int16_t getAdcPG();


void initAdcT11db0();
void initAdcK11db0();
int getAdcT();
int getAdcK();

    
    int16_t getPwmVal()     { return pwmVal; }
    int16_t getDacVal()     { return dacVal; }
    int8_t  getPerc()       { return perc; }
    int16_t getIdleI()      { return idleI; }   // Минимальный ток, при котором не нужна дополнительная нагрузка 
    int16_t getIdleDac()    { return idleDac; } // Ток в коде DAC

  private:
    uint8_t pcfOut;

    /* Set cycles that measurement operation takes the result from touchRead, threshold and detection
    *  accuracy depend on these values. Defaults are 0x1000 for measure and 0x1000 for sleep.
    *  With default values touchRead takes 0.5ms */
    const uint16_t time_measure = 0x1000; //0x1000;
    const uint16_t time_sleep   = 0x1000; //0x1000;

      // Время ожидания ответа по UART2
    #ifdef UART2
      const unsigned long time_out = 200;
    #endif
    
    // uint16_t touchInput = weight_ratio_no;      //
    // int holdTime = 0;                           // Время удержания кнопки ( * nu )

    // MF52AT MF52 B 3950 NTC термистор 2% 10 kOm
    const float reference_resistance = 10000.0f;    // R2 10kOm 1%
    const float nominal_resistance   = 10000.0f;    //    10kOm 2%
    const float nominal_temperature  =    25.0f;
    const float b_value              =  3950.0f;

    float celsius = 25.0;

    float readVoltage( int adc );
    float readSteinhart( const int adc );

// ================= Это всё дубли, окончательно они в tools 

    // // Test
    // uint8_t  swOnOff   = (uint8_t)false;
    uint16_t pwmVal    = 0x0335;
    uint16_t dacVal    = 0x0222;
    uint8_t  perc      = 50;
    uint16_t idleI     = 0x0555;        // Минимальный ток, при котором не нужна дополнительная нагрузка 
    uint16_t idleDac   = 0x0107;        // Ток в коде DAC

    // Пороги отключения                   default
    int16_t winLU     =  -200;          // 0xFF38;
    int16_t winUpU    = 18000;          // 0x4650;
    int16_t winLI     = -1500;          // 0xFA24;
    int16_t winUpI    =  5000;          // 0x1388;


// ======================================================================

};

#endif // !_MBOARD_H_
