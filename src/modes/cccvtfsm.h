#ifndef _CCCVTFSM_H_
#define _CCCVTFSM_H_

#include "state/mstate.h"

namespace MCccvt
{
  struct MConst
  {
    // static constexpr float fixedKpV =  0.02f;    //  15
    // static constexpr float fixedKiV =  0.20f;    // 
    // static constexpr float fixedKdV =  0.00f;    //   0 0x0000
    // static constexpr float fixedKpI =  0.02f;   //8f;    //  20
    // static constexpr float fixedKiI =  0.20f;    //  
    // static constexpr float fixedKdI =  0.00f;    //   0 0x0000 

    static constexpr float fixedKp =  0.02f;    //  15
    static constexpr float fixedKi =  0.20f;    // 
    static constexpr float fixedKd =  0.00f;    //   0 0x0000     
  };

    // Режимы работы PID-регулятора
  enum mode {MODE_OFF = 0, MODE_U, MODE_I, MODE_D, MODE_AUTO};

  class MStart : public MState
  {       
    public:
      MStart(MTools * Tools);
      MState * fsm() override;
    private:
      static constexpr float nominal_v_fixed  = 12.0f;
      static constexpr float capacity_fixed   = 55.0f;
      static constexpr float voltageMaxFactor =  1.234f;    // 12v  * 1.234 = 14.8v
      static constexpr float voltageMinFactor =  0.890f;    // 12v  * 0.89  = 10.7v
      static constexpr float currentMaxFactor =  0.100f;    // 55ah * 0.1   = 5,5A 
      static constexpr float currentMinFactor =  0.050f;    // 55ah * 0.05  = 2.75A
      short cnt;
      float voltageNom = 12.0;
      float capacity   = 55.0;
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

  class MSetCurrentMax : public MState
  {
    public:   
      MSetCurrentMax(MTools * Tools);
      MState * fsm() override;
    private:
        // Пределы регулирования max тока
      static constexpr float above = 6.0f;
      static constexpr float below = 0.2f;
  };
  
  class MSetVoltageMax : public MState
  {
    public:   
      MSetVoltageMax(MTools * Tools);
      MState * fsm() override;
    private:
        // Пределы регулирования max напряжения
      static constexpr float above = 16.2f;
      static constexpr float below = 10.0f;
  };

  class MSetCurrentMin : public MState
  {
    public:     
      MSetCurrentMin(MTools * Tools);
      MState * fsm() override;
    private:
        // Пределы регулирования min тока
      static constexpr float above = 6.0f;
      static constexpr float below = 0.2f;
  };

  class MSetVoltageMin : public MState
  {
    public:     
      MSetVoltageMin(MTools * Tools);
      MState * fsm() override;
    private:
        // Пределы регулирования min напряжения
      static constexpr float above = 16.2f;
      static constexpr float below = 10.0f;
  };

  class MPostpone : public MState
  {
    public:   
      MPostpone(MTools * Tools);
      MState * fsm() override;
    private:
      float kp, ki, kd;
  };

  // class MImportPID : public MState
  // {
  //   public:   
  //     MImportPID(MTools * Tools);
  //     MState * fsm() override;
  //   private:
  //     short k = 0;
  //     float kpV, kiV, kdV, kpI, kiI, kdI;
  // };

  class MUpCurrent : public MState
  {
    public:   
      MUpCurrent(MTools * Tools);
      MState * fsm() override;
    private:
            // min/max для параметров PID
      static constexpr float above = ((0x1ULL << 16)-1) >> 8;  // 0x00FF
      static constexpr float below =   0.00f;
      
      float i;
      // bool io = true;
      unsigned short k = 0;
      // float spV, spI;   
      // float kpV, kiV, kdV;
      float kp, ki, kd;
      // unsigned short state;
      // short mode = 1;
  };

  class MKeepVmax : public MState
  {
    public: 
      MKeepVmax(MTools * Tools);
      MState * fsm() override;
  };

  class MKeepVmin : public MState
  {
    public:   
      MKeepVmin(MTools * Tools);
      MState * fsm() override;
    private:
      short timeOut;    // Максимальное время заряда
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

#endif  // !_CCCVTFSM_H_
