/*
*
*
* июль 2022
*/
#include "mdispatcher.h"
#include "mtools.h"
#include "board/mboard.h"
#include "board/msupervisor.h"
//#include "board/mkeyboard.h"
#include "measure/mkeyboard.h"
#include "display/mdisplay.h"
#include <string.h>
#include "modes/bootfsm.h"
#include "modes/optionsfsm.h"
#include "modes/pidtestfsm.h"
#include "modes/templatefsm.h"
#include "modes/cccvfsm.h"
#include "modes/cccvtfsm.h"
#include "modes/dischargefsm.h"
#include "modes/devicefsm.h"
#include "Arduino.h"

MDispatcher::MDispatcher(MTools * tools) :
Tools(tools), Board(tools->Board), Display(tools->Display)
{
  char sLabel[ MDisplay::MaxString ] = { 0 };
  strcpy(sLabel, " MESP32 v0.1");
  Display->showLabel( sLabel );

  latrus = Tools->readNvsBool("device", "local", true );
  //modeSelection = Tools->readNvsInt ( MNvs::nQulon, MNvs::kQulonMode, 0 );   // Индекс массива
modeSelection = BOOT;
sync = true;

    // Tools->paramMult = Tools->readNvsShort("device", "p_mult", 0x0100);
    // Tools->paramMax  = Tools->readNvsShort("device", "p_max",  0x00FF);
    // Tools->pidHz     = Tools->readNvsShort("device", "p_hz",   10);
    // #ifdef PRINTDISP
    //   Serial.print("\nparamMult=0x"); Serial.print(Tools->paramMult, HEX);
    //   Serial.print("\nparamMax=0x"); Serial.print(Tools->paramMax, HEX);
    //   Serial.print("\npidHz=0x"); Serial.print(Tools->pidHz, HEX);
    // #endif



  textMode( modeSelection );

//  Tools->postpone = Tools->readNvsInt("options", "postpone",  1 );

}

  // Выдерживается период запуска 100мс для вычисления амперчасов
void MDispatcher::run()
{
    // Индикация при инициализации процедуры выбора режима работы
  Display->showVolt(Tools->getRealVoltage(), 3);
  Display->showAmp (Tools->getRealCurrent(), 3);

  if (State)
  {
    // Работаем с FSM
    MState * newState = State->fsm();      
    if (newState != State)                  // State изменён!
    {
      delete State;
      State = newState;
    } 
    // Если будет 0, на следующем цикле увидим
  }
  else // State не определён (0) - выбираем или показываем режим
  {
    if (sync)
    {
      Board->buzzerOn();
      State = new MBoot::MStart(Tools);
      sync = false;
    }
    else  
    {
      if (Tools->Keyboard->getKey(MKeyboard::B_CLICK))
      {
        Tools->writeNvsInt( "device", "mode", modeSelection );  // Запомнить крайний выбор режима
 
        // Serial.print("Available heap: "); Serial.println(ESP.getFreeHeap());
        // Serial.print("Core ID: ");        Serial.println(xPortGetCoreID());

        switch (modeSelection)
        {
          case BOOT:        State = new MBoot::MStart(Tools);     break;
          case OPTIONS:     State = new MOption::MStart(Tools);   break;
          case PIDTEST:     State = new MPidtest::MStart(Tools);  break;
          case TEMPLATE:    State = new Template::MStart(Tools);  break;
          case CCCV:        State = new MCccv::MStart(Tools);     break;
          case CCCVT:        State = new MCccvt::MStart(Tools);     break;
          case DISCHARGE:   State = new MDisch::MStart(Tools);    break;
          case DEVICE:      State = new MDevice::MStart(Tools);   break;
          default:                                                break;
        }
      } // !B_CLICK

      if (Tools->Keyboard->getKey(MKeyboard::UP_CLICK))
      { 
        Board->buzzerOn();
        if (modeSelection == (int)DEVICE) modeSelection = OPTIONS;  // Исключена возможность выбора BOOT'а
        else modeSelection++;
        textMode( modeSelection );
      }

      if (Tools->Keyboard->getKey(MKeyboard::DN_CLICK))
      {
        Board->buzzerOn();
        if (modeSelection == (int)OPTIONS) modeSelection = DEVICE;  // Исключена возможность выбора BOOT'а
        else modeSelection--;
        textMode( modeSelection );
      }
    }
  }
}

void MDispatcher::textMode(short modeSelection)
{
  char sMode[MDisplay::MaxString] = {0};
  char sHelp[MDisplay::MaxString] = {0};

  switch(modeSelection)
  {
    case BOOT:
      sprintf(sMode, "       BOOT:      ");   // Только 17 знакомест для этого дисплея,
      sprintf(sHelp, "    ...WAIT...    ");   // в 18-й вводить пробел
    break;

    case OPTIONS:
      sprintf(sMode, "     OPTIONS:     ");
      sprintf(sHelp, " USER PARAM...S   ");
    break;

    case PIDTEST:
      sprintf(sMode, "      PIDTEST:     ");
      sprintf(sHelp, "  ADJ  KP, PD, KI  ");
    break;

    case TEMPLATE:
      sprintf(sMode, "    TEMPLATE:     " );
      sprintf(sHelp, "     EXAMPLE      " );
    break;

    case CCCV:
      sprintf(sMode, "   CC/CV CHARGE:   ");
      sprintf(sHelp, "     B-SELECT      ");
    break;

    case CCCVT:
      sprintf(sMode, "      CC/CVT:      ");
      sprintf(sHelp, "     B-SELECT      ");
    break;

    case DISCHARGE:
      sprintf(sMode, "    DISCHARGE:     ");
      sprintf(sHelp, "     B-SELECT      ");
    break;

    case DEVICE:
      sprintf(sMode, "      DEVICE:      ");   // Настройки с доступом (?) разработчика (заводской доступ)
      sprintf(sHelp, "  CALIBRATION ETC  ");
    break;

    default:
      sprintf(sMode, "      ERROR:       " );
      sprintf(sHelp, "   UNIDENTIFIED    " );
    break;
  }

  Display->showMode( sMode );
  Display->showHelp( sHelp );
  //  Serial.println( sMode);
}
