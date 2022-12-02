/*
  Файл: pidtestfsm.cpp
      Это инструмент разработчика, не более того, облегчающий настройку
                 ПИД-регуляторов напряжения и тока.

    Параметры ПИД-регуляторов сохраняются в разделе NVS "pidtest" под именами:
      "kpV", "kiV", "kdV" - коэффициенты для регулятора по напряжению;
      "kpI", "kiI", "kdI" - коэффициенты для регулятора по току.
    А также "spV", "spI" - уровни для напряжения и тока (для удобства).

    При входе имеется возможность выбрать между запуском с предустановленными (C) и 
  корректируемыми (P) уровнями напряжения и тока, а также удалить все сохраненные 
  параметры раздела NVS (7B). Напряжение и ток регулируются шагами по 0,1 или 1,0 
  вольт или ампер коротким или длинным нажатием на (+) или (-). 
    Переключение между вводами уровней напряжения и тока производится по (P) вплоть 
  до окончания их коррекции по (B), после чего уровни записываются в NVS и регулятор 
  включается. В зависимости от заданных уровней и подключенной нагрузки регулятор 
  включается в один из режимов поддержания напряжения или тока. Переход между 
  регуляторами сопровождается сменой свечения светодиода (крутим нагрузку или 
  следим за переключениями при заряде). Замечено, что для резисторной нагрузки 
  нужны несколько иные настройки ПИД-регуляторов, чем для аккумулятора.  
    Далее все регулировки можно производить "на лету", то есть включать и отключать 
  преобразователь (C), корректировать уровни напряжения и тока через (P) с 
  сохранением подобранных на этот момент коэффициентов.
    Подбор коэффициентов производится шагами по 0,01 (+ и -). Для этого сначала 
  (P*), (B*) или (C*) длинным нажатием выбирают соответствующий коэффициент KP, KI 
  или KD. Выбор регулятора производится автоматически, как и ограничение предельных 
  значений, но "вслепую", в чем видится как и недостаток, так и преимущество.
    Выход из тестового режима производить так: коротким (B), перейти в состояние 
  окончания подбора. При этом будет предложено экспортировать данные в любой из трёх 
  профилей, возможность возобновить подбор либо выйти в главное меню прибора.
    Подобранные коэффициенты использовать в целевых режимах, считав их из раздела 
  "pidtest#" и записав, например, в качестве параметра в состояние конечного автомата
  "cccv" под удобным именем. 

  Версия от  06.08.2022
*/

#include "modes/pidtestfsm.h"
#include "mtools.h"
#include "board/mboard.h"
//#include "board/mkeyboard.h"
#include "measure/mkeyboard.h"

#include "display/mdisplay.h"
#include <Arduino.h>

namespace MPidtest
{
  //========================================================================= MStart
    // Состояние "Старт", инициализация выбранного режима работы PIDTEST.
  MStart::MStart(MTools * Tools) : MState(Tools)
  {
    mode = Tools->readNvsShort("pidtest", "mode", MConst::fixedM);
    cnt = 7;
    Tools->txPowerStop();                                                     // 0x21  Перейти в безопасный режим
    Display->showMode((char*)"      PIDTEST     ");
    Display->showHelp((char*)"    P-ADJ   C-GO  ");
    Board->ledsOn();
  }
  MState * MStart::fsm()
  {
    switch (Keyboard->getKey())    //Здесь так можно
    {
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                          return new MStop(Tools);
      case MKeyboard::C_CLICK: Board->buzzerOn();                               return new MGo(Tools);
      case MKeyboard::P_CLICK: Board->buzzerOn();                               return new MSetpointV(Tools);
      case MKeyboard::B_CLICK: Board->buzzerOn();               if(--cnt <= 0)  return new MClearPidKeys(Tools);
      case MKeyboard::UP_CLICK: Board->buzzerOn();
        mode = Tools->updnInt(mode, below, above, +1);
        Tools->writeNvsShort("pidtest", "mode", mode);                          break;       
      case MKeyboard::DN_CLICK: Board->buzzerOn();
        mode = Tools->updnInt(mode, below, above, -1);
        Tools->writeNvsShort("pidtest", "mode", mode);                          break;   
      default:;
    }
    switch (mode)
    {
      case -1: Display->showMode((char*)"    PID GO   +/   "); break;
      case 0:  Display->showMode((char*)"    PID OFF  +/-  "); break;
      case 1:  Display->showMode((char*)"    PID  U   +/-  "); break;
      case 2:  Display->showMode((char*)"    PID  I   +/-  "); break;
      case 3:  Display->showMode((char*)"    PID  D    /-  "); break;
      default:;
    }
    Display->showVolt(Tools->getRealVoltage(), 2);
    Display->showAmp (Tools->getRealCurrent(), 2);
    return this;
  };

