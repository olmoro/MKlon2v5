#ifndef _MSUPERVISOR_H_
#define _MSUPERVISOR_H_

#include "stdint.h"

class MBoard;
class AutoPID;

class MSupervisor
{
public:
    MSupervisor(MBoard * Board);
    ~MSupervisor();

    void  runCool();
    void  setFan(uint16_t val);    
//    void  setCelsius(float val);  // Перемещен, не проверено
//    float getCelsius() const;
    int   getFanPwm()  const;

private:
    MBoard  * Board   = nullptr;
    AutoPID * coolPID = nullptr;
  
//    float celsius = 25.0f;

    // Fan parameters
    const float fan_pwm_max = 800.0f;
    const float fan_pwm_min = 50.0f;
    int fanPwm = fan_pwm_max;

    // PID
    const float k_p   = 50.0f;
    const float k_i   =  5.0f;
    const float k_d   =  0.0f;
    float coolSetPoint;
    float coolInput;
    float coolOutput;

};

#endif // !_MSUPERVISOR_H_
