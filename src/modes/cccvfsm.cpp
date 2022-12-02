/*
  Файл: cccvfsm.cpp
                          Как это работает.
    Режим простого заряда, известный как CC/CV, реализован в виде конечного автомата, 
  инициализация которого производится посредством выбора "CCCV" в меню диспетчера.
  График такого алгоритма заряда представлен на рисунке  http://www.balsat.ru/statia2.php.
    Начальное состояние MStart, как и все последующие, предоставляет оператору в виде 
  подсказок назначения сенсорных кнопок при входе в каждое состояние. Оператору доступны 
  короткое, длинное и автонажатие. Подсказки вида "C-EXIT" и "C*EXIT" означают, что короткое 
  или длинное нажатие "C" соответственно вызывает переход к указанному состоянию. При вводе 
  параметров длинное нажатие используется для десятикратного увеличения шага приращения 
  параметра кнопками "+" и "-".
    По понятным причинам не все возможности отображены в подсказках. Например, по кнопке 
  "B" при вводе параметров обычно производится сохранение параметра с переходом к 
  следующему параметру, а "P" - возврат к редактированию предыдущего. Длинное "C"  
  за редким исключением инициирует завершение активной части CCCV, в этом случае "p" 
  вернёт к редактированию первого параметра, а не предыдущего.
    Откуда берутся параметры? Некоторые, редко корректируемые, такие как номинальные 
  параметры заряжаемой батареи, задержка пуска и длительность выдержки в третьей фазе 
  заряда автоматически вводятся по ходу исполнения алгоритма конечным автоматом (они 
  заданы в пользовательских настройках "OPTIONS"). Впрочем, иногда оператору предоставляется 
  возможность ввести коррекцию, выбрав собственную комбинацию уровней напряжений и токов - 
  всё на усмотрение разработчика.
    Иначе в случае необходимости корректируются коэффициенты ПИД-регуляторов 
  (пропорциональный, интегральный, дифференциальный). Известно, что оптимальный подбор 
  коэффициентов влияет как на устойчивость системы вцелом, так и предоставляет расширить 
  арсенал разработчика в части реализации алгоритмов заряда, разряда, сервиса и т.д.
  Подбор - дело муторное. С этой целью в режиме "PIDTEST" реализован конечный автомат,
  облегчающий сей процесс, здесь же коэффициенты задаются программно, если они известны 
  разработчику, или импортируются из "PIDTEST", где имеется возможность сохранить аж 
  целых три профиля, из которых остается только выбрать, причем можно это делать "на лету".
    В активной части алгоритма для регулировок "на лету" назначения кнопок могут быть 
  совсем иными - будьте внимательны.
 
  Версия от 15.08.2022
*/

#include "modes/cccvfsm.h"
#include "mtools.h"
#include "mcmd.h"
#include "board/mboard.h"
#include "board/msupervisor.h"
//#include "board/mkeyboard.h"
#include "measure/mkeyboard.h"

#include "display/mdisplay.h"
#include <Arduino.h>
#include <string>

namespace MCccv
{
  float maxV, minV, maxI, minI;
  
