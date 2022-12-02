/*
    Набор методов, доступных разработчику для программирования собственных
 режимов работы прибора.

              !!!!!!!!!!!!!!!!!!!!!!!!!!!! 2022 август
    Продолжается очистка от мусора проекта-прототипа, который был без драйвера.          
*/

#include "mtools.h"
#include "mcmd.h"
#include "driver/mcommands.h"
#include "board/mboard.h"
//#include "board/mkeyboard.h"
#include "measure/mkeyboard.h"

#include "display/mdisplay.h"
//#include "driver/mcommands.h"
#include <Preferences.h>
#include "Arduino.h"

    
MTools::MTools(MBoard * board, MDisplay * display) : 
  Board(board),
  Display(display),
  Keyboard(new MKeyboard),
  qPreferences(new Preferences) {}

MTools::~MTools()
{
  delete qPreferences;
  delete Keyboard;
}
  //analog:
  //MTools::MTools(MBoard * board, MDisplay * display, MKeyboard * keyboard) 
  //{
  //      Board = board;
  //      Display = display; 
  //      Keyboard = keyboard;
  //}


    // Флаг блокировки обмена с драйвером на время его рестарта
bool MTools::getBlocking() {return blocking;}
void MTools::setBlocking(bool bl) {blocking = bl;}

    // Флаг выбора режима коррекции смещения АЦП
bool MTools::getTuningAdc() {return tuningAdc;}
void MTools::setTuningAdc(bool tu) {tuningAdc = tu;}

void  MTools::setPostpone(short hour) {postpone = hour;}
short MTools::getPostpone() {return postpone;}


  // Напряжение и ток без преобразования
void    MTools::setMilliVolt(short val)   { milliVolt = val; voltage = (float)val / 1000;}
short   MTools::getMilliVolt()            { return milliVolt; }
void    MTools::setMilliAmper(short val)  { milliAmper = val; current = (float)val / 1000;}
short   MTools::getMilliAmper()           { return milliAmper; }

  // Текущие целочисленные в мВ и мА напряжение и ток преобразуются в вольты и амперы 
void    MTools::setVoltageVolt(short val)   { voltage = (float)val / 1000; }
float   MTools::getVoltageVolt()            { return voltage; }
void    MTools::setCurrentAmper(short val)  { current = (float)val / 1000; }
float   MTools::getCurrentAmper()           { return current; }

void    MTools::setCelsius(short val)       { celsius = val; }

void  MTools::setState(unsigned short val)  {state = val;}
unsigned short MTools::getState()           {return state;}

void  MTools::setErr(short err) { error = err; }
short MTools::getErr()          { return error; }

bool  MTools::getAP() { return false; }

float MTools::getRealVoltage() { return voltage; }  //-21
float MTools::getRealCurrent() { return current; }  //-21

  // Подсчет ампер-часов
void  MTools::zeroAhCounter() { timeCounter = 0; ahCharge = 0.0; }      // Обнуление счетчика ампер-часов заряда
int   MTools::getChargeTimeCounter() { return chargeTimeCounter; }
float MTools::getAhCharge()    { return ahCharge; }
void  MTools::chargeCalculations()
{
  timeCounter++;
  chargeTimeCounter = ((int)timeCounter / 10);
    //ahCharge += Tools->getRealCurrent() / 36000.0;     //    ((float)( 1000 / simpleChargerPeriod ) * 3600.0);
  ahCharge += current / 36000.0;     //    ((float)( 1000 / simpleChargerPeriod ) * 3600.0);
}

// ==================================== Nvs read ====================================

bool MTools::readNvsBool(const char * name, const char * key, const bool defaultValue)
{
  qPreferences->begin(name, true);                      // R-mode (second parameter has to be true).
  bool val = qPreferences->getBool(key, defaultValue);
  qPreferences->end();                                  // Close the Preferences
  return val;  
}

short MTools::readNvsShort(const char * name, const char * key, const short defaultValue)
{
  qPreferences->begin(name, true);
  short val = qPreferences->getShort(key, defaultValue);
  qPreferences->end();
  return val;  
}

int MTools::readNvsInt(const char * name, const char * key, const int defaultValue)
{
  qPreferences->begin(name, true);
  int val = qPreferences->getInt(key, defaultValue);
  qPreferences->end();
  return val;  
}

float MTools::readNvsFloat(const char * name, const char * key, const float defaultValue)
{
  qPreferences->begin(name, true);
  float val = qPreferences->getFloat(key, defaultValue);
  qPreferences->end();
  return val;
}

// ==================================== Nvs write ====================================
void MTools::writeNvsBool(const char * name, const char * key, const bool bValue)
{
  qPreferences->begin(name, false);                     // RW-mode (second parameter has to be false).
  qPreferences->putBool(key, bValue);
  qPreferences->end();
}

