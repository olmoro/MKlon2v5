#ifndef _DEVICEFSM_H_
#define _DEVICEFSM_H_

#include "state/mstate.h"

namespace MDevice
{
    /* Константы режима */
  struct MConst
  {
    //static constexpr short name = 0;
  };

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

  class MShiftV : public MState
  {       
    public:
      MShiftV(MTools * Tools);
      MState * fsm() override;
    private:
      short shift;
      static constexpr short fixed =    0;
      static constexpr short above = fixed + 200;
      static constexpr short below = fixed - 200;
  };

  class MFactorV : public MState
  {       
    public:
      MFactorV(MTools * Tools);
      MState * fsm() override;
    private:
      short factor;
      //static constexpr short fixed =      0x2D4F;
      //static constexpr short fixed =      0x43C7;
      //static constexpr short fixed =      0x2CEC;
      static constexpr short fixed =      0x4A4A;
      static constexpr short above = fixed + 200;   // Около 2%
      static constexpr short below = fixed - 200;
  };

  class MSmoothV : public MState
  {       
    public:
      MSmoothV(MTools * Tools);
      MState * fsm() override;
    private:
      short smooth;
      static constexpr short fixed = 3;
      static constexpr short above = 7;
      static constexpr short below = 1;      
  };

  class MShiftI : public MState
  {       
    public:
      MShiftI(MTools * Tools);
      MState * fsm() override;
    private:
      short shift;
      static constexpr short fixed =    0;
      static constexpr short above =  200;
      static constexpr short below = -200;
  };

  class MFactorI : public MState
  {       
    public:
      MFactorI(MTools * Tools);
      MState * fsm() override;
    private:
      short factor;
      //static constexpr short fixed =     0x079D;
      static constexpr short fixed =     0x030D;
      static constexpr short above = fixed + 50;
      static constexpr short below = fixed - 50;
  };

  class MSmoothI : public MState
  {       
    public:
      MSmoothI(MTools * Tools);
      MState * fsm() override;
    private:
      short smooth;
      static constexpr short fixed = 3;
      static constexpr short above = 7;
      static constexpr short below = 1; 
  };

  // class MClearAllKeys : public MState
  // {
  //   public:  
  //     MClearAllKeys(MTools * Tools);
  //     MState * fsm() override;
  // };

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

#endif  // !_DEVICEFSM_H_