  //========================================================================= MStart
    // Состояние "Старт", инициализация выбранного режима работы (CC/CV).
    /*  Параметры заряда восстанавливаются из энергонезависимой памяти и соответствуют 
      предыдущему включению. Пороговые значения напряжений и токов рассчитываются исходя
      из типа батареи, её номинального напряжения и емкости. Выбор предполагается производить в 
      "OPTION", что в данном проекте не реализовано. Здесь же реализован частный случай для 
      батареи 12в 50Ач, но имеется возможность коррекции пороговых значений, которые также 
      сохраняются и восстанавливаются из энергонезависимой памяти.
        В случае отсутствия в памяти таковых данных они заменяются рассчитанными.
    */
  MStart::MStart(MTools * Tools) : MState(Tools)
  {
    //Отключить на всякий пожарный
    Tools->txPowerStop();                                                   // 0x21  Команда драйверу
    voltageNom = Tools->readNvsFloat("options", "nominalV", nominal_v_fixed);
    capacity   = Tools->readNvsFloat("options", "capacity", capacity_fixed);

    maxV = Tools->readNvsFloat("cccv", "maxV", voltageNom * voltageMaxFactor);
    minV = Tools->readNvsFloat("cccv", "minV", voltageNom * voltageMinFactor);
    maxI = Tools->readNvsFloat("cccv", "maxI", capacity * currentMaxFactor);
    minI = Tools->readNvsFloat("cccv", "minI", capacity * currentMinFactor);
          #ifdef TESTCCCV
            vTaskDelay(2 / portTICK_PERIOD_MS);
            Serial.print("\nПользовательские, а если нет, то разработчика:");
            Serial.print("\nvoltageNom="); Serial.print(voltageNom, 1);
            Serial.print("\ncapacity="); Serial.print(capacity, 1);
            Serial.print("\nmaxV="); Serial.print(maxV, 3);
            Serial.print("\nminV="); Serial.print(minV, 3);
            Serial.print("\nmaxI="); Serial.print(maxI, 3);
            Serial.print("\nminI="); Serial.print(minI, 3);
          #endif
    cnt = 7;
    // Индикация
    Display->showMode((char*)"       CC/CV      ");  // В каком режиме
    Display->showHelp((char*)"    P-ADJ   C-GO  ");  // Активные кнопки
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
        maxV = voltageNom * voltageMaxFactor;
        minV = voltageNom * voltageMinFactor;
        maxI = capacity * currentMaxFactor;
        minI = capacity * currentMinFactor;
              #ifdef TESTCCCV
                Serial.print("\n\nСтарт по параметрам батареи:");
                Serial.print("\nvoltageNom="); Serial.print(voltageNom, 1);
                Serial.print("\ncapacity="); Serial.print(capacity, 1);                
                Serial.print("\nmaxV="); Serial.print(maxV, 3);
                Serial.print("\nminV="); Serial.print(minV, 3);
                Serial.print("\nmaxI="); Serial.print(maxI, 3);
                Serial.print("\nminI="); Serial.print(minI, 3);
                Serial.println();
              #endif
                                                                              return new MPostpone(Tools);
      case MKeyboard::P_CLICK: Board->buzzerOn();                             return new MSetCurrentMax(Tools);
      case MKeyboard::B_CLICK: Board->buzzerOn();
        if(--cnt <= 0)                                                        return new MClearCccvKeys(Tools);
        break;
      default:;
    }
    // Индикация текущих значений, указывается число знаков после запятой
    Display->showVolt(Tools->getRealVoltage(), 3);
    Display->showAmp (Tools->getRealCurrent(), 3);
    return this;
  };  // MStart

  //========================================================================= MClearCccvKeys
    // Состояние "Очистка всех ключей раздела".
  MClearCccvKeys::MClearCccvKeys(MTools * Tools) : MState(Tools)
  {
    Display->showMode((char*)"      CLEAR?      ");
    Display->showHelp((char*)"  P-NO     C-YES  ");
    Board->ledsBlue();
    cnt = 50;                                                                 // 5с 
  }
  MState * MClearCccvKeys::fsm()
  {
    switch  (Keyboard->getKey())
    {
    case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                          return new MStop(Tools);
    case MKeyboard::P_CLICK: Board->buzzerOn();                               return new MSetCurrentMax(Tools);
    case MKeyboard::C_CLICK: Board->buzzerOn();
      done = Tools->clearAllKeys("cccv");
      vTaskDelay(2 / portTICK_PERIOD_MS);
            #ifdef TEST_KEYS_CLEAR
              Serial.print("\nAll keys \"cccv\": ");
              (done) ? Serial.println("cleared") : Serial.println("err");
            #endif
      break;
    default:;
    }
    if(--cnt <= 0)                                                            return new MStart(Tools);
    Display->showMode((char*)"     CLEARING     ");
    Display->showHelp((char*)"    ___WAIT___    ");
    return this;
  };  // MClearCccvKeys

   //======================================================================== MSetCurrentMax
    // Состояние "Коррекция максимального тока заряда".
  MSetCurrentMax::MSetCurrentMax(MTools * Tools) : MState(Tools)
  {
    // Индикация
    Display->showMode((char*)" MAX_I        +/- ");  // Регулируемый параметр
    Display->showHelp((char*)" B-SAVE      C-GO ");  // Активные кнопки
    Board->ledsGreen();                                                       // Подтверждение входа
  }
  MState * MSetCurrentMax::fsm()
  {
    switch (Keyboard->getKey())
    {
        // Отказ от продолжения ввода параметров - стоп
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                        return new MStop(Tools);
        // Отказ от дальнейшего ввода параметров - исполнение
      case MKeyboard::C_CLICK: Board->buzzerOn();                             return new MPostpone(Tools);
      case MKeyboard::P_CLICK: Board->buzzerOn();                             return new MStart(Tools);
        // Сохранить и перейти к следующему параметру
      case MKeyboard::B_CLICK: Board->buzzerOn();       
        Tools->writeNvsFloat("cccv", "maxI", maxI);                           return new MSetVoltageMax(Tools);
        // Увеличить значение на указанную величину
      case MKeyboard::UP_CLICK:  Board->buzzerOn(); 
      case MKeyboard::UP_AUTO_CLICK:  
        maxI = Tools->updnFloat(maxI, below, above, 0.1f);
        break;
        // Уменьшить значение на указанную величину
      case MKeyboard::DN_CLICK: Board->buzzerOn();
      case MKeyboard::DN_AUTO_CLICK:  
        maxI = Tools->updnFloat(maxI, below, above, -0.1f);
        break;
      default:;
    }
    // Если не закончили ввод, то индикация введенного и остаться в том же состоянии
    Display->showVolt(Tools->getRealVoltage(), 3);
    Display->showAmp(maxI, 1);
    return this;
  };  // MSetCurrentMax

  //========================================================================= MSetVoltageMax
    // Состояние: "Коррекция максимального напряжения"
  MSetVoltageMax::MSetVoltageMax(MTools * Tools) : MState(Tools)
  {
    // Индикация 
    Display->showMode((char*)" MAX_V        +/- ");
    Display->showHelp((char*)" B-SAVE      C-GO ");
  }
  MState * MSetVoltageMax::fsm()
  {
    switch ( Keyboard->getKey() )
    {
        // Отказ от продолжения ввода параметров - стоп
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                        return new MStop(Tools);
        // Отказ от дальнейшего ввода параметров - исполнение
      case MKeyboard::C_CLICK: Board->buzzerOn();                             return new MPostpone(Tools);
      case MKeyboard::P_CLICK: Board->buzzerOn();                             return new MSetCurrentMax(Tools);
        // Сохранить и перейти к следующему параметру
      case MKeyboard::B_CLICK: Board->buzzerOn();
        Tools->writeNvsFloat("cccv", "maxV", maxV);                           return new MSetCurrentMin(Tools);
        // Увеличить значение на указанную величину
      case MKeyboard::UP_CLICK: Board->buzzerOn();
      case MKeyboard::UP_AUTO_CLICK:  
        maxV = Tools->updnFloat(maxV, below, above, 0.1f);
        break;
        // Уменьшить значение на указанную величину
      case MKeyboard::DN_CLICK: Board->buzzerOn(); 
      case MKeyboard::DN_AUTO_CLICK: 
        maxV = Tools->updnFloat(maxV, below, above, -0.1f);
        break;
      default:;
    }
      // Показать и продолжить
    Display->showVolt(maxV, 1);
    Display->showAmp(Tools->getRealCurrent(), 3);
    return this;
  };  // MSetVoltageMax

  //========================================================================= MSetCurrentMin
  // Состояние: "Коррекция минимального тока заряда"
  MSetCurrentMin::MSetCurrentMin(MTools * Tools) : MState(Tools)
  {
    // Индикация помощи
    Display->showMode((char*)" MIN_I        +/- ");
    Display->showHelp((char*)" B-SAVE      C-GO ");
  }   
  MState * MSetCurrentMin::fsm()
  {
    switch (Keyboard->getKey())
    {
        // Отказ от продолжения ввода параметров - стоп
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                        return new MStop(Tools);
        // Отказ от дальнейшего ввода параметров - исполнение
      case MKeyboard::C_CLICK: Board->buzzerOn();                             return new MPostpone(Tools);
      case MKeyboard::P_CLICK: Board->buzzerOn();                             return new MSetVoltageMax(Tools);
        // Сохранить и перейти к следующему параметру
      case MKeyboard::B_CLICK: Board->buzzerOn();
        Tools->writeNvsFloat("cccv", "minI", minI);                           return new MSetVoltageMin(Tools);
      case MKeyboard::UP_CLICK: Board->buzzerOn();
      case MKeyboard::UP_AUTO_CLICK:
        minI = Tools->updnFloat(minI, below, above, 0.1f);
        break;
      case MKeyboard::DN_CLICK: Board->buzzerOn();
      case MKeyboard::DN_AUTO_CLICK:
        minI = Tools->updnFloat(minI, below, above, -0.1f);
        break;
      default:;
    }
    // Показать и продолжить
    Display->showVolt(Tools->getRealVoltage(), 3);
    Display->showAmp(minI, 1);
    return this;
  };  // MSetCurrentMin

  //========================================================================= MSetVoltageMin
  // Состояние: "Коррекция минимального напряжения окончания заряда"
  MSetVoltageMin::MSetVoltageMin(MTools * Tools) : MState(Tools)
  {
    // Индикация помощи
    Display->showMode((char*)" MIN_V        +/- ");
    Display->showHelp((char*)" B-SAVE      C-GO ");
  }   
  MState * MSetVoltageMin::fsm()
  {
    switch (Keyboard->getKey())
    {
        // Отказ от продолжения ввода параметров - стоп    
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                        return new MStop(Tools);
        // Отказ от дальнейшего ввода параметров - исполнение
      case MKeyboard::C_CLICK: Board->buzzerOn();                             return new MPostpone(Tools);
      case MKeyboard::P_CLICK: Board->buzzerOn();                             return new MSetCurrentMin(Tools);
        // Сохранить и перейти к исполнению
      case MKeyboard::B_CLICK: Board->buzzerOn();
        Tools->writeNvsFloat( "cccv", "minV", minV );                         return new MPostpone(Tools);
      case MKeyboard::UP_CLICK: Board->buzzerOn();
      case MKeyboard::UP_AUTO_CLICK:
        minV = Tools->updnFloat(minV, below, above, 0.1f);
        break;
      case MKeyboard::DN_CLICK: Board->buzzerOn();
      case MKeyboard::DN_AUTO_CLICK:
        minV = Tools->updnFloat(minV, below, above, -0.1f);
        break;
      default:;
    }
    // Индикация ввода
    Display->showVolt(minV, 1);
    Display->showAmp(Tools->getRealCurrent(), 3);
    return this;
  };  // MSetVoltageMin

  //========================================================================= MPostpone
  // Состояние: "Задержка включения (отложенный старт)", время ожидания старта задается в OPTIONS.
  MPostpone::MPostpone(MTools * Tools) : MState(Tools)
  {
      // Параметр задержки начала заряда из энергонезависимой памяти, при первом включении - заводское
    Tools->postpone = Tools->readNvsShort("options", "postpone", 0);
          #ifdef PRINT_PID
            Serial.print("\nПараметр задержки установлен на ");
            Serial.print(Tools->postpone);  Serial.print(" час.\n\n");
          #endif
      // Восстановление пользовательских kp, ki
    kp = Tools->readNvsFloat("cccv", "kpV", MConst::fixedKpV);
    ki = Tools->readNvsFloat("cccv", "kiV", MConst::fixedKiV);
    kd = Tools->readNvsFloat("cccv", "kdV", MConst::fixedKdV);
    Tools->txSetPidCoeffV(kp, ki, kd);  vTaskDelay(80 / portTICK_PERIOD_MS);  // 0x41  Команда драйверу
          #ifdef PRINT_PID
            vTaskDelay(2 / portTICK_PERIOD_MS);

            Serial.print("\nОтправлена команда 0x41 (V)");
            Serial.print("\nkpV="); Serial.print(kp, 2);
            Serial.print("\nkiV="); Serial.print(ki, 2);
            Serial.print("\nkdV="); Serial.println(kd, 2);
          #endif
    kp = Tools->readNvsFloat("cccv", "kpI", MConst::fixedKpI);
    ki = Tools->readNvsFloat("cccv", "kiI", MConst::fixedKiI);
    kd = Tools->readNvsFloat("cccv", "kdI", MConst::fixedKdI);
    Tools->txSetPidCoeffI(kp, ki, kd);  vTaskDelay(80 / portTICK_PERIOD_MS);  // 0x41  Команда драйверу
          #ifdef PRINT_PID
            vTaskDelay(2 / portTICK_PERIOD_MS);

            Serial.print("\nОтправлена команда 0x41 (I)");
            Serial.print("\nkpI="); Serial.print(kp, 2);
            Serial.print("\nkiI="); Serial.print(ki, 2);
            Serial.print("\nkdI="); Serial.println(kd, 2);
          #endif
      // Индикация помощи
    Display->showMode((char*)"  DELAYED START   ");
    Display->showHelp((char*)"     C-START      ");
      // Инициализация счетчика времени до старта
    Tools->setTimeCounter( Tools->postpone * 36000 );                // Отложенный старт ( * 0.1s в этой версии)
  }     
  MState * MPostpone::fsm()
  {
      // Старт по времени
    if(Tools->postponeCalculation())                                          return new MUpCurrent(Tools);

    switch (Keyboard->getKey())
    {
        // Досрочное прекращение заряда оператором
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                        return new MStop(Tools);
        // Старт оператором
      case MKeyboard::C_CLICK: Board->buzzerOn();                             return new MUpCurrent(Tools);
      case MKeyboard::B_LONG_CLICK : Board->buzzerOn();                       return new MImportPID(Tools);
      default:;
    }
    // Индикация в период ожидания старта (обратный отсчет)
    Display->showDuration( Tools->getChargeTimeCounter(), MDisplay::SEC );
    //Board->blinkWhite();                // Исполняется  некорректно - пока отменено
    return this;
  };  // MPostpone

  //========================================================================= MImportPID
  // Состояние: "Импорт коэффициентов PID"
  MImportPID::MImportPID(MTools * Tools) : MState(Tools)
  {
    Display->showMode((char*)"    LOADED COPY   ");
    Display->showHelp((char*)" P*P1  B*P2  C*P3 ");
  }
  MState * MImportPID::fsm()
  {

    switch (Keyboard->getKey())
    {
      case MKeyboard::C_CLICK: Board->buzzerOn();                             return new MPostpone(Tools);
      case MKeyboard::P_CLICK: Board->buzzerOn();                             return new MSetVoltageMin(Tools);
      // case MKeyboard::C_CLICK: Board->buzzerOn();
      //   break;     
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
          kpV = Tools->copyNvsFloat("profil1", "cccv", "kpV", MConst::fixedKpV);
          kiV = Tools->copyNvsFloat("profil1", "cccv", "kiV", MConst::fixedKiV);
          kdV = Tools->copyNvsFloat("profil1", "cccv", "kdV", MConst::fixedKdV);
          kpI = Tools->copyNvsFloat("profil1", "cccv", "kpI", MConst::fixedKpI);
          kiI = Tools->copyNvsFloat("profil1", "cccv", "kiI", MConst::fixedKiI);
          kdI = Tools->copyNvsFloat("profil1", "cccv", "kdI", MConst::fixedKdI);
              #ifdef PRINT_PID
                Serial.print("\nИмпорт: k\nВыбран профиль №1");
                Serial.print("\nkpV="); Serial.print(kpV, 2);
                Serial.print("\nkiV="); Serial.print(kiV, 2);
                Serial.print("\nkdV="); Serial.print(kdV, 2);
                Serial.print("\nkpI="); Serial.print(kpI, 2);
                Serial.print("\nkiI="); Serial.print(kiI, 2);
                Serial.print("\nkdI="); Serial.print(kdI, 2);
              #endif
          break;
        case 2:
          kpV = Tools->copyNvsFloat("profil2", "cccv", "kpV", MConst::fixedKpV);
          kiV = Tools->copyNvsFloat("profil2", "cccv", "kiV", MConst::fixedKiV);
          kdV = Tools->copyNvsFloat("profil2", "cccv", "kdV", MConst::fixedKdV);
          kpI = Tools->copyNvsFloat("profil2", "cccv", "kpI", MConst::fixedKpI);
          kiI = Tools->copyNvsFloat("profil2", "cccv", "kiI", MConst::fixedKiI);
          kdI = Tools->copyNvsFloat("profil2", "cccv", "kdI", MConst::fixedKdI);
              #ifdef PRINT_PID
                Serial.print("\nИмпорт: k\nВыбран профиль №2");
                Serial.print("\nkpV="); Serial.print(kpV, 2);
                Serial.print("\nkiV="); Serial.print(kiV, 2);
                Serial.print("\nkdV="); Serial.print(kdV, 2);
                Serial.print("\nkpI="); Serial.print(kpI, 2);
                Serial.print("\nkiI="); Serial.print(kiI, 2);
                Serial.print("\nkdI="); Serial.print(kdI, 2);
              #endif
          break;
        case 3:
          kpV = Tools->copyNvsFloat("profil3", "cccv", "kpV", MConst::fixedKpV);
          kiV = Tools->copyNvsFloat("profil3", "cccv", "kiV", MConst::fixedKiV);
          kdV = Tools->copyNvsFloat("profil3", "cccv", "kdV", MConst::fixedKdV);
          kpI = Tools->copyNvsFloat("profil3", "cccv", "kpI", MConst::fixedKpI);
          kiI = Tools->copyNvsFloat("profil3", "cccv", "kiI", MConst::fixedKiI);
          kdI = Tools->copyNvsFloat("profil3", "cccv", "kdI", MConst::fixedKdI);
              #ifdef PRINT_PID
                Serial.print("\nИмпорт: k\nВыбран профиль №3");
                Serial.print("\nkpV="); Serial.print(kpV, 2);
                Serial.print("\nkiV="); Serial.print(kiV, 2);
                Serial.print("\nkdV="); Serial.print(kdV, 2);
                Serial.print("\nkpI="); Serial.print(kpI, 2);
                Serial.print("\nkiI="); Serial.print(kiI, 2);
                Serial.print("\nkdI="); Serial.print(kdI, 2);
              #endif
          break;        
        default:;
        }

        break;
      default:;
    }
    
    return new MPostpone(Tools);                                              // Вернуться к запуску заряда
  };  // MProfil

  //========================================================================= MUpCurrent

  /*  Начальный этап заряда - ток поднимается не выше заданного уровня, при достижении 
    заданного максимального напряжения переход к его удержанию. 
    Здесь и далее подсчитывается время и отданный заряд, а также сохраняется возможность
    прекращения заряда оператором. */

  // Состояние: "Подъем и удержание максимального тока"
  MUpCurrent::MUpCurrent(MTools * Tools) : MState(Tools)
  {   
      // Индикация входа в режим ConstCurrent
    Display->showMode((char*)"  CONST CURRENT   ");
    Display->showHelp((char*)"      C-STOP      ");
    Board->ledsGreen();
    
      // Обнуляются счетчики времени и отданного заряда
    Tools->clrTimeCounter();
    Tools->clrAhCharge();

    /* Включение преобразователя и коммутатора драйвером силовой платы.
     Параметры PID-регулятора заданы в настройках прибора (DEVICE).
     Здесь задаются сетпойнты по напряжению и току. Подъем тока
     производится ПИД-регулятором.
    */ 
    Tools->txPowerAuto(maxV * 1.05f, maxI);                                   // 0x20  Команда драйверу
  }     
  MUpCurrent::MState * MUpCurrent::fsm()
  {
    Tools->chargeCalculations();                                              // Подсчет отданных ампер-часов.

    switch ( Keyboard->getKey() )
    {
        // Досрочное прекращение заряда оператором
      case MKeyboard::C_CLICK:
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                        return new MStop(Tools);
      default:;
    }

    // // Проверка напряжения и переход на поддержание напряжения.
    // if(Tools->getRealVoltage() >= maxV)                                       return new MKeepVmax(Tools);
    
      // Индикация фазы подъема тока не выше заданного
    Display->showVolt(Tools->getRealVoltage(), 3);
    Display->showAmp (Tools->getRealCurrent(), 3);    // Может быть избыточно 3 знака
    Display->initBar(TFT_GREEN);
    Display->showDuration(Tools->getChargeTimeCounter(), MDisplay::SEC);
    Display->showAh(Tools->getAhCharge());
    return this;
  };  //MUpCurrent

  //========================================================================= MKeepVmax

  /*  Вторая фаза заряда - достигнуто заданное максимальное напряжение.
    Настройки регулятора не меняются, по факту состояние необходимо только для 
    изменения индикации.
    При падении тока ниже заданного уровня - переход к третьей фазе. */

    // Состояние: "Удержание максимального напряжения"
  MKeepVmax::MKeepVmax(MTools * Tools) : MState(Tools)
  {
    // Индикация помощи
    Display->showMode((char*)"  CONST VOLTAGE   ");
    Display->showHelp((char*)"     C-STOP       ");
    Board->ledsYellow();
    //Tools->txPowerAuto(maxV, maxI);                                         // 0x20  Команда драйверу
  }       
  MState * MKeepVmax::fsm()
  {
    Tools->chargeCalculations();                                              // Подсчет отданных ампер-часов.

    switch ( Keyboard->getKey() )
    {
        // Досрочное прекращение заряда оператором
      case MKeyboard::C_CLICK:                            
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                        return new MStop(Tools);
      default:;
    }
      // Ожидание спада тока ниже C/20 ампер.
    if(Tools->getRealCurrent() <= minI)     //     Tools->getCurrentMin())                     return new MKeepVmin(Tools);

    // Индикация фазы удержания максимального напряжения
    // Реальные ток и напряжения - без изменения, можно не задавать?
    Display->initBar(TFT_YELLOW);
    Display->showDuration(Tools->getChargeTimeCounter(), MDisplay::SEC);
    Display->showAh(Tools->getAhCharge());
    return this;
  };  // MKeepVmax

  //========================================================================= MKeepVmin

  // Третья фаза заряда - достигнуто снижение тока заряда ниже заданного предела.
  // Проверки различных причин завершения заряда.
  MKeepVmin::MKeepVmin(MTools * Tools) : MState(Tools)
  {
    timeOut = Tools->readNvsShort("options", "timeout", 0);
    // Индикация помощи
    Display->showMode((char*)" KEEP MIN VOLTAGE ");
    Display->showHelp((char*)"      C-STOP      ");
    Board->ledsYellow();
      // Порог регулирования по минимальному напряжению
    Tools->txPowerAuto(minV, maxI);                                           // 0x20  Команда драйверу
  }     
  MState * MKeepVmin::fsm()
  {
    Tools->chargeCalculations();                                              // Подсчет отданных ампер-часов.

    switch ( Keyboard->getKey() )
    {
      case MKeyboard::C_CLICK:    
      case MKeyboard::C_LONG_CLICK: Board->buzzerOn();                        return new MStop(Tools);
      default:;
    }
    // Здесь возможны проверки других условий окончания заряда
    // if( ( ... >= ... ) && ( ... <= ... ) )  { return new MStop(Tools); }

      // Максимальное время заряда, задается в "Настройках"
    //if(Tools->getChargeTimeCounter() >= (Tools->charge_time_out_limit * 36000))  return new MStop(Tools);
    if(Tools->getChargeTimeCounter() >= (timeOut * 36000))                    return new MStop(Tools);

    // Необходимая коррекция против выброса тока
//        if( Tools->getRealCurrent() > Tools->getCurrentMax() ) 
//        { Tools->adjustIntegral( -0.250f ); }        // -0.025A

    Display->initBar(TFT_MAGENTA);
    Display->showDuration(Tools->getChargeTimeCounter(), MDisplay::SEC);
    Display->showAh(Tools->getAhCharge());
    return this;
  };  // MKeepVmin

  //========================================================================= MStop

  // Завершение режима заряда - до нажатия кнопки "С" удерживается индикация 
  // о продолжительности и отданном заряде.
  // Состояние: "Завершение заряда"
  MStop::MStop(MTools * Tools) : MState(Tools)
  {

    Tools->txPowerStop();                                                     // 0x21 Команда драйверу отключить преобразователь

    Display->showMode((char*)"     POWER OFF    ");
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
    return this;
  };  // MStop

  //========================================================================= MExit
  // Процесс выхода из режима заряда - до нажатия кнопки "С" удерживается индикация о продолжительности и отданном заряде.
  // Состояние: "Индикация итогов и выход из режима заряда в меню диспетчера" 
  MExit::MExit(MTools * Tools) : MState(Tools)
  {
    //Tools->shutdownCharge();
    Display->showMode((char*)"  CC/CV MODE OFF  ");
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
        Display->showMode( (char*) "   CC/CV CHARGE:  " );
        Display->showHelp( (char*) "     B-SELECT     " );                    return nullptr;  // Возврат к выбору режима
      default:;
    }
    return this;
  };  // MExit

};  // !Конечный автомат режима заряда CCCV.