  //========================================================================= MClearPidKeys
    // Состояние "Очистка всех ключей режима PIDTEST".
  MClearPidKeys::MClearPidKeys(MTools * Tools) : MState(Tools)
  {
    Display->showMode((char*)"      CLEAR?      ");   // В каком режиме
    Display->showHelp((char*)"  P-NO     C-YES  ");   // Активные кнопки
    Board->ledsBlue();
    cnt = 50;                                         // 5с на принятие решения, иначе отказ
  }
  MState * MClearPidKeys::fsm()
  {
    switch  (Keyboard->getKey())
    {
    case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                          return new MStop(Tools);
    case MKeyboard::P_CLICK: Board->buzzerOn();                               return new MSetpointV(Tools);
    case MKeyboard::C_CLICK: Board->buzzerOn();
      done = Tools->clearAllKeys("pidtest");
      vTaskDelay(2 / portTICK_PERIOD_MS);
            #ifdef TEST_KEYS_CLEAR
              Serial.print("\nAll keys \"pidtest\": ");
              (done) ? Serial.println("cleared") : Serial.println("err");
            #endif
      vTaskDelay(100 / portTICK_PERIOD_MS);
      done = Tools->clearAllKeys("profil1");
      vTaskDelay(2 / portTICK_PERIOD_MS);
            #ifdef TEST_KEYS_CLEAR
              Serial.print("\nAll keys \"profil1\": ");
              (done) ? Serial.println("cleared") : Serial.println("err");
            #endif
      vTaskDelay(100 / portTICK_PERIOD_MS);
      done = Tools->clearAllKeys("profil2");
      vTaskDelay(2 / portTICK_PERIOD_MS);
            #ifdef TEST_KEYS_CLEAR
              Serial.print("\nAll keys \"profil2\": ");
              (done) ? Serial.println("cleared") : Serial.println("err");
            #endif
      vTaskDelay(100 / portTICK_PERIOD_MS);
      done = Tools->clearAllKeys("profil3");
      vTaskDelay(2 / portTICK_PERIOD_MS);
            #ifdef TEST_KEYS_CLEAR
              Serial.print("\nAll keys \"profil3\": ");
              (done) ? Serial.println("cleared") : Serial.println("err");
            #endif
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;
    default:;
    }
    if(--cnt <= 0)                                                            return new MStart(Tools);
    Display->showMode((char*)"     CLEARING     ");
    Display->showHelp((char*)"  ___C-CLEAR___   ");
    return this;
  };

  //========================================================================= MSetpointV
    // Состояние: "Ввод порога ПИД-регулятора напряжения".
    /* Устанавливаемый уровень напряжения индицируется в первой строке дисплея, во второй - 
      установленный уровень тока, для справки. Отличие от текущих значений в количестве 
      знаков после запятой. */
  MSetpointV::MSetpointV(MTools * Tools) : MState(Tools)
  {
    spV  = Tools->readNvsFloat("pidtest", "spV", MConst::fixedV);
    spI  = Tools->readNvsFloat("pidtest", "spI", MConst::fixedI);

      // Индикация 
    Display->showMode((char*)"  SP_V       +/-  ");
    Display->showHelp((char*)"  B-SAVE    C-GO  ");
    Board->ledsGreen();
  }
  MState * MSetpointV::fsm()
  {
    switch (Keyboard->getKey())
    {
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                        return new MStop(Tools);
      case MKeyboard::C_CLICK: Board->buzzerOn();                             return new MGo(Tools);
      case MKeyboard::B_CLICK: Board->buzzerOn();
        Tools->writeNvsFloat("pidtest", "spV", spV);                          return new MSetpointI(Tools);
      case MKeyboard::P_CLICK: Board->buzzerOn();                             return new MSetpointI(Tools);
      case MKeyboard::UP_CLICK: Board->buzzerOn();
        spV = Tools->updnFloat(spV, below, above, 0.1f);                      break;     
      case MKeyboard::UP_LONG_CLICK: Board->buzzerOn();
        spV = Tools->updnFloat(spV, below, above, 1.0f);                      break;
      case MKeyboard::DN_CLICK: Board->buzzerOn();
        spV = Tools->updnFloat(spV, below, above, -0.1f);                     break;
      case MKeyboard::DN_LONG_CLICK: Board->buzzerOn();
        spV = Tools->updnFloat(spV, below, above, -1.0f);                     break;
      default:;
    }
    Display->showVolt(spV, 1);
    Display->showAmp(spI, 1);
    return this;
  };