void MTools::writeNvsShort(const char * name, const char * key, const short sValue)
{
  qPreferences->begin(name, false);
  qPreferences->putShort(key, sValue);
  qPreferences->end();
}

void MTools::writeNvsInt(const char * name, const char * key, const int iValue)
{
  qPreferences->begin(name, false);
  qPreferences->putInt(key, iValue);
  qPreferences->end();
}

void MTools::writeNvsFloat(const char * name, const char * key, const float fValue)
{
  qPreferences->begin(name, false);
  qPreferences->putFloat(key, fValue);
  qPreferences->end();
}

// ================================= Nvs copy/move ===================================
float MTools::copyNvsFloat (const char * src, const char * dst, const char * key, const float defaultValue )
{
  qPreferences->begin(src, false);
  float val = qPreferences->getFloat(key, defaultValue);

  qPreferences->begin(dst, false);
  qPreferences->putFloat(key, val);
  qPreferences->end();
  return val;
}

float MTools::copyNvsFloat (const char * src, const char * dst, const char * key, const char * newkey, const float defaultValue )
{
  qPreferences->begin(src, false);
  float val = qPreferences->getFloat(key, defaultValue);

  qPreferences->begin(dst, false);
  qPreferences->putFloat(newkey, val);
  qPreferences->end();
  return val;
}

// ===================== Clear all keys in opened preferences ========================
bool MTools::clearAllKeys(const char * name)
{
  qPreferences->begin(name, false);                     // RW-mode (second parameter has to be false).
  bool err = qPreferences->clear();
  qPreferences->end();
  return err;
}

// =================================  Remove a key ===================================
void MTools::removeKey(const char * name, const char * key)
{
  qPreferences->begin(name, false);
  qPreferences->remove(key);
  qPreferences->end();
}


void MTools::clrTimeCounter() { timeCounter = 0; }
void MTools::clrChargeTimeCounter() { chargeTimeCounter = 0; }
void MTools::clrAhCharge() { ahCharge = 0; }
int  MTools::getTimeCounter() { return timeCounter; }
void MTools::setTimeCounter( int ivalue ) { timeCounter = ivalue; }


uint8_t buffCmd = MCmd::cmd_nop;             // 0x00 - нет операции

uint8_t MTools::getBuffCmd()            {return buffCmd;}
void    MTools::setBuffCmd(uint8_t cmd) {buffCmd = cmd;}

short MTools::getParamMult() {return paramMult;}
void  MTools::setParamMult(short _pm) {paramMult = _pm;}

short MTools::getCooler() {return cool;}
void  MTools::setCooler(short val) {cool = val;}

void  MTools::setAdcV( short val ) { adcV = val; }
void  MTools::setAdcI( short val ) { adcI = val; }

short MTools::getAdcOffset() {return offsetAdc;}
void  MTools::setAdcOffset(short val) {offsetAdc = val;}

short MTools::getAdcV() {return adcV;}
short MTools::getAdcI() {return adcI;}

short MTools::getLtV() {return ltV;}
short MTools::getUpV() {return upV;}
short MTools::getLtI() {return ltI;}
short MTools::getUpI() {return upI;}

// ======================== ЦЕЛЕВЫЕ КОМАНДЫ УПРАВЛЕНИЯ ДРАЙВЕРОМ SAMD21 MO MINI ========================
  //Команды чтения результатов измерений:
void MTools::txReadUIS()                                {buffCmd = MCmd::cmd_get_uis;}                  // 0x10;
void MTools::txGetU()                                   {buffCmd = MCmd::cmd_get_u;}                    // 0x11 Чтение напряжения (мВ)
void MTools::txGetI()                                   {buffCmd = MCmd::cmd_get_i;}                    // 0x12 Чтение тока (мА)
void MTools::txGetUI()                                  {buffCmd = MCmd::cmd_get_ui;}                   // 0x13 Чтение напряжения (мВ) и тока (мА)
void MTools::txGetState()                               {buffCmd = MCmd::cmd_get_state;}                // 0x14 Чтение состояния
void MTools::txCelsius()                                {buffCmd = MCmd::cmd_get_celsius;}              // 0x15 Чтение температуры радиатора

  // Команда управления PID-регулятором заряда    0x20
void MTools::txPowerAuto(float spV, float spI)
{
  setpointU = (short)(spV * 1000);
  setpointI = (short)(spI * 1000);
  buffCmd   = MCmd::cmd_power_auto;
} 

void MTools::txPowerStop()                              {buffCmd = MCmd::cmd_power_stop;}               // 0x21


  // Команда управления PID-регулятором с выбором режима                                                // 0x22
