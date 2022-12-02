#ifndef _DISCHARGEFSM_H_
#define _DISCHARGEFSM_H_

#include "state/mstate.h"

namespace MDisch
{
  struct MConst
  {
    // Параметры условий разряда
    static constexpr float discharge_i_fixed  = -1.0f;
  };

  class MStart : public MState
  {       
    public:
      MStart(MTools * Tools);
      MState * fsm() override;
    private:
      float maxI;
      short cnt;
  };

  class MClearDischargeKeys : public MState
  {
    public:  
      MClearDischargeKeys(MTools * Tools);
      MState * fsm() override;
    private:
      short cnt;
      bool done;
  };

  class MSetCurrent : public MState
  {
    public:   
      MSetCurrent(MTools * Tools);
      MState * fsm() override;
    private:
        // Пределы регулирования тока
      //static constexpr float above = 3.0f;
      //static constexpr float below = 0.0f;
      static constexpr float above =  0.0f;
      static constexpr float below = -3.0f;

      float maxI;
  };

  class MGo : public MState
  {
    public:   
      MGo(MTools * Tools);
      MState * fsm() override;
    private:
        // Пределы регулирования тока "на лету".
      static constexpr float above =  0.0f;
      static constexpr float below = -3.0f;
      
      bool io = true;
      float spD;   
  };










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

#endif  // _DISCHARGEFSM_H_
