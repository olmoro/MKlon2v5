#ifndef _OPTIONSFSM_H_
#define _OPTIONSFSM_H_

/*
 * Доступные пользователю регулировки измерителей,
 *
 *
 */

#include "state/mstate.h"

namespace MOption
{
    struct MConst
  {
    // Константы, общие для состояний: 
    static constexpr unsigned short postpone_fixed  =  0U;
    static constexpr unsigned short timeout_fixed   =  5U;
    static constexpr float nominal_v_fixed          = 12.0F;
    static constexpr float capacity_fixed           = 55.0F;
  };

  class MStart : public MState
  {       
    public:
      MStart(MTools * Tools);
      MState * fsm() override;
    private:
      short cnt;
  };

  class MClearOptionsKeys : public MState
  {
    public:  
      MClearOptionsKeys(MTools * Tools);
      MState * fsm() override;
    private:
      short cnt;
      bool done;
  };

  class MSetPostpone : public MState
  {       
    public:
      MSetPostpone(MTools * Tools);
      MState * fsm() override;
    private:
        // Пределы регулирования задержки пуска, час
  //    static constexpr short fixed =  0;
      static constexpr short above = 23;
      static constexpr short below =  0;

      short postpone;
  };

  class MTimeout : public MState
  {       
    public:
      MTimeout(MTools * Tools);
      MState * fsm() override;
    private:
        // Пределы регулирования максимального времени заряда, час
  //    static constexpr short fixed =  5;
      static constexpr short above =  5;
      static constexpr short below =  0;

      short timeout;
  };

  class MNominalV : public MState
  {       
    public:
      MNominalV(MTools * Tools);
      MState * fsm() override;
    private:
        // Пределы регулирования 
  //    static constexpr float fixed = 12.0f;
      static constexpr float above = 12.0f;
      static constexpr float below =  2.0f;

      float val;
  };

  class MCapacity : public MState
  {       
    public:
      MCapacity(MTools * Tools);
      MState * fsm() override;
    private:
        // Пределы регулирования 
  //    static constexpr float fixed = 55.0f;
      static constexpr float above = 60.0f;
      static constexpr float below =  5.0f;

      float val;
  };


  class MNext : public MState
  {       
    public:
      MNext(MTools * Tools);
      MState * fsm() override;
    private:
        // Пределы регулирования 
//      static constexpr short fixed =  0;
      static constexpr short above =  0;
      static constexpr short below =  0;

      short val;
  };

  class MLast : public MState
  {       
    public:
      MLast(MTools * Tools);
      MState * fsm() override;
    private:
        // Пределы регулирования 
//      static constexpr short fixed =  0;
      static constexpr short above =  0;
      static constexpr short below =  0;

      short val;
  };

//***************

  class MStop : public MState
  {
    public:  
      MStop(MTools * Tools);   
      MState * fsm() override;
  };

  class MExit : public MState
  {
    public:  
      MExit(MTools * Tools);   
      MState * fsm() override;
  };
};

#endif // !_OPTIONSFSM_H_