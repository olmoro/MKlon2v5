#ifndef _BOOTFSM_H_
#define _BOOTFSM_H_

#include "state/mstate.h"

namespace MBoot
{
  class MStart : public MState
  {       
    public:
      MStart(MTools * Tools);
      MState * fsm() override;
  };

  class MTxPowerStop : public MState
  {
    public:   
      MTxPowerStop(MTools * Tools);
      MState * fsm() override;
  };

  class MTxSetMult : public MState
  {
    public:   
      MTxSetMult(MTools * Tools);
      MState * fsm() override;
  };



  class MTxsetFactorV : public MState
  {
    public:   
      MTxsetFactorV(MTools * Tools);
      MState * fsm() override;
  };

  class MTxSmoothV : public MState
  {
    public:   
      MTxSmoothV(MTools * Tools);
      MState * fsm() override;
  };

  class MTxShiftV : public MState
  {
    public:   
      MTxShiftV(MTools * Tools);
      MState * fsm() override;
  };

  class MTxFactorI : public MState
  {
    public:   
      MTxFactorI(MTools * Tools);
      MState * fsm() override;
  };

  class MTxSmoothI : public MState
  {
    public:   
      MTxSmoothI(MTools * Tools);
      MState * fsm() override;
  };
  
  class MTxShiftI : public MState
  {
    public:   
      MTxShiftI(MTools * Tools);
      MState * fsm() override;
  };

  class MExit : public MState
  {
    public:
      MExit(MTools * Tools);
      MState * fsm() override;
  };

};

#endif  //_BOOTFSM_H_