  //========================================================================= MSetpointI
    // Состояние: "Ввод порога ПИД-регулятора тока".
    /* Кнопкой (P) можно вернуться к вводу напряжения */
  MSetpointI::MSetpointI(MTools * Tools) : MState(Tools)
  {
    spV = Tools->readNvsFloat("pidtest", "spV", MConst::fixedV);
    spI = Tools->readNvsFloat("pidtest", "spI", MConst::fixedI);
    Display->showMode((char*)"  SP_I       +/-  ");
    Display->showHelp((char*)"  B-SAVE    C-GO  ");
    Board->ledsGreen();
  }
  MState * MSetpointI::fsm()
  {
    switch (Keyboard->getKey())
    {
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                        return new MStop(Tools);
      case MKeyboard::C_CLICK: Board->buzzerOn();                             return new MGo(Tools);
      case MKeyboard::B_CLICK: Board->buzzerOn();
        Tools->writeNvsFloat("pidtest", "spI", spI);                          return new MGo(Tools);
      case MKeyboard::P_CLICK: Board->buzzerOn();                             return new MSetpointV(Tools);
      case MKeyboard::UP_CLICK: Board->buzzerOn();
        spI = Tools->updnFloat(spI, below, above, 0.1f);                      break;     
      case MKeyboard::UP_LONG_CLICK: Board->buzzerOn();
        spI = Tools->updnFloat(spI, below, above, 1.0f);                      break;
      case MKeyboard::DN_CLICK: Board->buzzerOn();
        spI = Tools->updnFloat(spI, below, above, -0.1f);                     break;
      case MKeyboard::DN_LONG_CLICK: Board->buzzerOn();
        spI = Tools->updnFloat(spI, below, above, -1.0f);                     break;
        
      default:;
    }
    Display->showVolt(spV, 1);
    Display->showAmp(spI, 1);
    return this;
  };

