#ifndef _MCOMMANDS_H_
#define _MCOMMANDS_H_
/*
  19.07.2022
*/

#include "stdint.h"

class MTools;
class MBoard;
class MWake;

class MCommands
{
  public:
    MCommands(MTools * tools);
    ~MCommands();

    void doCommand();
    short dataProcessing();

  private:
    MTools * Tools;   // = nullptr;
    MBoard * Board;
    MWake  * Wake;    // = nullptr;

  public:
      enum ROLES
    {
      RS = 0,    // режим прямого регулирования
      RU,        // режим управления напряжением
      RI,        // режим управления током
      RD         // режим управления током разряда
    };
  
  private:
      //Команды запроса данных в реальном времени и могут быть фоновыми
    void doGetUIS();                    // 0x10 Чтение напряжения (мВ), тока (мА), состояния
    void doGetU();                      // 0x11 Чтение напряжения (мВ)                        nu
    void doGetI();                      // 0x12 Чтение тока (мА)                              nu
    void doGetUI();                     // 0x13 Чтение напряжения (мВ) и тока (мА)            nu
    void doGetState();                  // 0x14 Чтение состояния                              nu
    void doGetCelsius();                // 0x15 Чтение температуры радиатора

      // Целевые команды, вызываются конечными автоматами
        // Команды управления
    void doPowerAuto();                 // 0x20   
    void doPowerStop();                 // 0x21
    void doPowerMode();                 // 0x22
    void doDischargeGo();               // 0x24   

        // Команды работы с измерителем напряжения 
    void doGetFactorU();                // 0x30   
    void doSetFactorU();                // 0x31   
    void doSetFactorDefaultU();         // 0x32   
    void doGetSmoothU();                // 0x33   
    void doSetSmoothU();                // 0x34   
    void doGetOffsetU();                // 0x35   
    void doSetOffsetU();                // 0x36   
      
        // Команды работы с измерителем тока
    void doGetFactorI();                // 0x38   
    void doSetFactorI();                // 0x39   
    void doSetFactorDefaultI();         // 0x3A   
    void doGetSmoothI();                // 0x3B   
    void doSetSmoothI();                // 0x3C   
    void doGetOffsetI();                // 0x3D   
    void doSetOffsetI();                // 0x3E   

        // Команды работы с регуляторами
    void doPidConfigure();              // 0x40   
    void doPidSetCoefficients();        // 0x41   
    void doPidOutputRange();            // 0x42   
    void doPidReconfigure();            // 0x43   
    void doPidClear();                  // 0x44   
    void doPidTest();                   // 0x46   
    void doPidGetParam();           // 0x47   
    void doPidGetConfigure();           // 0x48   
//    void doPidSetMaxSum();              // 0x49    nu

    void doCooler();                    // 0x4F Задать скорость вентилятора 
      
        // Команды работы с АЦП
    void doReadProbes();                // 0x50   
    void doAdcGetOffset();              // 0x51   nu
    void doAdcSetOffset();              // 0x52   nu
    void doAdcAutoOffset();             // 0x53   nu

        // Команды тестовые
    void doSwPin();                     // 0x54   nu
    void doSetPower();                  // 0x56   nu
    void doSetDischg();                 // 0x57   nu
    void doSetVoltage();                // 0x58   nu
    void doSetCurrent();                // 0x59   nu
    void doSetDiscurrent();             // 0x5A   nu
    void doSurgeCompensation();         // 0x5B   nu
    void doIdleLoad();                  // 0x5C   nu

        // Команды задания порогов отключения
    void doGetLtV();                    // 0x60
    void doSetLtV();                    // 0x61
    void doSetLtDefaultV();             // 0x62
    void doGetUpV();                    // 0x63
    void doSetUpV();                    // 0x64
    void doSetUpDefaultV();             // 0x65

    void doGetLtI();                    // 0x68
    void doSetLtI();                    // 0x69
    void doSetLtDefaultI();             // 0x6A
    void doGetUpI();                    // 0x6B
    void doSetUpI();                    // 0x6C
    void doSetUpDefaultI();             // 0x6D

        // Команды универсальные
    void doNop();                       // 0x00 nu
//    void doErr();                    // 0x01
//    void doEcho();                   // 0x02
    void doInfo();                        // 0x03 nu 

    void txU08(uint8_t id,  uint8_t value);
    void txU16(uint8_t id, uint16_t value);
    void txU32(uint8_t id, uint32_t value);

};

#endif  //!_MCOMMANDS_H_
