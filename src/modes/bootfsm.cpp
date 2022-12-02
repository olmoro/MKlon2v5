/*
  syncingfsm.cpp
  Конечный автомат синхронизации данных между контроллерами.
            Как это работает.
    Процесс синхронизации (BOOT) запускается диспетчером при инициализации прибора. 
  Пользовательские параметры (OPTIONS) и параметры разработчика (DEVICE) восстанавливаются из
  энергонезависимой памяти ESP32 и одновременнно передаются на драйвер SAMD21, где заменяют 
  соответствующие дефолтные значения. Во время синхронизации на дисплей выводится информация 
  о ходе синхронизации. По окончании процесса синхронизации прибор готов к работе в выбранном режиме.
  07.07.2022 - 
*/

#include "modes/bootfsm.h"
#include "mdispatcher.h"
#include "mtools.h"
#include "mcmd.h"
#include "board/mboard.h"
#include "board/msupervisor.h"
//#include "board/mkeyboard.h"
#include "measure/mkeyboard.h"
#include "display/mdisplay.h"
#include <Arduino.h>
#include <string>

namespace MBoot
{
  // Старт и инициализация выбранного режима работы.
  MStart::MStart(MTools * Tools) : MState(Tools) 
  {
    Board->ledsBlue();                              // Подтверждение входа синим свечением светодиода как и любую загрузку
    Tools->Keyboard->getKey(MKeyboard::UP_CLICK);
  }
  MState * MStart::fsm()
  {
    Tools->setBlocking(true);                                                 // Блокировать обмен
    vTaskDelay(1000/portTICK_PERIOD_MS);                                      // Не беспокоим драйвер 3 секунды после рестарта 
    Tools->setBlocking(false);                                                // Разблокировать обмен
    return new MTxPowerStop(Tools);                                           // Перейти к следующему параметру
  };

  MTxPowerStop::MTxPowerStop(MTools * Tools) : MState(Tools) {}
  MState * MTxPowerStop::fsm()
  {
    Tools->txPowerStop();                                                     // 0x21  Команда драйверу
    return new MTxSetMult(Tools);
  };

  // Восстановление множителя, максимума и частоты для обмена с драйвером.
  MTxSetMult::MTxSetMult(MTools * Tools) : MState(Tools) {}
  MState * MTxSetMult::fsm()
  {
    Tools->txGetPidParam();                                                   // 0x47  Команда драйверу
    return new MTxsetFactorV(Tools);
  };

  // Восстановление пользовательского (или заводского) коэфициента преобразования в милливольты.
  MTxsetFactorV::MTxsetFactorV(MTools * Tools) : MState(Tools) {}
  MState * MTxsetFactorV::fsm()
  {
    //Tools->factorV = Tools->readNvsShort("device", "factorV", 0x2DA0);        // Взять сохраненное из nvs.
    //Tools->factorV = Tools->readNvsShort("device", "factorV", 0x43C7);        // Взять сохраненное из nvs.
    //Tools->factorV = Tools->readNvsShort("device", "factorV", 0x2CEC);        // Взять сохраненное из nvs.
    Tools->factorV = Tools->readNvsShort("device", "factorV", 0x4A4A);        // Взять сохраненное из nvs.
    Tools->txSetFactorU(Tools->factorV);                                      // 0x31  Команда драйверу
    return new MTxSmoothV(Tools);                                             // Перейти к следующему параметру
  };

  // Восстановление пользовательского (или заводского) коэффициента фильтрации по напряжению.
  MTxSmoothV::MTxSmoothV(MTools * Tools) : MState(Tools) {}
  MState * MTxSmoothV::fsm()
  {
    Tools->smoothV = Tools->readNvsShort("device", "smoothV", 0x0003);        // Взять сохраненное из nvs.
    Tools->txSetSmoothU(Tools->smoothV);                                      // 0x34  Команда драйверу
    return new MTxShiftV(Tools);                                              // Перейти к следующему параметру
  };

  // Восстановление пользовательской (или заводской) настройки сдвига по напряжению.
  MTxShiftV::MTxShiftV(MTools * Tools) : MState(Tools) {}
  MState * MTxShiftV::fsm()
  {
    Tools->shiftV = Tools->readNvsShort("device", "offsetV", 0x0000);         // Взять сохраненное из nvs.
    Tools->txSetShiftU(Tools->shiftV);                                        // 0x36  Команда драйверу
    return new MTxFactorI(Tools);                                             // Перейти к следующему параметру
  };

  // Восстановление пользовательского (или заводского) коэфициента преобразования в миллиамперы.
  MTxFactorI::MTxFactorI(MTools * Tools) : MState(Tools) {}
  MState * MTxFactorI::fsm()
  {
    //Tools->factorI = Tools->readNvsShort("device", "factorI", 0x079D);        // Взять сохраненное из nvs.
    Tools->factorI = Tools->readNvsShort("device", "factorI", 0x030D);        // Взять сохраненное из nvs.
    Tools->txSetFactorI(Tools->factorI);                                      // 0x39  Команда драйверу
    return new MTxSmoothI(Tools);                                             // Перейти к следующему параметру
  };

  // Восстановление пользовательского (или заводского) коэффициента фильтрации по току.
  MTxSmoothI::MTxSmoothI(MTools * Tools) : MState(Tools) {}
  MState * MTxSmoothI::fsm()
  {
    Tools->smoothI = Tools->readNvsShort("device", "smoothI", 0x0003);        // Взять сохраненное из nvs.
    Tools->txSetSmoothI(Tools->smoothI);                                      // 0x3C  Команда драйверу
    return new MTxShiftI(Tools);                                              // Перейти к следующему параметру
  };

  // Восстановление пользовательской (или заводской) настройки сдвига по току.
  MTxShiftI::MTxShiftI(MTools * Tools) : MState(Tools) {}
  MState * MTxShiftI::fsm()
  {
    Tools->shiftI = Tools->readNvsShort("device", "offsetI", 0x0000);         // Взять сохраненное из nvs.
    Tools->txSetShiftI(Tools->shiftI);                                        // 0x3E  Команда драйверу
    return new MExit(Tools);      //return new MTxPidConfigure(Tools); // Перейти к следующему параметру (Временно закончить)
  };

  // Процесс выхода из режима "BOOT".
  // Состояние: "Индикация итогов и выход из режима заряда в меню диспетчера" 
  MExit::MExit(MTools * Tools) : MState(Tools)
  {Display->showHelp((char*) "   ...READY...    " );}    
  MState * MExit::fsm()
  {
    Board->ledsOff();
    Display->showVolt(Tools->getRealVoltage(), 3);
    Display->showAmp (Tools->getRealCurrent(), 3);
    return nullptr;                                                           // Возврат к выбору режима
  }
};  //MExit

// !Конечный автомат синхронизации данных между контроллерами.
