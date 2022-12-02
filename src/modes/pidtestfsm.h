#ifndef _PIDTESTFSM_H_
#define _PIDTESTFSM_H_

#include "state/mstate.h"

namespace MPidtest
{
    // Режимы работы PID-регулятора
  enum mode {MODE_OFF = 0, MODE_V, MODE_I, MODE_D, MODE_AUTO};
  
  struct MConst
  {
    static constexpr float fixedV   = 14.0;
    static constexpr float fixedI   =  2.0;
    static constexpr short fixedM   = MODE_V;
                                                // драйверу (для проверки кода команды)
    static constexpr float fixedKpV =  0.06f;    //  15
    static constexpr float fixedKiV =  0.40f;    // 
    static constexpr float fixedKdV =  0.00f;    //   0 0x0000
    static constexpr float fixedKpI =  0.08f;    //  20
    static constexpr float fixedKiI =  0.40f;    //  
    static constexpr float fixedKdI =  0.00f;    //   0 0x0000  
  };
 
  class MStart : public MState
  {       
    public:
      MStart(MTools * Tools);
      MState * fsm() override;
    private:
      static constexpr short above  = MODE_D;
      static constexpr short below  = MODE_OFF;
      short cnt;
      short mode;
  };

  class MClearPidKeys : public MState
  {
    public:  
      MClearPidKeys(MTools * Tools);
      MState * fsm() override;
    private:
      short cnt;
      bool done;
  };

  class MSetpointV : public MState
  {
    public:  
      MSetpointV(MTools * Tools);
      MState * fsm() override;
    private:
      float spV;
      float spI;
      static constexpr float above  = 17.0;
      static constexpr float below  =  2.0;
  };

  class MSetpointI : public MState
  {
    public:  
      MSetpointI(MTools * Tools);
      MState * fsm() override;
    private:
      float spV;
      float spI;
      static constexpr float above  =  6.0;
      static constexpr float below  =  0.1;
  };

  class MGo : public MState
  {
    public:  
      MGo(MTools * Tools);
      MState * fsm() override;
    private:
      bool io = true;
      unsigned short k = 0;
      float spV, spI;   
      float kpV, kiV, kdV;
      float kpI, kiI, kdI;
      unsigned short state;
      short mode = 1;

        // min/max для параметров PID
      static constexpr float above = ((0x1ULL << 16)-1) >> 8;  // 0x00FF
      static constexpr float below =   0.00f;
  };

  class MExportPID : public MState
  {
    public:   
      MExportPID(MTools * Tools);
      MState * fsm() override;
    private:
      short k = 0;
      float kpV, kiV, kdV, kpI, kiI, kdI;
      
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

#endif  // !_PIDTESTFSM_H_