  //========================================================================= MGo
    // Состояние: "Подбор параметров ПИД-регулятора".
  MGo::MGo(MTools * Tools) : MState(Tools)
  {
    spV  = Tools->readNvsFloat("pidtest", "spV",  MConst::fixedV);
    spI  = Tools->readNvsFloat("pidtest", "spI",  MConst::fixedI);
    mode = Tools->readNvsShort("pidtest", "mode", MConst::fixedM);


    kpV = Tools->readNvsFloat("pidtest", "kpV", MConst::fixedKpV);
    kiV = Tools->readNvsFloat("pidtest", "kiV", MConst::fixedKiV);
    kdV = Tools->readNvsFloat("pidtest", "kdV", MConst::fixedKdV);
    kpI = Tools->readNvsFloat("pidtest", "kpI", MConst::fixedKpI);
    kiI = Tools->readNvsFloat("pidtest", "kiI", MConst::fixedKiI);
    kdI = Tools->readNvsFloat("pidtest", "kdI", MConst::fixedKdI);

          #ifdef PRINT_PID
            vTaskDelay(2 / portTICK_PERIOD_MS);
            Serial.print("\nmode="); Serial.print(mode);
            Serial.print("\nspV="); Serial.print(spV, 2);
            Serial.print("\nspI="); Serial.print(spI, 2);
            Serial.print("\nkpV="); Serial.print(kpV, 2);
            Serial.print("\nkiV="); Serial.print(kiV, 2);
            Serial.print("\nkdV="); Serial.print(kdV, 2);
            Serial.print("\nkpI="); Serial.print(kpI, 2);
            Serial.print("\nkiI="); Serial.print(kiI, 2);
            Serial.print("\nkdI="); Serial.print(kdI, 2);
          #endif
    Display->showMode((char*)" P-SP B-SAVE C-IO ");
    Display->showHelp((char*)" P*KP  B*KI  C*KD ");
    Board->ledsGreen();
    Tools->txPowerMode(spV, spI, mode);                                            // 0x22  Команда драйверу
    Display->showVolt(Tools->getRealVoltage(), 2);
    Display->showAmp (Tools->getRealCurrent(), 2);
  }
  MState * MGo::fsm()
  {
    switch (Keyboard->getKey())
    {
      case MKeyboard::P_CLICK: Board->buzzerOn();
        Tools->writeNvsFloat("pidtest", "kpV", kpV);
        Tools->writeNvsFloat("pidtest", "kiV", kiV);
        Tools->writeNvsFloat("pidtest", "kdV", kdV);
        Tools->writeNvsFloat("pidtest", "kpI", kpI);
        Tools->writeNvsFloat("pidtest", "kiI", kiI);
        Tools->writeNvsFloat("pidtest", "kdI", kdI);                          return new MSetpointV(Tools);
      case MKeyboard::B_CLICK: Board->buzzerOn();
        Tools->writeNvsFloat("pidtest", "kpV", kpV);
        Tools->writeNvsFloat("pidtest", "kiV", kiV);
        Tools->writeNvsFloat("pidtest", "kdV", kdV);
        Tools->writeNvsFloat("pidtest", "kpI", kpI);
        Tools->writeNvsFloat("pidtest", "kiI", kiI);
        Tools->writeNvsFloat("pidtest", "kdI", kdI);                          return new MExportPID(Tools);
      case MKeyboard::C_CLICK: Board->buzzerOn();
        (io ^= true) ? Tools->txPowerMode(spV, spI, mode) : Tools->txPowerStop();  break;     // 0x22
      case MKeyboard::P_LONG_CLICK: Board->buzzerOn(); k = 1;
              #ifdef PRINT_PID
                Serial.print("\nk="); Serial.println(k);
                Serial.print("\nstate=0x"); Serial.println(state, HEX);
              #endif
        Display->showHelp((char*)" -<KP>+             ");                     break;
      case MKeyboard::B_LONG_CLICK: Board->buzzerOn(); k = 2;
              #ifdef PRINT_PID
                Serial.print("\nk="); Serial.println(k);
                Serial.print("\nstate=0x"); Serial.println(state, HEX);
              #endif
        Display->showHelp((char*)"       -<KI>+       ");                     break;
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn(); k = 3;
              #ifdef PRINT_PID
                Serial.print("\nk="); Serial.println(k);
                Serial.print("\nstate=0x"); Serial.println(state, HEX);
              #endif
        Display->showHelp((char*)"             -<KD>+ ");                     break;
      case MKeyboard::UP_CLICK: Board->buzzerOn();
        //if(state == Tools->status_pid_voltage )
        if(mode == 1)
        {
          switch (k)
          {
            case 1: kpV = Tools->updnFloat(kpV, below, above, 0.1f);         break;
            case 2: kiV = Tools->updnFloat(kiV, below, above, 0.1f);         break;
            case 3: kdV = Tools->updnFloat(kdV, below, above, 0.1f);         break;
            default:;
          }
          Tools->txSetPidCoeffV(kpV, kiV, kdV);
                #ifdef PRINT_PID
                  Serial.print("\nkpV="); Serial.print(kpV, 2);
                  Serial.print("\nkiV="); Serial.print(kiV, 2);
                  Serial.print("\nkdV="); Serial.print(kdV, 2);
                #endif
        }
        //if(state == Tools->status_pid_current)
        if(mode == 2)
        {          
          switch (k)
          {
            case 1: kpI = Tools->updnFloat(kpI, below, above, 0.1f);         break;
            case 2: kiI = Tools->updnFloat(kiI, below, above, 0.1f);         break;
            case 3: kdI = Tools->updnFloat(kdI, below, above, 0.1f);         break;
            default:;
          } 
          Tools->txSetPidCoeffI(kpI, kiI, kdI);
                #ifdef PRINT_PID
                  Serial.print("\nkpI="); Serial.print(kpI, 2);
                  Serial.print("\nkiI="); Serial.print(kiI, 2);
                  Serial.print("\nkdI="); Serial.print(kdI, 2);
                #endif
        }
      break;
      case MKeyboard::DN_CLICK: Board->buzzerOn();
        //if(state == Tools->status_pid_voltage)
        if(mode == 1)
        {
          switch (k)
          {
            case 1: kpV = Tools->updnFloat(kpV, below, above, -0.1f);        break;
            case 2: kiV = Tools->updnFloat(kiV, below, above, -0.1f);        break;
            case 3: kdV = Tools->updnFloat(kdV, below, above, -0.1f);        break;
            default:;
          }
          Tools->txSetPidCoeffV(kpV, kiV, kdV);
                #ifdef PRINT_PID
                  Serial.print("\nkpV="); Serial.print(kpV, 2);
                  Serial.print("\nkiV="); Serial.print(kiV, 2);
                  Serial.print("\nkdV="); Serial.print(kdV, 2);
                #endif
        }
        //if(state == Tools->status_pid_current)
        if(mode == 2)
        {          
          switch (k)
          {
            case 1: kpI = Tools->updnFloat(kpI, below, above, -0.1f);        break;
            case 2: kiI = Tools->updnFloat(kiI, below, above, -0.1f);        break;
            case 3: kdI = Tools->updnFloat(kdI, below, above, -0.1f);        break;
            default:;
          }
          Tools->txSetPidCoeffI(kpI, kiI, kdI);
                #ifdef PRINT_PID
                  Serial.print("\nkpI="); Serial.print(kpI, 2);
                  Serial.print("\nkiI="); Serial.print(kiI, 2);
                  Serial.print("\nkdI="); Serial.print(kdI, 2);
                #endif                                                                         
        }
      break;
      default:;
    }
    state = Tools->getState();
    if(state == Tools->status_pid_voltage)      Display->initBar(TFT_YELLOW);
    else if(state == Tools->status_pid_current) Display->initBar(TFT_GREEN);
    else                                        Display->barOff();
    Display->showVolt(Tools->getRealVoltage(), 2);
    Display->showAmp (Tools->getRealCurrent(), 2);
    return this;
  };

