/*
  Файл: optionsfsm.cpp
                          Как это работает.
  ВВод и коррекция пользовательских параметров реализованы в виде конечного автомата, 
инициализация которого производится посредством выбора "OPTIONS" в меню диспетчера.
  Начальное состояние MStart в свою очередь предоставляет оператору в виде подсказок 
назначение сенсорных кнопок. Программисту доступны короткое и длинное "нажатие", 
автонажатие (здесь не востребовано). Подсказки вида "C-EXIT" и "C*EXIT" означают, что 
короткое или длинное нажатие соответственно вызывают переход к указанному состоянию. 
  По понятным причинам не все возможности отображены в подсказках. Например, по кнопке 
"B" при вводе параметров обычно производится сохранение параметра с переходом к 
следующему параметру, а "P" - возврат к редактированию предыдущего. Длинное "C" в 
любом случае инициирует выход из режима ввода параметров, а в этом случае "p" вернёт
к редактированию первого параметра, а не предыдущего.

  Вариант 20220804 - в разработке ...
*/

#include "modes/optionsfsm.h"
#include "mtools.h"
#include "mcmd.h"
#include "board/mboard.h"
//#include "board/mkeyboard.h"
#include "measure/mkeyboard.h"

#include "display/mdisplay.h"
#include <Arduino.h>
#include <string>

namespace MOption
{
  //========================================================================= MStart
  // Состояние "Старт", инициализация выбранного режима работы.
  MStart::MStart(MTools * Tools) : MState(Tools)
  {
    #ifdef PRINT_OPTION
      Serial.println("\nOptions: Start");
      Serial.print("\npostpone="); Serial.print(Tools->readNvsShort("options",  "postpone", MConst::postpone_fixed));
      Serial.print("\ntimeout=");  Serial.print(Tools->readNvsShort("options",  "timeout",  MConst::timeout_fixed));
      Serial.print("\nnominalV="); Serial.print((Tools->readNvsFloat("options", "nominalV", MConst::nominal_v_fixed)), 1);
      Serial.print("\ncapacity="); Serial.print((Tools->readNvsFloat("options", "capacity", MConst::capacity_fixed)), 1);
    #endif
    cnt = 7;
      // Индикация
    Display->showMode((char*) " OPTIONS SELECTED ");
    Display->showHelp((char*) "  P-PPONE C-EXIT  ");
    Board->ledsOn();
  }
  MState * MStart::fsm()
  {
    switch ( Keyboard->getKey() )
    {
      case MKeyboard::C_CLICK: Board->buzzerOn();                             return new MStop(Tools);
      case MKeyboard::P_CLICK: Board->buzzerOn();                             return new MSetPostpone(Tools);
      case MKeyboard::B_CLICK: Board->buzzerOn();
        if(--cnt <= 0)                                                        return new MClearOptionsKeys(Tools);
        break;
      default:;
    }
    Display->showVolt(Tools->getRealVoltage(), 3);
    Display->showAmp (Tools->getRealCurrent(), 3);
    return this;
  };  // MStart

  //========================================================================= MClearOptionsKeys
  // Состояние "Очистка всех ключей раздела NVS "options" "

  MClearOptionsKeys::MClearOptionsKeys(MTools * Tools) : MState(Tools)
  {
    Display->showMode((char*)"      CLEAR?      ");
    Display->showHelp((char*)"  P-NO     C-YES  ");
    Board->ledsBlue();
    cnt = 50;                                                                 // 5с 
  }
  MState * MClearOptionsKeys::fsm()
  {
    switch (Keyboard->getKey())
    {
    case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                          return new MStop(Tools);
    case MKeyboard::P_CLICK: Board->buzzerOn();                               return new MSetPostpone(Tools);
    case MKeyboard::C_CLICK: Board->buzzerOn();
      done = Tools->clearAllKeys("options");
            #ifdef TEST_KEYS_CLEAR
              vTaskDelay(2 / portTICK_PERIOD_MS);
              Serial.print("\nAll keys \"options\": ");
              (done) ? Serial.println("cleared") : Serial.println("err");
            #endif
      break;
    default: break;
    }
    if(--cnt <= 0)                                                            return new MStart(Tools);
    Display->showMode((char*)"     CLEARING     ");   // В каком режиме
    Display->showHelp((char*)"    ___WAIT___    ");   // Активные кнопки - нет
    return this;
  };  // MClearOptionsKeys

