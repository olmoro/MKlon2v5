/*
 * templatefsm.cpp – шаблон конечного автомата.
 * На дисплее при выборе режима отображается как "   Пример FSM   ", которое
 * надо указать в диспетчере выбора режима, как и его позицию в меню.
 * Выбор режима производится коротким нажатием на "В" – "Выбор"
 * В примере наглядно реализованы переключения между состояниями
 * оператором с использованием длинных и коротких нажатий или по таймеру по 
 * числу отработанных циклов.
*/

#include "modes/templatefsm.h"
//#include "nvs.h"
#include "mtools.h"
    #include "driver/mcommands.h"
#include "board/mboard.h"
#include "board/msupervisor.h"
//#include "board/mkeyboard.h"
#include "measure/mkeyboard.h"

#include "display/mdisplay.h"
#include <Arduino.h>
#include <string>

// Переименуйте поле имен для вашего режима.
namespace Template
{
      // Старт
  MStart::MStart(MTools * Tools) : MState(Tools) 
  {
    // Здесь, в конструкторе класса для этого состояния, разместите код инициализации 
    // состояния MStart, который будет исполняться всякий раз при активации этого режима.
    // Для других состояний - при входе из иного состояния.
    cnt = 7;
        // Индикация
    Display->showMode( (char*) "     TEMPLATE     " );
    Display->showHelp( (char*) "  P-YELL B-BLUE   " );    // Подсказка какие кнопки активны
    Display->barOff();
    Board->ledsOn();         // Светодиод светится белым как индикатор входа в режим
  }
  MState * MStart::fsm()
  {
    // В любом состоянии короткое нажатие на кнопку "C" резервируем для выхода из режима.
    // Диспетчер режимов производит вход каждые 100 миллисекунд. 
    switch ( Keyboard->getKey() )
    {
      case MKeyboard::C_CLICK:        // Досрочное прекращение заряда оператором
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                      return new MStop(Tools);

      case MKeyboard::P_CLICK: Board->buzzerOn();                           return new MYellow(Tools);      // Выбран желтый

      // case MKeyboard::B_CLICK: Board->buzzerOn();
      // return new MBlue(Tools);        // Выбран синий

      case MKeyboard::B_CLICK: Board->buzzerOn();
        if(--cnt <= 0)                                                      return new MClearCccvKeys(Tools);
        break;
      default:;
    }
    
    return this;             // Ничего не выбрано, через 100мс проверять снова.
  };

    //================================================================================ MClearCccvKeys

  MClearCccvKeys::MClearCccvKeys(MTools * Tools) : MState(Tools)
  {
    Display->showMode((char*)"      CLEAR?      ");   // В каком режиме
    Display->showHelp((char*)"  P-NO     C-YES  ");   // Активные кнопки
    Board->ledsBlue();
    cnt = 50;                                         // 5с 
  }
  MState * MClearCccvKeys::fsm()
  {
    switch  (Keyboard->getKey())
    {
    case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                  return new MStop(Tools);
    case MKeyboard::P_CLICK: Board->buzzerOn();                       return new MYellow(Tools);
    case MKeyboard::C_CLICK: Board->buzzerOn();
      done = Tools->clearAllKeys("template");
      vTaskDelay(2 / portTICK_PERIOD_MS);
      #ifdef TEST_KEYS_CLEAR
        Serial.print("\nAll keys \"template\": ");
        (done) ? Serial.println("cleared") : Serial.println("err");
      #endif
      break;
    default:                                                          break;
    }
    if(--cnt <= 0)                                                    return new MStart(Tools);
    Display->showMode((char*)"     CLEARING     ");   // В каком режиме
    Display->showHelp((char*)"    ___WAIT___    ");   // Активные кнопки - нет
    return this;
  };