  //========================================================================= MExportPID
    // Состояние: "Экспорт параметров ПИД-регулятора".
  MExportPID::MExportPID(MTools * Tools) : MState(Tools)
  {
    Display->showMode((char*)"  SAVE PROFIL AS  ");
    Display->showHelp((char*)" P*P1  B*P2  C*P3 ");
  }
  MState * MExportPID::fsm()
  {
    switch (Keyboard->getKey())
    {
      case MKeyboard::C_CLICK: Board->buzzerOn();                             return new MStop(Tools);
      case MKeyboard::P_CLICK: Board->buzzerOn();                             return new MGo(Tools);
      case MKeyboard::P_LONG_CLICK: Board->buzzerOn(); k = 1;
              #ifdef PRINT_PID
                Serial.print("\n\nВыбран профиль №1");
              #endif
        Display->showHelp((char*)"  <P1>              ");
        break;
      case MKeyboard::B_LONG_CLICK: Board->buzzerOn(); k = 2;
              #ifdef PRINT_PID
                Serial.print("\n\nВыбран профиль №2");
              #endif
        Display->showHelp((char*)"        <P2>        ");
        break;
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn(); k = 3;
              #ifdef PRINT_PID
                Serial.print("\n\nВыбран профиль №3");
              #endif
        Display->showHelp((char*)"              <P3>  ");
        break;

      case MKeyboard::B_CLICK: Board->buzzerOn();
        switch (k)
        {
        case 1:
          kpV = Tools->copyNvsFloat("pidtest", "profil1", "kpV", MConst::fixedKpV);
          kiV = Tools->copyNvsFloat("pidtest", "profil1", "kiV", MConst::fixedKiV);
          kdV = Tools->copyNvsFloat("pidtest", "profil1", "kdV", MConst::fixedKdV);
          kpI = Tools->copyNvsFloat("pidtest", "profil1", "kpI", MConst::fixedKpI);
          kiI = Tools->copyNvsFloat("pidtest", "profil1", "kiI", MConst::fixedKiI);
          kdI = Tools->copyNvsFloat("pidtest", "profil1", "kdI", MConst::fixedKdI);
                  #ifdef PRINT_PID
                    vTaskDelay(2 / portTICK_PERIOD_MS);
                    Serial.print("\nЭкспорт: k\nВыбран профиль №1");
                    Serial.print("\nkpV="); Serial.print(kpV, 1);
                    Serial.print("\nkiV="); Serial.print(kiV, 1);
                    Serial.print("\nkdV="); Serial.print(kdV, 1);
                    Serial.print("\nkpI="); Serial.print(kpI, 1);
                    Serial.print("\nkiI="); Serial.print(kiI, 1);
                    Serial.print("\nkdI="); Serial.print(kdI, 1);
                  #endif
          break;
        case 2:
          kpV = Tools->copyNvsFloat("pidtest", "profil2", "kpV", MConst::fixedKpV);
          kiV = Tools->copyNvsFloat("pidtest", "profil2", "kiV", MConst::fixedKiV);
          kdV = Tools->copyNvsFloat("pidtest", "profil2", "kdV", MConst::fixedKdV);
          kpI = Tools->copyNvsFloat("pidtest", "profil2", "kpI", MConst::fixedKpI);
          kiI = Tools->copyNvsFloat("pidtest", "profil2", "kiI", MConst::fixedKiI);
          kdI = Tools->copyNvsFloat("pidtest", "profil2", "kdI", MConst::fixedKdI);
                    #ifdef PRINT_PID
                    vTaskDelay(2 / portTICK_PERIOD_MS);
                      Serial.print("\nЭкспорт: k\nВыбран профиль №2");
                      Serial.print("\nkpV="); Serial.print(kpV, 1);
                      Serial.print("\nkiV="); Serial.print(kiV, 1);
                      Serial.print("\nkdV="); Serial.print(kdV, 1);
                      Serial.print("\nkpI="); Serial.print(kpI, 1);
                      Serial.print("\nkiI="); Serial.print(kiI, 1);
                      Serial.print("\nkdI="); Serial.print(kdI, 1);
                    #endif
          break;
        case 3:
          kpV = Tools->copyNvsFloat("pidtest", "profil3", "kpV", MConst::fixedKpV);
          kiV = Tools->copyNvsFloat("pidtest", "profil3", "kiV", MConst::fixedKiV);
          kdV = Tools->copyNvsFloat("pidtest", "profil3", "kdV", MConst::fixedKdV);
          kpI = Tools->copyNvsFloat("pidtest", "profil3", "kpI", MConst::fixedKpI);
          kiI = Tools->copyNvsFloat("pidtest", "profil3", "kiI", MConst::fixedKiI);
          kdI = Tools->copyNvsFloat("pidtest", "profil3", "kdI", MConst::fixedKdI);
                  #ifdef PRINT_PID
                    vTaskDelay(2 / portTICK_PERIOD_MS);
                    Serial.print("\nЭкспорт: k\nВыбран профиль №3");
                    Serial.print("\nkpV="); Serial.print(kpV, 1);
                    Serial.print("\nkiV="); Serial.print(kiV, 1);
                    Serial.print("\nkdV="); Serial.print(kdV, 1);
                    Serial.print("\nkpI="); Serial.print(kpI, 1);
                    Serial.print("\nkiI="); Serial.print(kiI, 1);
                    Serial.print("\nkdI="); Serial.print(kdI, 1);
                  #endif
          break;        
        default:;
        }

        break;
      default:;
    }
    return this;    //       new MGo(Tools);                                                    // Вернуться к запуску заряда
  };

//=========================================================================== MStop
  // Состояние: "авершение режима PIDTEST".
  MStop::MStop(MTools * Tools) : MState(Tools)
  {
    Tools->txPowerStop();                               // 0x21  Команда драйверу перейти в безопасный режим
    Display->showMode((char*)"       STOP        ");
    Display->showHelp((char*)"      C-EXIT       ");
    Board->ledsRed();
    Display->barOff();
  }    
  MState * MStop::fsm()
  {
    switch (Keyboard->getKey())
    {
    case MKeyboard::C_CLICK:  Board->buzzerOn();                              return new MExit(Tools);
    default:;
    }
    Display->showVolt(Tools->getRealVoltage(), 2);
    Display->showAmp (Tools->getRealCurrent(), 2);
    return this;
  };  //MStop

  //========================================================================= MExit

    // Процесс выхода из режима - до нажатия кнопки "С" удерживается индикация о завершении.
    /* Состояние: "Индикация итогов и выход из режима в меню диспетчера". */ 
  MExit::MExit(MTools * Tools) : MState(Tools)
  {
    Display->showMode((char*)"    PIDTEST OFF   ");
    Display->showHelp((char*)"  P-AGAIN  C-EXIT ");  // To select the mode
    Board->ledsOff();
  }    
  MState * MExit::fsm()
  {
    switch (Keyboard->getKey())
    {
    case MKeyboard::P_CLICK:  Board->buzzerOn();                              return new MStart(Tools);
    case MKeyboard::C_CLICK:  Board->buzzerOn(); 
      Display->showMode((char*)"      PIDTEST:     ");
      Display->showHelp((char*)"  ADJ  KP, PD, KI  ");                        return nullptr;
    default:;
    }
    return this;
  };

};