  //========================================================================= MSetPostpone
  // Состояние "Ввод времени отложенного старта, час"
  MSetPostpone::MSetPostpone(MTools * Tools) : MState(Tools) 
  {
    postpone = Tools->readNvsShort("options", "postpone", MConst::postpone_fixed);
    #ifdef PRINT_OPTION
      Serial.print("\npostpone="); Serial.print(postpone);
    #endif
      // Подсказка
    Display->showMode((char*) "  POSTPONE      +/-  ");
    Display->showHelp((char*) "      B-SAVE C-EXIT  ");
    Board->ledsGreen();
  }
  MState * MSetPostpone::fsm()
  {
    switch (Keyboard->getKey())
    {
    case MKeyboard::C_CLICK: Board->buzzerOn();                          return new MStop(Tools);
    case MKeyboard::P_CLICK: Board->buzzerOn();                               return new MTimeout(Tools);
    case MKeyboard::B_CLICK: Board->buzzerOn();       
      Tools->writeNvsShort("options", "postpone", postpone);
      #ifdef PRINT_OPTION
        postpone = Tools->readNvsShort("options", "postpone", MConst::postpone_fixed);
        //vTaskDelay(2 / portTICK_PERIOD_MS);
        Serial.print("\nSaved postpone="); Serial.print(postpone);
      #endif
                                                                              return new MTimeout(Tools);
    case MKeyboard::UP_CLICK: Board->buzzerOn();
      postpone = Tools->updnInt(postpone, below, above, +1);
      //Tools->setPostpone(postpone);
      break;
    case MKeyboard::DN_CLICK: Board->buzzerOn();
      postpone = Tools->updnInt(postpone, below, above, -1);
      //Tools->setPostpone(postpone);
      break;
    default:;
    }
    //Display->showDuration(Tools->getPostpone() * 3600, MDisplay::SEC);
    Display->showDuration(postpone * 3600, MDisplay::SEC);
    return this;
  };  // MSetPostpone

  //========================================================================= MTimeout
  // Состояние "Ввод максимального времени заряда, час" 
  MTimeout::MTimeout(MTools * Tools) : MState(Tools) 
  {
    timeout = Tools->readNvsShort("options", "timeout", MConst::timeout_fixed);
          #ifdef PRINT_OPTION
            vTaskDelay(2 / portTICK_PERIOD_MS);
            Serial.print("\ntimeout="); Serial.print(timeout);
          #endif
    Display->showMode((char*) "  TIMEOUT       +/-  ");
    Display->showHelp((char*) "      B-SAVE C-EXIT  ");
    Board->ledsGreen();
  }
  MState * MTimeout::fsm()
  {
    switch (Keyboard->getKey())
    {
    case MKeyboard::C_CLICK: Board->buzzerOn();                          return new MStop(Tools);
    case MKeyboard::P_CLICK: Board->buzzerOn();                               return new MSetPostpone(Tools);
    case MKeyboard::B_CLICK: Board->buzzerOn();       
      Tools->writeNvsShort("options", "timeout", timeout);
            #ifdef PRINT_OPTION
              timeout = Tools->readNvsShort("options", "timeout", MConst::timeout_fixed);
              Serial.print("\nSaved timeout="); Serial.print(timeout);
            #endif
                                                                              return new MNominalV(Tools);
    case MKeyboard::UP_CLICK: Board->buzzerOn();
      timeout = Tools->updnInt(timeout, below, above, +1);
      Tools->setTimeCounter(timeout);
      break;
    case MKeyboard::DN_CLICK: Board->buzzerOn();
      timeout = Tools->updnInt(timeout, below, above, -1);
      Tools->setTimeCounter(timeout);
      break;
    default:;
    }
    Display->showDuration(timeout * 3600, MDisplay::SEC);
    return this;
  };  // MTimeout

  //========================================================================= MNominalV
  // Состояние "Выбор количества банок в заряжаемом аккумуляторе для расчета
  // номинального напряжения."
  MNominalV::MNominalV(MTools * Tools) : MState(Tools)
  {
    val = Tools->readNvsFloat("options", "nominalV", MConst::nominal_v_fixed);

    Display->showMode((char*) "  NOMINAL_V     +/-  ");
    Display->showHelp((char*) "      B-SAVE C-EXIT  ");
    Board->ledsGreen();
  }
  MState * MNominalV::fsm()
  {
  switch (Keyboard->getKey())
    {
    case MKeyboard::C_CLICK: Board->buzzerOn();                          return new MStop(Tools);
    case MKeyboard::P_CLICK: Board->buzzerOn();                               return new MTimeout(Tools);
    case MKeyboard::B_CLICK: Board->buzzerOn();       
      Tools->writeNvsFloat("options", "nominalV", val);                       
            #ifdef PRINT_OPTION
              val = Tools->readNvsShort("options", "nominalV", MConst::nominal_v_fixed);
              Serial.print("\nSaved nominalV="); Serial.print(val);
            #endif      
                                                                              return new MCapacity(Tools);
    case MKeyboard::UP_CLICK: Board->buzzerOn();
      val = Tools->updnFloat(val, below, above, +2.0f);
      break;
    case MKeyboard::DN_CLICK: Board->buzzerOn();
      val = Tools->updnFloat(val, below, above, -2.0f);
      break;
    default:;
    }

    Display->showVolt(val, 1);
    Display->showAmp (Tools->getRealCurrent(), 3);


    return this;
  };  // MNominalV