      // Желтый
  MYellow::MYellow(MTools * Tools) : MState(Tools)
  {
    Display->showMode( (char*) "   LED YELLOW     " );
    Display->showHelp( (char*) "  UP-RED  DN-GRN  " );    // Подсказка какие кнопки активны
    Board->ledsOff();    Board->ledROn();   Board->ledGOn();
  } 
  MState * MYellow::fsm()
  {
    switch ( Keyboard->getKey() )
    {
      case MKeyboard::C_CLICK:      // Досрочное прекращение заряда оператором
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();
      return new MStop(Tools);

      case MKeyboard::UP_CLICK: Board->buzzerOn();
      return new MRed(Tools);       // Выбран красный

      case MKeyboard::DN_CLICK: Board->buzzerOn();
      return new MGreen(Tools);     // Выбран зеленый
      default:;
    }

    return this;                    // Ничего не выбрано, через 100мс проверять снова.
  };

      // Красный
  MRed::MRed(MTools * Tools) : MState(Tools)
  {
    Display->showMode( (char*) "     LED  RED     " );
    Display->showHelp( (char*) "   LONG B-START   " );    // Подсказка какие кнопки активны
    Board->ledsOff();    Board->ledROn();
  }     
  MState * MRed::fsm()
  {
    switch ( Keyboard->getKey() )
    {
      case MKeyboard::C_CLICK:        // Досрочное прекращение заряда оператором
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();
      return new MStop(Tools);

      case MKeyboard::B_CLICK: Board->buzzerOn();
      return new MStart(Tools);       // Выбран Старт, белый
      default:;
    }    
       
    return this;                      // Ничего не выбрано, через 100мс проверять снова.
  };

      // Зеленый
  MGreen::MGreen(MTools * Tools) : MState(Tools) 
  {
    Display->showMode( (char*) "    BLINK GRN     " );
    Display->showHelp( (char*) "  LONG B-R  B-Y   " );    // Подсказка какие кнопки активны
    cnt = duration;
  } 
  MState * MGreen::fsm()
  {
    switch ( Keyboard->getKey() )
    {
      case MKeyboard::C_CLICK:        // Досрочное прекращение заряда оператором
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();
      return new MStop(Tools);

      case MKeyboard::B_LONG_CLICK: Board->buzzerOn();
      return new MRed(Tools);         // Выбран красный

      case MKeyboard::B_CLICK: Board->buzzerOn();
      return new MYellow(Tools);      // Выбран желтый
      default:;
    }
    if( --cnt <= 0 ) 
    return new MStop(Tools);          // Безусловный выход через 10 секунд

    Board->blinkGreen( cnt );         // Мигать зеленым

    return this;                      // Ничего не выбрано, через 100мс проверять снова.
  };

      // Синий
  MBlue::MBlue(MTools * Tools) : MState(Tools) 
  {
    Display->showMode( (char*) "    LED  BLUE     " );
    Display->showHelp( (char*) "   LONG B-START   " );    // Подсказка какие кнопки активны
    Board->ledsOff();    Board->ledBOn();
  }
  MState * MBlue::fsm()
  {
    if( Keyboard->getKey(MKeyboard::C_CLICK) ) return new MStop(Tools);       // Выбран выход
    if( Keyboard->getKey(MKeyboard::B_LONG_CLICK) ) return new MStart(Tools); // Выбран Старт, белый
    return this;                    // Ничего не выбрано, через 100мс проверять снова.
  };

  // Состояние выхода из режима заряда – до короткого нажатия кнопки "С" 
  // Почему не сделали это в предыдущем шаге? Для удержания индикация о 
  // продолжительности и отданном заряде как итоге, например  
  MStop::MStop(MTools * Tools) : MState(Tools)
  {
    Display->showMode( (char*) "      GOODBYE     " );
    Display->showHelp( (char*) "       C-EXIT     " );    // Подсказка какие кнопки активны
    //???Tools->shutdownCharge();   // Как пример выхода из режима заряда командой драйверу силовой платы
    Board->ledsRed();

  }
  MState * MStop::fsm()
  {
    if(Keyboard->getKey(MKeyboard::C_CLICK)) { 
        //Tools->activateExit(" ");    // Можно сделать лучше, гасит светодиоды
    Board->ledsOff();
        Display->showMode( (char*) "     TEMPLATE:    " );
        Display->showHelp( (char*) "     B-SELECT     " );

        return nullptr;              // Для возврата к выбору режима возвращается nullptr;. 
    }
    return this;
  };

};
