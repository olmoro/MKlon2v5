#ifndef _MCMD_H_
#define _MCMD_H_
/*
  Коды команд обмена с драйвером силовой платы
  2021 май
  pcb: 
*/

#include "stdint.h"

namespace MCmd
{
    // Команды измерений
  constexpr uint8_t cmd_get_uis                  = 0x10; // Чтение напряжения(мВ), тока (мА) и состояния
  constexpr uint8_t cmd_get_u                     = 0x11; // Чтение напряжения (мВ)
  constexpr uint8_t cmd_get_i                     = 0x12; // Чтение тока (мА)
  constexpr uint8_t cmd_get_ui                    = 0x13; // Чтение напряжения (мВ) и тока (мА)
  constexpr uint8_t cmd_get_state                 = 0x14; // Чтение состояния
  constexpr uint8_t cmd_get_celsius               = 0x15; // Чтение температуры радиатора

    // Команды управления
  constexpr uint8_t cmd_power_auto                = 0x20; // старт преобразователя с автоматическим выбором регулятора U и I
  constexpr uint8_t cmd_power_stop                = 0x21; // отключение ( и разряда в том числе)
  constexpr uint8_t cmd_power_mode                = 0x22; // старт с выбором режима PID-регулятора

  constexpr uint8_t cmd_discharge_go              = 0x24; // старт разряда с отключением преобразователя

  // Команды работы с измерителем напряжения
  // Множитель преобразования в милливольты
  constexpr uint8_t cmd_read_factor_u             = 0x30; // Чтение
  constexpr uint8_t cmd_write_factor_u            = 0x31; // Запись
  constexpr uint8_t cmd_write_factor_default_u    = 0x32; // Возврат к заводскому
  // Параметр сглаживания
  constexpr uint8_t cmd_read_smooth_u             = 0x33; // Чтение
  constexpr uint8_t cmd_write_smooth_u            = 0x34; // Запись
  // Приборное смещение
  constexpr uint8_t cmd_read_offset_u             = 0x35; // Чтение
  constexpr uint8_t cmd_write_offset_u            = 0x36; // Запись

    // Команды работы с измерителем тока
    // Множитель преобразования в миллиамперы
  constexpr uint8_t cmd_read_factor_i             = 0x38; // Чтение
  constexpr uint8_t cmd_write_factor_i            = 0x39; // Запись
  constexpr uint8_t cmd_write_factor_default_i    = 0x3A; // Возврат к заводскому
    // Параметр сглаживания
  constexpr uint8_t cmd_read_smooth_i             = 0x3B; // Чтение
  constexpr uint8_t cmd_write_smooth_i            = 0x3C; // Запись
    // Приборное смещение
  constexpr uint8_t cmd_read_offset_i             = 0x3D; // Чтение
  constexpr uint8_t cmd_write_offset_i            = 0x3E; // Запись

    // ПИД-регулятор
  constexpr uint8_t cmd_pid_configure             = 0x40; // set mode, kp, ki, kd, min, max
  constexpr uint8_t cmd_pid_write_coefficients    = 0x41; // set kp, ki, kd
  constexpr uint8_t cmd_pid_output_range          = 0x42; // set min, max
  constexpr uint8_t cmd_pid_reconfigure           = 0x43; // set kp, ki, kd,min, max w/o clear
  constexpr uint8_t cmd_pid_clear                 = 0x44; // clear
  
  constexpr uint8_t cmd_pid_test                  = 0x46; // mode, setpoint, sw
  constexpr uint8_t cmd_pid_read_param       = 0x47; // param: paramMult, paramMax, pidHz 
  constexpr uint8_t cmd_pid_read_configure        = 0x48; // mode, kP, kI, kD, min, max - возвращает параметры текущего режима регулирования
    //    constexpr uint8_t cmd_pid_write_max_sum         = 0x49; // Задает максимальный интеграл при вычислении шага рег
  constexpr uint8_t cmd_pid_up_current            = 0x4A; // Для фазы подъема тока - резерв
  //constexpr uint8_t cmd_set_cooler                = 0x4F; // Задать скорость вентилятора

    // АЦП - настройки
  constexpr uint8_t cmd_adc_read_probes           = 0x50; // Read all probes
  constexpr uint8_t cmd_adc_read_offset           = 0x51; // Читать смещение АЦП
  constexpr uint8_t cmd_adc_write_offset          = 0x52; // Запись смещения АЦП
  constexpr uint8_t cmd_adc_auto_offset           = 0x53; // Автоматическая компенсация смещения АЦП (пока нет)

    // Команды тестовые
  constexpr uint8_t cmd_write_switch_pin          = 0x54; // sw_pin D13 (PA17) Управление силовыми ключами SAMD21 MINI 

  constexpr uint8_t cmd_write_power               = 0x56; // пользоваться с осторожностью - выяснение пределов регулирования
  constexpr uint8_t cmd_write_discharge           = 0x57; // не проверена
  constexpr uint8_t cmd_write_voltage             = 0x58; // старая, не проверена
  constexpr uint8_t cmd_write_current             = 0x59; // старая, не проверена 
  constexpr uint8_t cmd_write_discurrent          = 0x5A; // старая, не проверена
  constexpr uint8_t cmd_write_surge_compensation  = 0x5B; // параметры подавления всплеска напряжения na
  constexpr uint8_t cmd_write_idle_load           = 0x5C; // параметры доп.нагрузки ХХ

    // Команды задания порогов отключения
  constexpr uint8_t cmd_get_lt_v                  = 0x60; // 
  constexpr uint8_t cmd_set_lt_v                  = 0x61; // 
  constexpr uint8_t cmd_set_lt_default_v          = 0x62; // 
  constexpr uint8_t cmd_get_up_v                  = 0x63; // 
  constexpr uint8_t cmd_set_up_v                  = 0x64; // 
  constexpr uint8_t cmd_set_up_default_v          = 0x65; // 

  constexpr uint8_t cmd_get_lt_i                  = 0x68; // 
  constexpr uint8_t cmd_set_lt_i                  = 0x69; // 
  constexpr uint8_t cmd_set_lt_default_i          = 0x6A; // 
  constexpr uint8_t cmd_get_up_i                  = 0x6B; // 
  constexpr uint8_t cmd_set_up_i                  = 0x6C; // 
  constexpr uint8_t cmd_set_up_default_i          = 0x6D; // 

    // Команды универсальные
  constexpr uint8_t cmd_nop                       = 0x00; // нет операции
  constexpr uint8_t cmd_err                       = 0x01; // 
  constexpr uint8_t cmd_echo                      = 0x02; // 
  constexpr uint8_t cmd_info                      = 0x03; // 

};

#endif // !_MCMD_H_
