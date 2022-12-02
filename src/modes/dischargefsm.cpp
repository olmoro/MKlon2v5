/*
  Файл: cccvfsm.cpp
                          Как это работает.


  Версия от 07.08.2022
*/

#include "modes/dischargefsm.h"
#include "mtools.h"
#include "mcmd.h"
#include "board/mboard.h"
#include "board/msupervisor.h"
//#include "board/mkeyboard.h"
#include "measure/mkeyboard.h"

#include "display/mdisplay.h"
#include <Arduino.h>
#include <string>

namespace MDisch
{
  //========================================================================= MStart
    // Состояние "Старт", инициализация выбранного режима работы (DISCHARGE).
  MStart::MStart(MTools * Tools) : MState(Tools)
  {
    //Отключить на всякий пожарный
    Tools->txPowerStop();                                                   // 0x21  Команда драйверу

    maxI = Tools->readNvsFloat("discharge", "maxI", MConst::discharge_i_fixed);
    vTaskDelay(2 / portTICK_PERIOD_MS);
              #ifdef PRINT_DISCHARGE
                Serial.print("\nРежим DISCHARGE с параметрами:");
                Serial.print("\nПользовательские, а если нет, то разработчика:");
                Serial.print("\nmaxI="); Serial.print(maxI, 2);
              #endif
    cnt = 7;
    // Индикация
    Display->showMode((char*)"     DISCHARGE    ");
    Display->showHelp((char*)"    P-ADJ   C-GO  ");
    Display->barOff();
    Board->ledsOn();              // Подтверждение входа в настройки заряда белым свечением светодиода
  }
  MState * MStart::fsm()
  {
    switch (Keyboard->getKey())    //Здесь так можно
    {
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                        return new MStop(Tools);
      case MKeyboard::C_CLICK: Board->buzzerOn(); 
        // Старт без уточнения параметров
        maxI = MConst::discharge_i_fixed;
              #ifdef PRINT_DISCHARGE
                Serial.print("\n\nПредустановленные разработчиком:");
                Serial.print("\nmaxI="); Serial.print(maxI, 2);
                Serial.println();
              #endif
                                                                              return new MGo(Tools);
      case MKeyboard::P_CLICK: Board->buzzerOn();                             return new MSetCurrent(Tools);
      case MKeyboard::B_CLICK: Board->buzzerOn();
        if(--cnt <= 0)                                                        return new MClearDischargeKeys(Tools);
        break;
      default:;
    }
    // Индикация текущих значений, указывается число знаков после запятой
    Display->showVolt(Tools->getRealVoltage(), 3);
    Display->showAmp (Tools->getRealCurrent(), 3);
    return this;
  };  // MStart

  //========================================================================= MClearDischargeKeys
    // Состояние "Очистка всех ключей раздела".
  MClearDischargeKeys::MClearDischargeKeys(MTools * Tools) : MState(Tools)
  {
    Display->showMode((char*)"      CLEAR?      ");
    Display->showHelp((char*)"  P-NO     C-YES  ");
    Board->ledsBlue();
    cnt = 50;                                                                 // 5с 
  }
  MState * MClearDischargeKeys::fsm()
  {
    switch  (Keyboard->getKey())
    {
    case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                          return new MStop(Tools);
    case MKeyboard::P_CLICK: Board->buzzerOn();                               return new MSetCurrent(Tools);
    case MKeyboard::C_CLICK: Board->buzzerOn();
      done = Tools->clearAllKeys("discharge");
      vTaskDelay(2 / portTICK_PERIOD_MS);
            #ifdef TEST_KEYS_CLEAR
              Serial.print("\nAll keys \"discharge\": ");
              (done) ? Serial.println("cleared") : Serial.println("err");
            #endif
      break;
    default:;
    }
    if(--cnt <= 0)                                                            return new MStart(Tools);
    Display->showMode((char*)"     CLEARING     ");
    Display->showHelp((char*)"    ___WAIT___    ");
    return this;
  };  // MClearDischargeKeys

  //========================================================================= MSetCurrent
    // Состояние "Установка максимального тока разряда, А".
MSetCurrent::MSetCurrent(MTools * Tools) : MState(Tools)
{
  maxI = Tools->readNvsFloat("discharge", "maxI", MConst::discharge_i_fixed);

  Display->showMode((char*)" DISCHARGE_I  +/- ");
  Display->showHelp((char*)" B-SAVE      C-GO ");
  //Board->ledsOn();
}
MState * MSetCurrent::fsm()
  {
    switch (Keyboard->getKey())
    {
        // Отказ от продолжения ввода параметров - стоп
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                        return new MStop(Tools);
        // Отказ от дальнейшего ввода параметров - исполнение
      case MKeyboard::C_CLICK: Board->buzzerOn();                             return new MGo(Tools);
//      case MKeyboard::P_CLICK: Board->buzzerOn();                             return new M...(Tools);
        // Сохранить и перейти к следующему параметру
      case MKeyboard::B_CLICK: Board->buzzerOn();
        Tools->writeNvsFloat("discharge", "maxI", maxI);                           return new MGo(Tools);
      case MKeyboard::UP_CLICK: Board->buzzerOn();
        maxI = Tools->updnFloat(maxI, below, above, 0.1f);
        break;
      case MKeyboard::DN_CLICK: Board->buzzerOn();
        maxI = Tools->updnFloat(maxI, below, above, -0.1f);
        break;
      default:;
    }

    // Считать ампер-часы

    // Показать и продолжить
    Display->showVolt(Tools->getRealVoltage(), 3);
    Display->showAmp(maxI, 1);
    return this;
  };

