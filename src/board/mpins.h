/*
  Файл    mpins.h
  Проект  MKlon2 -> Mklon2v5, v7
  Порты   ESP32
  pcb:    mklon2v5, mklon2v7
  30.11.2022
*/

#ifndef _MPINS_H_
#define _MPINS_H_

namespace MPins
{
  //                 имя          порт   функция      цепь pcb         примечание
  // Входы
  constexpr char celsius_pin      = 34;   // an       CELSIUS     Порт измерителя температуры
  constexpr char pow_good_pin     = 35;   // in       POW_CH      Порт проверки сетевого питания 
  constexpr char rdy_pin          = 27;   // in       READY       Порт готовности драйвера 
  constexpr char ts_pin           = 13;   // in       TS          Порт контроля кулера
  constexpr char key_pin          = 39;   // an       KEY         Порт клавиатуры

  // Выходы
  constexpr char pwm_fan_pin      = 15;   // out      FAN         Порт управления вентилятором
  constexpr char buz_pin          = 14;   // out      BUZ         Порт управления активным зуммером
  
  // Выходы управления светодиодами 
  constexpr char led_r_pin        = 04;   // out,pd   LEDR        Порт управления красным светодиодом
  constexpr char led_g_pin        = 25;   // out,pd   LEDG        Порт управления зеленым светодиодом
  constexpr char led_b_pin        = 02;   // out,pd   LEDB        Порт управления синим светодиодом
  
  #ifdef MKLON2V7
    // Порты дисплея и SD (справочно)                 Дисплей     microSD
    constexpr char vspi_sck_pin   = 18;   //          VSPI_SCL*   VSPI_SCK
    constexpr char vspi_mosi_pin  = 23;   //          VSPI_SDA*   VSPI_DI
    constexpr char vspi_miso_pin  = 19;   //            -         VSPI_DO
    constexpr char lcd_dc_pin     = 05;   //          LCD_DC*       -
    constexpr char lcd_res_pin    = -1;   //          EN*           -
    constexpr char lcd_cs_pin     = 33;   //          LCD_CS*       -           
    constexpr char sd_cs_pin      = 26;   //            -          SD_CS
    // * - справочно, настройки интерфейса дисплея задавать в mklon27_setup.h
  #else
    // Порты дисплея (справочно). Порт IO19 не занимать, зарезервирован для MISO 
    constexpr char vspi_scl_pin   = 18;   //          VSPI_SCL*    Порт тактовый
    constexpr char vspi_sda_pin   = 23;   //          VSPI_SDA*    Порт данных
    constexpr char lcd_res_pin    = -1; //19;   //          LCD_RES*     Порт рестарта (ОШИБКА переподключить к EN)
    constexpr char lcd_dc_pin     = 05;   //          LCD_DC*      Порт выбора
    constexpr char lcd_cs_pin     = -1;   //          Grounded*
    // * - справочно, настройки интерфейса дисплея задавать в mklon25_setup.h
  #endif

  // UART
  constexpr char u2rxd_pin        = 16;   // in       U2RXD       Порт приемника
  constexpr char u2txd_pin        = 17;   // out      U2TXD       Порт передатчика

  // I2C 
  constexpr char i2c2_sda_pin     = 21;   // io       SDA         Порт данных
  constexpr char i2c2_scl_pin     = 22;   // out      SCL         Порт тактовый

  #ifdef MKLON2V7
    constexpr char rtc_sqw_pin    = 32;    // in     SQW
    constexpr char io36_pin       = 36;    // in     IO36        Резерв 
    constexpr char io12_pin       = 12;    // io     IO12        Резерв 
  #endif
};

#endif // !_MPINS_H_






// /*
//   Файл   mpins.h
//   Проект MESP32v7 -> Mklon
//   Порты ESP32
//   pcb: mesp.v7.1  -> mklon2v5
// */

// #ifndef _MPINS_H_
// #define _MPINS_H_

// namespace MPins
// {
//   //                 имя          порт   функция      цепь pcb         примечание
//   // Входы
//   constexpr char celsius_pin      = 34;   // an       CELSIUS     Порт измерителя температуры
//   constexpr char pow_good_pin     = 35;   // in       POW_CH      Порт проверки сетевого питания 
//   constexpr char rdy_pin          = 27;   // in       READY       Порт готовности драйвера 
//   constexpr char ts_pin           = 13;   // in       TS          Порт контроля кулера
//   constexpr char key_pin          = 39;   // an       KEY         Порт клавиатуры

//   // Выходы
//   constexpr char pwm_fan_pin      = 15;   // out      FAN         Порт управления вентилятором
//   constexpr char buz_pin          = 14;   // out      BUZ         Порт управления активным зуммером
  
//   // Выходы управления светодиодами 
//   constexpr char led_r_pin        = 04;   // out,pd   LEDR        Порт управления красным светодиодом
//   constexpr char led_g_pin        = 25;   // out,pd   LEDG        Порт управления зеленым светодиодом
//   constexpr char led_b_pin        = 02;   // out,pd   LEDB        Порт управления синим светодиодом
  
//   // USPI для дисплея 128*160. Порт IO12 как MISO не используется
//   constexpr char uspi_scl_pin     = 18;   // out      USPI_SCL    Порт тактовый
//   constexpr char uspi_sda_pin     = 23;   // out      USPI_SDA    Порт данных
//   constexpr char lcd_res_pin      = 19;   // out      LCD_RES     Порт рестарта
//   constexpr char lcd_dc_pin       = 05;   //          LCD_DC      Порт выбора
//   constexpr char lcd_cs_pin       = -1;   //          LCD_CS      Grounded

//   // UART
//   constexpr char u2rxd_pin        = 16;   // in       U2RXD       Порт приемника
//   constexpr char u2txd_pin        = 17;   // out      U2TXD       Порт передатчика

//   // I2C 
//   constexpr char i2c2_sda_pin     = 21;   // io       SDA        Порт данных
//   constexpr char i2c2_scl_pin     = 22;   // out      SCL        Порт тактовый

// };

// #endif // !_MPINS_H_