void MTools::txPowerMode(float spV, float spI, uint8_t mode)
{
  setpointU = (short)(spV * 1000);
  setpointI = (short)(spI * 1000);
  pidMode   = mode;                   // Выбор режима старта
  buffCmd   = MCmd::cmd_power_mode;
}

  // Команда управления pid-регулятором разряда                                                         // 0x24
void MTools::txDischargeGo(float spI)
{
  setpointI = (short)(spI * 1000);
  buffCmd = MCmd::cmd_discharge_go;
}                       

  // Команды управления измерителями:
    // Множитель преобразования в милливольты
void MTools::txGetFactorU()                             {buffCmd = MCmd::cmd_read_factor_u;}            // 0x30 Чтение
void MTools::txSetFactorU(short val) {factorV = val;     buffCmd = MCmd::cmd_write_factor_u;}           // 0x31 Запись
void MTools::txSetFactorDefaultU()                      {buffCmd = MCmd::cmd_write_factor_default_u;}   // 0x32 Возврат к заводскому
    // Параметр сглаживания по напряжению
void MTools::txGetSmoothU()                             {buffCmd = MCmd::cmd_read_smooth_u;}            // 0x33 Чтение
void MTools::txSetSmoothU(short val) {smoothV = val;     buffCmd = MCmd::cmd_write_smooth_u;}           // 0x34 Запись
    // Приборное смещение по напряжению
void MTools::txGetShiftU()                              {buffCmd = MCmd::cmd_read_offset_u;}            // 0x35 Чтение
void MTools::txSetShiftU(short val)  {shiftV  = val;     buffCmd = MCmd::cmd_write_offset_u;}           // 0x36 Запись
    // Множитель преобразования в миллиамперы
void MTools::txGetFactorI()                             {buffCmd = MCmd::cmd_read_factor_i;}            // 0x38 Чтение
void MTools::txSetFactorI(short val) {factorI = val;     buffCmd = MCmd::cmd_write_factor_i;}           // 0x39 Запись
void MTools::txSetFactorDefaultI()                      {buffCmd = MCmd::cmd_write_factor_default_i;}   // 0x3A Возврат к заводскому
    // Параметр сглаживания по току
void MTools::txGetSmoothI()                             {buffCmd = MCmd::cmd_read_smooth_i;}            // 0x3B Чтение
void MTools::txSetSmoothI(short val) {smoothI = val;     buffCmd = MCmd::cmd_write_smooth_i;}           // 0x3C Запись
    // Приборное смещение по току
void MTools::txGetShiftI()                              {buffCmd = MCmd::cmd_read_offset_i;}            // 0x3D Чтение
void MTools::txSetShiftI(short val)  {shiftI  = val;     buffCmd = MCmd::cmd_write_offset_i;}           // 0x3E Запись

  // Команды работы с ПИД-регулятором:
void MTools::txSetPidConfig(uint8_t _m, float _kp, float _ki, float _kd, uint16_t _minOut, uint16_t _maxOut)
{
  pidMode = _m;
  kp      = (unsigned short) (_kp * paramMult);
  ki      = (unsigned short)((_ki * paramMult) / pidHz);
  kd      = (unsigned short)((_kd * paramMult) * pidHz);
  minOut  = _minOut;
  maxOut  = _maxOut;
  buffCmd = MCmd::cmd_pid_configure;                                                                    // 0x40 Запись
}

void MTools::txSetPidCoeff(unsigned short m, float _kp, float _ki, float _kd)    // 0x41 Запись
{
    pidMode = m;
    kp      = (unsigned short) (_kp * paramMult);
    ki      = (unsigned short)((_ki * paramMult) / pidHz);
    kd      = (unsigned short)((_kd * paramMult) * pidHz);
    buffCmd = MCmd::cmd_pid_write_coefficients;                                                         // 0x41 Запись
}

void MTools::txSetPidCoeffV(float _kp, float _ki, float _kd)
{
    pidMode = 1;
    kp      = (unsigned short) (_kp * paramMult);
    ki      = (unsigned short)((_ki * paramMult) / pidHz);
    kd      = (unsigned short)((_kd * paramMult) * pidHz);
    buffCmd = MCmd::cmd_pid_write_coefficients;                                                         // 0x41 Запись
}

void MTools::txSetPidCoeffI(float _kp, float _ki, float _kd)
{
    pidMode = 2;
    kp      = (unsigned short) (_kp * paramMult);
    ki      = (unsigned short)((_ki * paramMult) / pidHz);
    kd      = (unsigned short)((_kd * paramMult) * pidHz);
    buffCmd = MCmd::cmd_pid_write_coefficients;                                                         // 0x41 Запись
}

