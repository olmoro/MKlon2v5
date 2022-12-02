#ifndef _MTOOLS_H_
#define _MTOOLS_H_

#include "stdint.h"

class MBoard;
class MDisplay;
class MKeyboard;
class Preferences;

class MTools
{
  public:
    MTools(MBoard * board, MDisplay * display);
    ~MTools();
   
    //pointers are public - easier to access
    MBoard      * Board        = nullptr; // external
    MDisplay    * Display      = nullptr; // external
    MKeyboard   * Keyboard     = nullptr; // local
    Preferences * qPreferences = nullptr; // local

    // Буфер команд(ы) на исполнение, при необходимости будет очередь (queue) 
    uint8_t commBuff  = 0x00;   

    short error;              // Код ошибки обработки команды драйвером
    void  setErr(short err);
    short getErr();

    // Локализация
    bool getLocalization() const;
    void setLocalization(bool);
    
    // Флаг блокировки обмена с драйвером на время его рестарта
    bool getBlocking();
    void setBlocking(bool);

    // Флаг выбора режима коррекции смещения АЦП  nu?
    bool getTuningAdc();
    void setTuningAdc(bool);

    // Настройки АЦП
    short offsetAdc = 0x0000;
    short adcV      = 0x0000;
    short adcI      = 0x0000;

    short getAdcV();
    short getAdcI();
    
    // Переменные настроек измерителей
    short factorV   = 0x2DA0;   // Коэффициент преобразования в милливольты
    short smoothV   = 0x0003;   // Коэффициент фильтрации
    short shiftV    = 0x0000;   // Начальное смещение в вольтах

    short factorI   = 0x030C;   // Коэффициент преобразования в миллиамперы
    short smoothI   = 0x0003;   // Коэффициент фильтрации
    short shiftI    = 0x0000;   // Начальное смещение в миллиамперах

    // ========== Управление ПИД-регулятором, частота фиксирована ==========

    //bool setPidCoefficients(float kp, float ki, float kd);


      // Pid parameters 
    uint16_t setpoint   = 0x0800;     // 
    uint16_t setpointU  = 0x3390;     // 13200
    uint16_t setpointI  = 0x0BB8;     //  3000

    unsigned short pidMode    = 0x01;       // 1 - начать с регулирования по напряжению
    unsigned short kp         = 0x0190;
    unsigned short ki         = 0x00C0;
    unsigned short kd         = 0x0190;
    unsigned short minOut     = 0x0220;
    unsigned short maxOut     = 0x1000;

    // PWM
    uint8_t  pwmInvert = (uint8_t)false;   // Выбор полярности PWM (v55: для отключения при сбросе - 0x00)
    uint16_t pwmPeriod = 0x1012;           // Выбор частоты (через период)

    uint8_t  swOnOff   = (uint8_t)false;

    //   // Предвычисления коэффициентов
    // void  setPid( float kp, float ki, float kd );    // KP, KI, KD 

    void    setMilliVolt(short val);
    short   getMilliVolt();
    void    setMilliAmper(short val);
    short   getMilliAmper();

      // Текущие целочисленные в мВ и мА напряжение и ток преобразуются в вольты и амперы 
    void    setVoltageVolt(short);
    float   getVoltageVolt();
    void    setCurrentAmper(short);
    float   getCurrentAmper();
    void    setCelsius(short val);

    void    setState(unsigned short val);
    unsigned short getState();

    //  void  setProtErr(uint8_t val);  // protocol error - или подтверждения исполнения команды 

    float getRealVoltage(); //+m
    float getRealCurrent(); //+m

    short getParamMult();
    void  setParamMult(short pm);

    short postpone = 0;      // Заданная задержка включения (отложенный старт), ч
    void  setPostpone(short hour);
    short getPostpone();

    bool getAP();

      // Чтение и запись NVS
    bool  readNvsBool  (const char * name, const char * key, const bool  defaultValue );
    short readNvsShort (const char * name, const char * key, const short defaultValue );
    int   readNvsInt   (const char * name, const char * key, const int   defaultValue );
    float readNvsFloat (const char * name, const char * key, const float defaultValue );

    void writeNvsBool  (const char * name, const char * key, const bool  bValue );
    void writeNvsShort (const char * name, const char * key, const short sValue );
    void writeNvsInt   (const char * name, const char * key, const int   iValue );
    void writeNvsFloat (const char * name, const char * key, const float fValue );

    float copyNvsFloat (const char * src, const char * dst, const char * key, const float defaultValue );
    float copyNvsFloat (const char * src, const char * dst, const char * key, const char * newkey, const float defaultValue );

    // 202207
    short updnInt(short value, short below, short above, short additives);
    float updnFloat(float value, float below, float above, float additives);

