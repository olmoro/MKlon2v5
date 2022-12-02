/*
  Методы работы с аппаратными ресурсами платы
  pcb: mesp_v7.1 -> Mklon
  20221023
*/

//#include <Wire.h>
#include "board/mboard.h"
#include "board/mpins.h"
#include "board/msupervisor.h"
#include <Arduino.h>

MBoard::MBoard()
{
	initEsp32();
  Supervisor = new MSupervisor(this);
}

  // Управление светодиодами
  
void MBoard::ledROn()	 { digitalWrite(MPins::led_r_pin, LOW ); }   // Turn the RED LED on
void MBoard::ledROff() { digitalWrite(MPins::led_r_pin, HIGH); }  // Turn the RED LED off
void MBoard::ledGOn()	 { digitalWrite(MPins::led_g_pin, LOW ); }  // Turn the GREEN LED on
void MBoard::ledGOff() { digitalWrite(MPins::led_g_pin, HIGH); }  // Turn the GREEN LED off
void MBoard::ledBOn()	 { digitalWrite(MPins::led_b_pin, LOW ); }  // Turn the BLUE LED on
void MBoard::ledBOff() { digitalWrite(MPins::led_b_pin, HIGH); }  // Turn the BLUE LED off

  // Управление цветом светодиода

void MBoard::ledsOn()	    { ledROn();  ledGOn();  ledBOn();  }    // Белый
void MBoard::ledsOff()	  { ledROff(); ledGOff(); ledBOff(); }    // Погасить все
void MBoard::ledsRed()    { ledROn();  ledGOff(); ledBOff(); }    // Красный
void MBoard::ledsGreen()  { ledGOn();  ledROff(); ledBOff(); }    // Зеленый
void MBoard::ledsBlue()	  { ledROff(); ledGOff(); ledBOn();  }    // Синий                            
void MBoard::ledsYellow() { ledGOn();  ledROn();  ledBOff(); }    // Желтый

  // Ввести таймер ... в прошлой версии использовался подсчет входов в дисплей : оформить задачей
void MBoard::blinkWhite(short cnt)  { cnt % 10 > 5 ? ledsOn()     : ledsOff(); }
void MBoard::blinkWhite()
{
  static short cnt = 0;
  cnt++;
  cnt % 10 > 5 ? ledsOn() : ledsOff(); 
}

void MBoard::blinkRed(short cnt)    { cnt % 10 > 5 ? ledsRed()    : ledsOff(); }
void MBoard::blinkGreen(short cnt)  { cnt % 10 > 5 ? ledsGreen()  : ledsOff(); }
void MBoard::blinkBlue(short cnt)   { cnt % 10 > 5 ? ledsBlue()   : ledsOff(); }
void MBoard::blinkYellow(short cnt) { cnt % 10 > 5 ? ledsYellow() : ledsOff(); }

  // Управление зуммером
void MBoard::buzzerOn()	 { digitalWrite(MPins::buz_pin, LOW ); }
void MBoard::buzzerOff() { digitalWrite(MPins::buz_pin, HIGH); }

void MBoard::lcdRstOn()  { digitalWrite(MPins::lcd_res_pin, LOW ); }  // или наоборот?
void MBoard::lcdRstOff() { digitalWrite(MPins::lcd_res_pin, HIGH); }

  // Вход проверки подачи питания (пока без преобразования в вольты)
int16_t MBoard::getAdcPG() { return analogRead( MPins::pow_good_pin ); }

  // Вычисление реальной температуры, результат в celsius
void MBoard::calculateCelsius() { celsius = readSteinhart( analogRead( MPins::celsius_pin ) ); }

float MBoard::getCelsius() { return celsius; }  //30


    void MBoard::initAdcT11db0() { adcAttachPin( MPins::celsius_pin );  analogSetPinAttenuation( MPins::celsius_pin,  ADC_11db); }
    void MBoard::initAdcK11db0() { adcAttachPin( MPins::key_pin );  analogSetPinAttenuation( MPins::key_pin,  ADC_11db); }
    int MBoard::getAdcT() { return analogRead( MPins::celsius_pin ); }
    int MBoard::getAdcK() { return analogRead( MPins::key_pin ); }


  // Инициализация ресурсов ESP32
void MBoard::initEsp32() 
{
    // ADC - все измерения 12 бит 
  analogSetWidth(12);                                       //Sets the sample bits and read resolution

    // ADC измерителя температуры
  adcAttachPin( MPins::celsius_pin );                       // Attach a pin to ADC
  analogSetPinAttenuation( MPins::celsius_pin, ADC_11db);   // Set the input attenuation
  
    // Светодиоды
  pinMode( MPins::led_r_pin, OUTPUT_OPEN_DRAIN );
  pinMode( MPins::led_g_pin, OUTPUT_OPEN_DRAIN );
  pinMode( MPins::led_b_pin, OUTPUT_OPEN_DRAIN );
  ledROff();
  ledGOff();
  ledBOff();

		  // Порт наличия питания (аналоговый)
  adcAttachPin( MPins::pow_good_pin );                     // Attach a pin to ADC
  analogSetPinAttenuation( MPins::pow_good_pin, ADC_11db); // Set the input attenuation

    // Порт сброса дисплея
  pinMode(MPins::lcd_res_pin, OUTPUT );

	  // Настройка ШИМ вентилятора
	ledcSetup( ch_fan, 20000, 10);              // канал, частота, разрядность
	ledcAttachPin( MPins::pwm_fan_pin, ch_fan );
	ledcWrite( ch_fan, 0x0000 );	// Начальная установка ШИМ в 0. (или лучше в максимум?)

}

  // Преобразование данных АЦП в напряжение с коррекцией линейности 
  // https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function/blob/master/ESP32_ADC_Read_Voltage_Accurate.ino
float MBoard::readVoltage( int adc )
{
  // Reference voltage is 3v3 = 4095 in range 0 to 4095
  if ( adc < 1 || adc >= 4095 ) return 0;
  const float adc2 = adc  * adc;
  const float adc3 = adc2 * adc;
  return -0.000000000000016f * adc3 * adc
        + 0.000000000118171f * adc3
        - 0.000000301211691f * adc2
        + 0.001109019271794f * adc
        + 0.034143524634089f;
}

  // Преобразование данных АЦП в градусы цельсия     °С ( Alt+ 0 1 7 6 )
float MBoard::readSteinhart( const int adc )
{
// https://neyasyt.ru/uploads/files/termistor-NTC-10-K-MF52.pdf
  float steinhart;
  float tr = 3.30f / readVoltage( adc ) - 1.0f;

  tr = reference_resistance / tr;
  steinhart = tr / nominal_resistance;                  // (R/Ro)
  steinhart = log(steinhart);                           // ln(R/Ro)
  steinhart /= b_value;                                 // 1/B * ln(R/Ro)
  steinhart += 1.0f / (nominal_temperature + 273.15f);  // + (1/To)
  steinhart = 1.0f / steinhart;                         // Invert
  steinhart -= 273.15f;
  if ( steinhart == -273.15f ) steinhart = 120.0f;
  return ( steinhart > 120.0f ) ? 120.0f : steinhart;   // В случае обрыва датчика  = 120
}
