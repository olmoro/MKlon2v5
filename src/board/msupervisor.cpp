/*
  Функции супервайзера в основном ограничиваются контролем за работой персонала.
                                                        Супервайзер — Википедия

  1. Отслеживание температуры, регулирование скорости вентилятора
  2. Отслеживание сетевого напряжения - будет перенесено сюда
*/

#include "board/msupervisor.h"
#include "board/mboard.h"
#include <AutoPID.h>
#include <Arduino.h>


MSupervisor::MSupervisor(MBoard * Board) : Board(Board)
{
  coolPID = new AutoPID(&coolSetPoint, &coolInput, &coolOutput, fan_pwm_min, fan_pwm_max, k_p, k_i, k_d);
  setFan( fan_pwm_max );
  coolPID->setBangBang( 30.0f, 5.0f );
  coolSetPoint = 50.0f;   // Температурный порог хорошо бы задать в нестройках  
}

MSupervisor::~MSupervisor() { delete coolPID; }

// Управление охлаждением
void MSupervisor::setFan(uint16_t val) 
{
  // If less than minimum is specified - stop
  if ( val <= fan_pwm_min ) val = 0;

  fanPwm = val;
  ledcWrite( MBoard::ch_fan, val );         // Так задается ШИМ
}

void MSupervisor::runCool() 
{
  coolInput = Board->getCelsius();    //          celsius;
  coolPID->run();
  setFan( coolOutput );
}

// Пока измеритель температуры не здесь - так уж сложилось
//void  MSupervisor::setCelsius(float value) { celsius = value; }
//float MSupervisor::getCelsius() const { return celsius; }
int   MSupervisor::getFanPwm()  const { return fanPwm; }



// Контроль сетевого питания


// Измерение температуры и сетевого напряжения