  //========================================================================= MGo
    // Состояние "Включение цепи разряда".
MGo::MGo(MTools * Tools) : MState(Tools)
{
  spD = Tools->readNvsFloat("discharge", "maxI", MConst::discharge_i_fixed);
              vTaskDelay(2 / portTICK_PERIOD_MS);
              #ifdef PRINT_DISCHARGE
                Serial.print("\nРежим DISCHARGE с выбранными параметрами:");
                Serial.print("\nspD="); Serial.print(spD, 2);
              #endif

      // Обнуляются счетчики времени и отданного заряда
    Tools->clrTimeCounter();
    Tools->clrAhCharge();

Board->ledsGreen();
  Tools->txDischargeGo(spD);        // Команда драйверу 0x24
}
MState * MGo::fsm()
  {
    Tools->chargeCalculations();                                              // Подсчет отданных ампер-часов.

    switch (Keyboard->getKey())
    {
        // Отказ от продолжения ввода параметров - стоп
      case MKeyboard::C_LONG_CLICK: 
      case MKeyboard::C_CLICK: Board->buzzerOn();                        return new MStop(Tools);


      // case MKeyboard::P_CLICK: Board->buzzerOn();
      // // Отключить и вернуться

      case MKeyboard::UP_CLICK: Board->buzzerOn();
        spD = Tools->updnFloat(spD, below, above, 0.1f);
          Tools->txDischargeGo(spD);        // Команда драйверу 0x24

        break;
      case MKeyboard::DN_CLICK: Board->buzzerOn();
        spD = Tools->updnFloat(spD, below, above, -0.1f);
          Tools->txDischargeGo(spD);        // Команда драйверу 0x24

        break;
      default:;
    }
    Display->showVolt(Tools->getRealVoltage(), 2);
    Display->showAmp (Tools->getRealCurrent(), 2);
    Display->initBar(TFT_GREEN);
    Display->showDuration(Tools->getChargeTimeCounter(), MDisplay::SEC);
    Display->showAh(Tools->getAhCharge());
    return this;
  };  // MGo


  //========================================================================= MStop

  // Завершение режима заряда - до нажатия кнопки "С" удерживается индикация 
  // о продолжительности и отданном заряде.
  // Состояние: "Завершение заряда"
  MStop::MStop(MTools * Tools) : MState(Tools)
  {

    Tools->txPowerStop();                                                     // 0x21 Команда драйверу отключить преобразователь

    Display->showMode((char*)"   DISCHARGE OFF  ");
    Display->showHelp((char*)"      C-EXIT      ");
    Display->barStop();
    Board->ledsRed();
  }    
  MState * MStop::fsm()
  {
    switch (Keyboard->getKey())
    {
      case MKeyboard::C_CLICK:  Board->buzzerOn();                            return new MExit(Tools);
      default:;
    }
    Display->showVolt(Tools->getRealVoltage(), 3);
    Display->showAmp (Tools->getRealCurrent(), 3);
    return this;
  };  // MStop

  //========================================================================= MExit
  // Процесс выхода из режима разряда - до нажатия кнопки "С" удерживается индикация о продолжительности и отданном заряде.
  // Состояние: "Индикация итогов и выход из режима заряда в меню диспетчера" 
  MExit::MExit(MTools * Tools) : MState(Tools)
  {
    //Tools->shutdownCharge();
    Display->showMode((char*)"    MODE OFF     ");
    Display->showHelp((char*)" C-TO SELECT MODE ");
//    Board->ledsOn();
    Board->ledsOff();
    Display->barOff();
  }    
  MState * MExit::fsm()
  {
    switch ( Keyboard->getKey() )
    {
      case MKeyboard::P_CLICK:  Board->buzzerOn();                            return new MStart(Tools); // Вернуться в начало
      case MKeyboard::C_CLICK:  Board->buzzerOn(); 
        Board->ledsOff();
        // Надо бы восстанавливать средствами диспетчера...
        Display->showMode( (char*) "   MODE     +/-   " );
        Display->showHelp( (char*) "     B-SELECT     " );                    return nullptr;  // Возврат к выбору режима
      default:;
    }
    return this;
  };  // MExit


};  // !Конечный автомат режима разряда DISCHARGE.