  //========================================================================= MCapacity
  // Состояние "Выбор емкости батареи для расчета рекомендуемого максимального
  // тока заряда" 
  MCapacity::MCapacity(MTools * Tools) : MState(Tools)
  {
    val = Tools->readNvsFloat("options", "capacity", MConst::capacity_fixed);

    Display->showMode((char*) "  CAPACITY      +/-  ");
    Display->showHelp((char*) "      B-SAVE C-EXIT  ");
    Board->ledsGreen();
  }
  MState * MCapacity::fsm()
  {
  switch (Keyboard->getKey())
    {
    case MKeyboard::C_CLICK: Board->buzzerOn();                          return new MStop(Tools);
    case MKeyboard::P_CLICK: Board->buzzerOn();                               return new MNominalV(Tools);
    case MKeyboard::B_CLICK: Board->buzzerOn();       
      Tools->writeNvsFloat("options", "capacity", val);
            #ifdef PRINT_OPTION
              val = Tools->readNvsShort("options", "capacity", MConst::capacity_fixed);
              Serial.print("\nSaved capacity="); Serial.print(val);
            #endif
                                                                              return new MNext(Tools);
    case MKeyboard::UP_CLICK: Board->buzzerOn();
      val = Tools->updnFloat(val, below, above, +5.0f);
      break;
    case MKeyboard::DN_CLICK: Board->buzzerOn();
      val = Tools->updnFloat(val, below, above, -5.0f);
      break;
    default:;
    }

    return this;
  };  //MCapacity

  //========================================================================= MNext
  // Состояние " 
  MNext::MNext(MTools * Tools) : MState(Tools)
  {


    Display->showMode((char*) "  ...NEXT...    +/-  ");
    Display->showHelp((char*) "      B-SAVE C-EXIT  ");
    Board->ledsGreen();
  }
  MState * MNext::fsm()
  {
  switch (Keyboard->getKey())
    {
    case MKeyboard::C_CLICK: Board->buzzerOn();                          return new MStop(Tools);
    case MKeyboard::P_CLICK: Board->buzzerOn();                               return new MTimeout(Tools);
    case MKeyboard::B_CLICK: Board->buzzerOn();       
      //Tools->writeNvsShort("options", "...", val);                    
                                                                              return new MNominalV(Tools);
    case MKeyboard::UP_CLICK: Board->buzzerOn();
      val = Tools->updnInt(val, below, above, +1);
      //Tools->set...(val);
      break;
    case MKeyboard::DN_CLICK: Board->buzzerOn();
      val = Tools->updnInt(val, below, above, -1);
      //Tools->set...(val);
      break;
    default:;
    }

    return this;
  };  // MNext





  //========================================================================= MLast
  // Состояние " 

  MLast::MLast(MTools * Tools) : MState(Tools)
  {


    Display->showMode((char*) "  ...LAST...    +/-  ");
    Display->showHelp((char*) "        B-SAVE C-EX  ");
    Board->ledsGreen();
  }
  MState * MLast::fsm()
  {
  switch (Keyboard->getKey())
    {
    case MKeyboard::C_CLICK: Board->buzzerOn();                          return new MStop(Tools);
    case MKeyboard::P_CLICK: Board->buzzerOn();                               return new MNext(Tools);
    case MKeyboard::B_CLICK: Board->buzzerOn();       
      //Tools->writeNvsShort("options", "...", val);                    return new MNext(Tools);
    case MKeyboard::UP_CLICK: Board->buzzerOn();
      val = Tools->updnInt(val, below, above, +1);
      //Tools->set...(val);                                         break;
    case MKeyboard::DN_CLICK: Board->buzzerOn();
      val = Tools->updnInt(val, below, above, -1);
      //Tools->set...(val);                                         break;
    default:;
    }

    return this;
  };  // MLast




  //========================================================================= MStop
  // Состояние "Завершение режима - до нажатия кнопки "С" предоставляется
  // возможность вернуться в нвчало.
  MStop::MStop(MTools * Tools) : MState(Tools)
  {
    //Tools->shutdownCharge();

    Display->showMode((char*) "   OPTIONS OFF    ");
    Display->showHelp((char*) "      C-STOP      ");
    Board->ledsRed();
  }    
  MState * MStop::fsm()
  {
    switch ( Keyboard->getKey() )
    {
      case MKeyboard::C_CLICK:  Board->buzzerOn();                            return new MExit(Tools);
      case MKeyboard::P_CLICK:  Board->buzzerOn();                            return new MStart(Tools);  // Вернуться в начало
      default:;
    }
    return this;
  };  //MStop

  //========================================================================= MExit
  // Состояние "Выход из режима"
  MExit::MExit(MTools * Tools) : MState(Tools)
  {
    // Индикация помощи
    Display->showMode((char*) "     OPTIONS      ");
    Display->showHelp((char*) " C-RET TO SELECT  ");
  }      
  MState * MExit::fsm()
  {
    switch ( Keyboard->getKey() )
    {
      case MKeyboard::C_CLICK: Board->buzzerOn();
        Board->ledsOff();
        // Надо бы восстанавливать средствами диспетчера...
        Display->showMode((char*) "     OPTIONS:     ");
        Display->showHelp((char*) "   +/- B-SELECT   ");
      return nullptr;                             // Возврат к выбору режима
      default:;
    }
    return this;
  };  // MExit

}; // MOption
