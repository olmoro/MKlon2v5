/*  Протокол WAKE является логическим уровнем интерфейса управления оборудованием с помощью 
  асинхронного последовательного канала. Протокол позволяет производить обмен 
  пакетами данных (data frames) длиной до 255 байт с адресуемыми устройствами, которых может 
  быть до 127. Формат данных может быть любым. Могут передаваться байтовые поля, многобайтовые 
  слова или строки символов. Для контроля правильности передачи данных используется контрольная 
  сумма (CRC-8).

  Подробная информация о WAKE: http://leoniv.diod.club/articles/wake/wake.html
*/

#ifndef _MWAKE_H_
#define _MWAKE_H_

#include "stdint.h"

class MWake
{
  public:

    MWake();
    ~MWake();

    static constexpr uint8_t frame    = 0xFF; // Максимальная длина пакета 255 (полезных данных)

    void wakeInit( uint8_t addr, long time );
    void wakeRead();

    // передача ответа на команду
    void txReplay(char n, char err);
    void configReply(char n, char err, uint8_t comm);   // 
    void configAsk(char n, uint8_t comm);         // для активного контроллера - без байта подтверждения

    // работа с буфером приема
    uint8_t  getCommand();
    uint8_t  get08(int i);
    uint16_t get16(int i);
    float    getF16(int i);

    uint8_t  getNbt();                        // получить число байт в ответе


    // работа с буфером передачи
    void setU8(int id, uint8_t value);

    int replyU08(int id, uint8_t  value);
    int replyU16(int id, uint16_t value);
    int replyU32(int id, uint32_t value);



    void testReply( int n );                  // тест отправить n байт из буфера приемника

  private:

    void doCrc8(char b, char *crc);
    void wakeWrite();
    void wakeStartWrite();

  public:

    // Коды универсальных команд:
    static constexpr uint8_t cmd_nop  = 0x00; // нет операции
    static constexpr uint8_t cmd_err  = 0x01; // ошибка приема пакета
    static constexpr uint8_t cmd_echo = 0x02; // передать эхо
    static constexpr uint8_t cmd_info = 0x03; // передать информацию об устройстве

  private:

};

#endif  //!_MWAKE_H_
