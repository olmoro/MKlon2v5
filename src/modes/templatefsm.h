#ifndef _TEMPLATEFSM_H_
#define _TEMPLATEFSM_H_

#include "mtools.h"                 // Методы для реализации пользовательских алгоритмов.
#include "board/mboard.h"           // Ресурсы платы управления и методы их использования.  
#include "display/mdisplay.h"       // Ресурсы дисплея и методы отбражения
#include "state/mstate.h"           // Методы управления конечным автоматом

// Переименуйте поле имен для вашего режима.
namespace Template
{
  // Какие-то константы для этого режима могут быть определены здесь, например
  struct MConsts
  {
    // Пределы регулирования
    static constexpr float i_l =  0.2f;
    static constexpr float i_h = 12.2f;
    static constexpr float v_l = 10.0f;
    static constexpr float v_h = 16.0f;

    // Параметры условий заряда
    static constexpr float voltageMaxFactor     = 1.234f;    // 12v  * 1.234 = 14.8v
    static constexpr float voltageMinFactor     = 0.890f;    // 12v  * 0.89  = 10.7v
    static constexpr float currentMaxFactor     = 0.100f;    // 55ah * 0.1   = 5,5A 
    static constexpr float currentMinFactor     = 0.050f;    // 55ah * 0.05  = 2.75A
  };   


  // Класс MStart должен быть указан в меню диспетчера режимов,
  // напрмер: State = new TemplateFsm::MStart(Tools); 
  // для запуска вашего режима.
  class MStart : public MState
  {       
    public:
      MStart(MTools * Tools);    
      MState * fsm() override;
    private:
      short cnt;
  };

  class MClearCccvKeys : public MState
  {
    public:  
      MClearCccvKeys(MTools * Tools);
      MState * fsm() override;
    private:
      short cnt;
      bool done;
  };

  class MYellow : public MState
  {
    public:
      MYellow(MTools * Tools); 
      MState * fsm() override;
  };
      
  class MRed : public MState
  {
    public:   
      MRed(MTools * Tools);
      MState * fsm() override;
  };
  
  class MGreen : public MState
  {
    public:   
      MGreen(MTools * Tools);
      MState * fsm() override;

    private:
      // Здесь располагаются переменные и константы, доступные в этом классе 
      int cnt;    // счетчик
      const int duration = 10 * 10;   // 10 секунд по 10 вызовов за секунду.
  };
  
  class MBlue : public MState
  {
    public:   
      MBlue(MTools * Tools);    
      MState * fsm() override;
  };

  // Выключение преобразователя и коммутатора, установка в начальное состояние 
  // ШИМ-генераторов, включение красного светодиода тоже здесь.
  class MStop : public MState
  {
    public:  
      MStop(MTools * Tools);         
      MState * fsm() override;
  };

};

#endif // !_TEMPLATEFSM_H_