    bool  clearAllKeys (const char * name);
    void  removeKey(const char * name, const char * key);

    void clrTimeCounter();
    void clrChargeTimeCounter();
    void clrAhCharge();
    int  getTimeCounter();
    void setTimeCounter( int ivalue );

    uint8_t getBuffCmd();
    void    setBuffCmd(uint8_t cmd);

    short getCooler();
    void  setCooler(short val);

      // АЦП - настройки
    void  setAdcV(short val);
    void  setAdcI(short val);
    short getAdcOffset();
    void  setAdcOffset(short val);

    short getLtV();
    short getUpV();
    short getLtI();
    short getUpI();

    // ======================== ЦЕЛЕВЫЕ КОМАНДЫ УПРАВЛЕНИЯ ДРАЙВЕРОМ SAMD21 M0 MINI ========================

      // Команды чтения результатов измерений
    void txReadUIS();                                       // 0x10;
    void txGetU();                                          // 0x11 Чтение напряжения (мВ)
    void txGetI();                                          // 0x12 Чтение тока (мА)
    void txGetUI();                                         // 0x13 Чтение напряжения (мВ) и тока (мА)
    void txGetState();                                      // 0x14 Чтение состояния
    void txCelsius();                                       // 0x15 Чтение температуры радиатора
      
      // 
    void txPowerAuto(float spV, float spI);                 // 0x20
    void txPowerStop();                                     // 0x21
    void txPowerMode(float spV, float spI, uint8_t mode);   // 0x22
    void txDischargeGo(float spI);                          // 0x24

      // Команды работы с измерителем напряжения
        // Множитель преобразования в милливольты
    void txGetFactorU();                                    // 0x30 Чтение
    void txSetFactorU(short val);                           // 0x31 Запись
    void txSetFactorDefaultU();                             // 0x32 Возврат к заводскому
        // Параметр сглаживания
    void txGetSmoothU();                                    // 0x33 Чтение
    void txSetSmoothU(short val);                           // 0x34 Запись
        // Приборный сдвиг
    void txGetShiftU();                                     // 0x35 Чтение
    void txSetShiftU(short val);                            // 0x36 Запись

      // Команды работы с измерителем тока
        // Множитель преобразования в миллиамперы
    void txGetFactorI();                                    // 0x38 Чтение
    void txSetFactorI(short val);                           // 0x39 Запись
    void txSetFactorDefaultI();                             // 0x3A Возврат к заводскому
        // Параметр сглаживания
    void txGetSmoothI();                                    // 0x3B Чтение
    void txSetSmoothI(short val);                           // 0x3C Запись
        // Приборный сдвиг
    void txGetShiftI();                                     // 0x3D Чтение
    void txSetShiftI(short val);                            // 0x3E Запись

      // Команды работы с ПИД-регулятором
    void txSetPidConfig(uint8_t m, float kp, float ki, float kd, uint16_t minOut, uint16_t maxOut);   // 0x40 Запись

    void txSetPidCoeff(unsigned short m, float kp, float ki, float kd);    // 0x41 Запись
    void txSetPidCoeffV(float kp, float ki, float kd);      // 0x41 Запись
    void txSetPidCoeffI(float kp, float ki, float kd);      // 0x41 Запись
    void txSetPidCoeffD(float kp, float ki, float kd);      // 0x41 Запись

    void txSetPidOutputRange(uint8_t m, uint16_t minOut, uint16_t maxOut);                               // 0x42
    void txSetPidReconfig(uint8_t m, float kp, float ki, float kd, uint16_t minOut, uint16_t maxOut);    // 0x43, w/o clear
    void txPidClear();                                      // 0x44

    void txGetPidParam();                                   // 0x47 Get param  _mult ...
    void txGetPidConfig();                                  // 0x48 get mode, kP, kI, kD, min, max - возвращает параметры текущего режима регулирования
    void txSetCooler(short val);                            // 0x4F Задать скорость вентилятора

    void txGetProbes();                                     // 0x50
    void txGetAdcOffset();                                  // 0x51
    void txSetAdcOffset(short val);                         // 0x52
    void txAdcAutoOffset();                                 // 0x53 (пока в резерве)


      // Команды тестовые (отменены?)
    // const uint8_t cmd_set_switch_pin            = 0x54; // sw_pin D4 PA14

    // const uint8_t cmd_set_power                 = 0x56; // пользоваться с осторожностью - выяснение пределов регулирования
    // const uint8_t cmd_set_discharge             = 0x57; // не проверена
    // const uint8_t cmd_set_voltage               = 0x58; // старая, не проверена
    // const uint8_t cmd_set_current               = 0x59; // старая, не проверена 
    // const uint8_t cmd_set_discurrent            = 0x5A; // старая, не проверена
    // const uint8_t cmd_set_surge_compensation    = 0x5B; // параметры подавления всплеска напряжения na
    // const uint8_t cmd_set_idle_load             = 0x5C; // параметры доп.нагрузки ХХ