void MTools::txSetPidCoeffD(float _kp, float _ki, float _kd)
{
    pidMode = 3;
    kp      = (unsigned short) (_kp * paramMult);
    ki      = (unsigned short)((_ki * paramMult) / pidHz);
    kd      = (unsigned short)((_kd * paramMult) * pidHz);
    buffCmd = MCmd::cmd_pid_write_coefficients;                                                         // 0x41 Запись
}

void MTools::txSetPidOutputRange(uint8_t _m, uint16_t _minOut, uint16_t _maxOut)
{
    pidMode = _m;
    minOut  = _minOut;
    maxOut  = _maxOut;
    buffCmd = MCmd::cmd_pid_output_range;                                                               // 0x42 Запись
}

void MTools::txSetPidReconfig(uint8_t _m, float _kp, float _ki, float _kd, uint16_t _minOut, uint16_t _maxOut)
{
    pidMode = _m;
    kp      = (unsigned short) (_kp * paramMult);
    ki      = (unsigned short)((_ki * paramMult) / pidHz);
    kd      = (unsigned short)((_kd * paramMult) * pidHz);
    minOut  = _minOut;
    maxOut  = _maxOut;
    buffCmd = MCmd::cmd_pid_reconfigure;                                                                // 0x43 Запись
}

void MTools::txPidClear()                               {buffCmd = MCmd::cmd_pid_clear;}                // 0x44

void MTools::txGetPidParam()                            {buffCmd = MCmd::cmd_pid_read_param;}           // 0x47 Get param _mult ...

void MTools::txGetPidConfig()                           {buffCmd = MCmd::cmd_pid_read_configure;}       // 0x48 get mode, kP, kI, kD, min, max - возвращает параметры текущего режима регулирования

//   // Команда управления вентилятором
// void MTools::txSetCooler(short val)                     {cool = val; buffCmd = MCmd::cmd_set_cooler;}   // 0x4F Задать скорость вентилятора

  // Команды работы с АЦП
void MTools::txGetProbes()                              {buffCmd = MCmd::cmd_adc_read_probes;}          // 0x50
void MTools::txGetAdcOffset()                           {buffCmd = MCmd::cmd_adc_read_offset;}          // 0x51  
void MTools::txSetAdcOffset(short val) {offsetAdc = val; buffCmd = MCmd::cmd_adc_write_offset;}         // 0x52
void MTools::txAdcAutoOffset()                          {buffCmd = MCmd::cmd_adc_auto_offset;}          // 0x53 nu 

  // Команды управления тестовые

  // Команды задания порогов отключения
void MTools::txGetLtV()                            {buffCmd = MCmd::cmd_get_lt_v;}                      // 0x60
void MTools::txSetLtV(short val)        {ltV = val; buffCmd = MCmd::cmd_set_lt_v;}                      // 0x61
void MTools::txSetLtDefaultV(short val) {ltV = val; buffCmd = MCmd::cmd_set_lt_default_v;}              // 0x62
void MTools::txGetUpV()                            {buffCmd = MCmd::cmd_get_up_v;}                      // 0x63
void MTools::txSetUpV(short val)        {upV = val; buffCmd = MCmd::cmd_set_up_v;}                      // 0x64
void MTools::txSetUpDefaultV(short val) {upV = val; buffCmd = MCmd::cmd_set_up_default_v;}              // 0x65

void MTools::txGetLtI()                            {buffCmd = MCmd::cmd_get_lt_i;}                      // 0x68
void MTools::txSetLtI(short val)        {ltI = val; buffCmd = MCmd::cmd_set_lt_i;}                      // 0x69
void MTools::txSetLtDefaultI(short val) {ltI = val; buffCmd = MCmd::cmd_set_lt_default_i;}              // 0x6A
void MTools::txGetUpI()                            {buffCmd = MCmd::cmd_get_up_i;}                      // 0x6B
void MTools::txSetUpI(short val)        {upI = val; buffCmd = MCmd::cmd_set_up_i;}                      // 0x6C
void MTools::txSetUpDefaultI(short val) {upI = val; buffCmd = MCmd::cmd_set_up_default_i;}              // 0x6D


  // Подсчет задержки пуска
bool MTools::postponeCalculation()
{
    timeCounter--;
    chargeTimeCounter = timeCounter / 10;   //2;
    if( chargeTimeCounter == 0 ) return true;
    return false;
}

  // 202207 
short MTools::updnInt(short value, short below, short above, short additives)
{
    value += additives;
    if(value > above) return above;
    if(value < below) return below;
    return value;
}

float MTools::updnFloat(float value, float below, float above, float additives)
{
    value += additives;
    if(value > above) return above;
    if(value < below) return below;
    return value;
}