    // Команды задания порогов отключения
    void txGetLtV();                                        // 0x60
    void txSetLtV(short val);                               // 0x61
    void txSetLtDefaultV(short val);                        // 0x62
    void txGetUpV();                                        // 0x63
    void txSetUpV(short val);                               // 0x64
    void txSetUpDefaultV(short val);                        // 0x65

    void txGetLtI();                                        // 0x68
    void txSetLtI(short val);                               // 0x69
    void txSetLtDefaultI(short val);                        // 0x6A
    void txGetUpI();                                        // 0x6B
    void txSetUpI(short val);                               // 0x6C
    void txSetUpDefaultI(short val);                        // 0x6D

    // =====================================================================================================

    // Подсчет ампер-часов
    void  zeroAhCounter();                            // Обнуление счетчика ампер-часов заряда
    int   getChargeTimeCounter();
    float getAhCharge();
    void  chargeCalculations();

      // Подсчет задержки пуска
    bool  postponeCalculation();
  
    // ========================== FastPID ==========================
      /* Прототип: A fixed point PID controller with a 32-bit internal calculation pipeline.
      https://github.com/mike-matera/FastPID/tree/master/examples/VoltageRegulator
      Константы синхронизированы с ведомым контроллером (20220715)
    */  
    short paramMult = 0x0100;   // Полученный от драйвера
    short paramMax  = 0x00FF;   // Полученный от драйвера
    short pidHz     = 10;       // Полученный от драйвера
  

    static constexpr unsigned short status_switch           = 1U<<15; 
    static constexpr unsigned short status_power            = 1U<<14; 
    static constexpr unsigned short status_current_control  = 1U<<13; 
    static constexpr unsigned short status_voltage_control  = 1U<<12; 
    static constexpr unsigned short status_charge           = 1U<<11; 
    static constexpr unsigned short status_discharge        = 1U<<10; 
    static constexpr unsigned short status_auto_mode        = 1U<<9; 
    static constexpr unsigned short status_pid              = 1U<<8;
    static constexpr unsigned short status_overheating      = 1U<<7; 
    static constexpr unsigned short status_overload         = 1U<<6; 
    static constexpr unsigned short status_power_limit      = 1U<<5; 
    static constexpr unsigned short status_reverse_polarity = 1U<<4; 
    static constexpr unsigned short status_short_circuit    = 1U<<3; 
    static constexpr unsigned short status_calibration      = 1U<<2; 
    static constexpr unsigned short status_upgrade          = 1U<<1; 
    static constexpr unsigned short status_reserve2         = 1U; 

    static constexpr unsigned short status_pid_voltage     = status_switch |
                                                              status_power |
                                                              status_voltage_control |
                                                              status_charge |
                                                              status_pid;   // учесть status_auto_mode

    static constexpr unsigned short status_pid_current      = status_switch |
                                                              status_power |
                                                              status_current_control |
                                                              status_charge |
                                                              status_pid;


  private:
    //==== PRIVATE ==== PRIVATE ==== PRIVATE ==== PRIVATE ==== PRIVATE ==== PRIVATE ==== PRIVATE ====

    // Пороги отключения по умолчанию: мВ, мА  (имитация аппаратной поддержки)
    static constexpr short lt_default_v  =  -200;   // при переплюсовке
    static constexpr short up_default_v  = 18000;   // исполняется конструктивно
    static constexpr short lt_default_i  = -1500;   // максимальный ток разряда
    static constexpr short up_default_i  =  6000;   // максимальный ток заряда
      // Пороги отключения: мВ, мА  (имитация аппаратной поддержки)
    short ltV = lt_default_v;
    short upV = up_default_v;
    short ltI = lt_default_i;
    short upI = up_default_i;

    short milliVolt   = 0;        // Принятое от драйвера напряжение в милливольтах
    short milliAmper  = 0;        // Принятый от драйвера ток в миллиамперах

    float voltage     = 0.0f;    // Напряжение на клеммах аккумулятора, В
    float current     = 0.0f;    // Текущий измеренный ток, А

    unsigned short state = 0x0000;
    short celsius     = 0x0000;   // Температура радиатора в "попугаях" АЦП

      // Подсчет ампер-часов
    float ahCharge          = 0.0f;
    int   timeCounter       = 0;
    int   chargeTimeCounter = 0;

    //    void printAll( const char * s );

    //================= Measures =====================
        int   keyCode = 0;    


      short cool = 0;           // Скорость вентилятора - уточнить

      bool blocking;    // 
      bool tuningAdc  = false;  // Флаг подстройки смещения АЦП
};

#endif //_MTOOLS_H_