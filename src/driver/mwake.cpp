#include <Arduino.h>
#include "mwake.h"

MWake::MWake() {}
MWake::~MWake() {}

// Константы протокола
static constexpr uint8_t fend     = 0xC0; // Frame END
static constexpr uint8_t fesc     = 0xDB; // Frame ESCape
static constexpr uint8_t tfend    = 0xDC; // Transposed Frame END
static constexpr uint8_t tfesc    = 0xDD; // Transposed Frame ESCape
static constexpr uint8_t crc_init = 0xDE; // Initial CRC value
static constexpr uint8_t frame    = 0xFF; // Максимальная длина пакета 255 (полезных данных)

char          rxSta;        // состояние процесса приема пакета
char          rxPre;        // предыдущий принятый байт
char          rxAdd;        // адрес, с которым сравнивается принятый
char          rxCmd;        // принятая команда
char          rxCrc;        // контрольная сумма принимаемого пакета
unsigned char rxPtr;        // указатель на массив принимаемых данных

char          txSta;        // состояние процесса передачи пакета
char          txPre;        // предыдущий переданный байт
char          txAdd;        // адрес, передававемый в пакете
char          txCrc;        // контрольная сумма передаваемого пакета
unsigned char txPtr;        // указатель на массив передаваемых данных

//RX process states:
enum { wait_fend = 0,   // ожидание приема FEND
      wait_addr,       // ожидание приема адреса
      wait_cmd,        // ожидание приема команды
      wait_nbt,        // ожидание приема количества байт в пакете
      wait_data,       // прием данных
      wait_crc,        // ожидание окончания приема CRC nu
      wait_carr };     // ожидание несущей nu

//TX process states:
enum { send_idle = 0,   // состояние бездействия == 0
      send_addr,       // передача адреса
      send_cmd,        // передача команды
      send_nbt,        // передача количества байт в пакете
      send_data,       // передача данных
      send_crc,        // передача CRC
      send_end };      // окончание передачи пакета nu

// Переменные - уточнить типы  
char    rxNbt;          // принятое количество байт в пакете
char    rxDat[frame];   // массив принятых данных
uint8_t command;        // код команды на выполнение

char    txCmd;          // команда, передаваемая в пакете
uint8_t txNbt;          // количество байт данных в пакете
char    txDat[frame];   // массив данных для передачи


void MWake::wakeInit( uint8_t addr, long time )
{
  #ifdef UART2
    //Serial2.begin(115200);            // это порт дрйвера
    Serial2.begin(230400);            // это порт дрйвера - май 2022
    Serial2.setTimeout( time );       // время тайм-аута для функции readBytes(), ms. 
  #endif

  rxAdd   = addr;                     // адрес на прием
  txAdd   = addr;                     // адрес на передачу
  rxSta   = wait_fend;                // ожидание пакета
  txSta   = send_idle;                // ничего пока не передаем
  command = cmd_nop;                  // нет команды на выполнение
}


// Вычисление контрольной суммы
void MWake::doCrc8(char b, char *crc)
{
  char i;
  for(i = 0; i < 8; b = b >> 1, i++)
    if((b ^ *crc) & 1) *crc = ((*crc ^ 0x18) >> 1) | 0x80;
    else *crc = (*crc >> 1) & ~0x80;
}


// Чение данных порта
void MWake::wakeRead()
{
  char error_flags = 0;               // чтение флагов ошибок UART - пока нет
  
  uint8_t dataByte;
  Serial2.readBytes( &dataByte, 1 );  // чтение одного байта с удалением из приемного буфера
  char pre = rxPre;                   // сохранение старого пре-байта

  if( error_flags )                   // если обнаружены ошибки при приеме байта
  {
    rxSta = wait_fend;                // ожидание нового пакета
    command = cmd_err;                // сообщаем об ошибке
    return;
  }

  if( dataByte == fend )              // если обнаружено начало фрейма,
  {
    rxPre = dataByte;                 // то сохранение пре-байта,
    rxCrc = crc_init;                 // инициализация CRC,
    rxSta = wait_addr;                // сброс указателя данных,
    doCrc8( dataByte, &rxCrc );       // обновление CRC,
    return;                           // выход
  }

  if( rxSta == wait_fend )            // -----> если ожидание FEND,
    return;                           // то выход

  rxPre = dataByte;                   // обновление пре-байта
  if( pre == fesc )                   // если пре-байт равен FESC,
  {
    if( dataByte == tfesc )           // а байт данных равен TFESC,
      dataByte = fesc;                // то заменить его на FESC
    else if( dataByte == tfend )      // если байт данных равен TFEND,
          dataByte = fend;           // то заменить его на FEND
        else
        {
          rxSta = wait_fend;         // для всех других значений байта данных,
          command = cmd_err;         // следующего за FESC, ошибка
          return;
        }
  }
  else
  {
    if( dataByte == fesc )            // если байт данных равен FESC, он просто
      return;                         // запоминается в пре-байте
  }

  switch( rxSta )
  {
    case wait_addr :                   // -----> ожидание приема адреса
    {
      if( dataByte & 0x80 )           // если dataByte.7 = 1, то это адрес
      {
        dataByte = dataByte & 0x7F;   //обнуляем бит 7, получаем истинный адрес
        if( !dataByte || dataByte == rxAdd )   // если нулевой или верный адрес,
        {
          doCrc8( dataByte, &rxCrc ); // то обновление CRC и
          rxSta = wait_cmd;           // переходим к приему команды
          break;
        }
        rxSta = wait_fend;            // адрес не совпал, ожидание нового пакета
        break;
      }                               // если dataByte.7 = 0, то
      rxSta = wait_cmd;               // сразу переходим к приему команды
    }
    case wait_cmd :                    // -----> ожидание приема команды
    {
      if( dataByte & 0x80 )           // проверка бита 7 данных
      {
        rxSta = wait_fend;            // если бит 7 не равен нулю,
        command = cmd_err;            // то ошибка
        break;
      }
      rxCmd = dataByte;               // сохранение команды
      doCrc8( dataByte, &rxCrc );     // обновление CRC
      rxSta = wait_nbt;               // переходим к приему количества байт
      break;
    }
    case wait_nbt :                    // -----> ожидание приема количества байт
    {
      if( dataByte > frame )          // если количество байт > FRAME,
      {
        rxSta = wait_fend;
        command = cmd_err;            // то ошибка
        break;
      }
      rxNbt = dataByte;
      doCrc8( dataByte, &rxCrc );     // обновление CRC
      rxPtr = 0;                      // обнуляем указатель данных
      rxSta = wait_data;              // переходим к приему данных
      break;
    }
    case wait_data :                   // -----> ожидание приема данных
    {
      if( rxPtr < rxNbt )             // если не все данные приняты,
      {
        rxDat[rxPtr++] = dataByte;    // то сохранение байта данных,
        doCrc8( dataByte, &rxCrc );   // обновление CRC
        break;
      }

      if(dataByte != rxCrc)           // если приняты все данные, то проверка CRC
      {
        rxSta = wait_fend;            // если CRC не совпадает,
        command = cmd_err;            // то ошибка
        break;
      }
      rxSta = wait_fend;              // прием пакета завершен,
      command = rxCmd;                // загрузка команды на выполнение
      break;
    }
  }
}


// Передача пакета
void MWake::wakeWrite()
{
  char dataByte;

  if( txPre == fend )                     // если производится стаффинг,
  {
    dataByte = tfend;                     // передача TFEND вместо FEND
    txPre = dataByte;
    Serial2.write( dataByte );            // dataByte -> UART
    return;
  }

  if( txPre == fesc )                     // если производится стаффинг,
  {
    dataByte = tfesc;                     // передача TFESC вместо FESC
    txPre = dataByte;
    Serial2.write( dataByte );            // dataByte -> UART
    return;
  }

  switch( txSta )
  {
    case send_addr :                       // -----> передача адреса
    {
      if( txAdd )                         // если адрес не равен нулю, передаем его
      {
        dataByte = txAdd;
        doCrc8( dataByte, &txCrc );       // вычисление CRC для истинного адреса
        dataByte |= 0x80;                 // установка бита 7 для передачи адреса
        txSta = send_cmd;
        break;
      }                                   // иначе сразу передаем команду
    }
    case send_cmd :                        // -----> передача команды
    {
      dataByte = txCmd & 0x7F;
      txSta = send_nbt;
      break;
    }
    case send_nbt :                        // -----> передача количества байт
    {
      dataByte = txNbt;
      txSta = send_data;
      txPtr = 0;                          // обнуление указателя данных для передачи
      break;
    }
    case send_data :                       // -----> передача данных
    {
      if(txPtr < txNbt)
      {
        dataByte = txDat[ txPtr++ ]; 

//SerialUSB.print(" ->0x");   SerialUSB.print( dataByte, HEX );           // dataByte -> USB
      }
      else
      {
        dataByte = txCrc;                 // передача CRC
        txSta = send_crc;
      }
      break;
    }
    default:
    {
      txSta = send_idle;                  // передача пакета завершена
      return;
    }
  }

  if(txSta != send_cmd)                   // если не передача адреса, то
  {
    doCrc8( dataByte, &txCrc );           // вычисление CRC
  }

  txPre = dataByte;                       // сохранение пре-байта
  
  if( dataByte == fend || dataByte == fesc )
  {
    dataByte = fesc;                      // передача FESC, если нужен стаффинг
  }

  Serial2.write( dataByte );
}

// Инициализация передачи пакета
void MWake::wakeStartWrite()
{
  char dataByte = fend;
  int countByte = 0;
  txCrc = crc_init;                       // инициализация CRC,
  doCrc8( dataByte, &txCrc );             // обновление CRC
  txSta = send_addr;
  txPre = tfend;
  Serial2.write( dataByte );              // dataByte -> UART
    
  do
  {
    wakeWrite();                          // all bytes -> UART
    countByte++;
  }
  while( txSta != send_idle || countByte < 518 );

  if( countByte >= 518 )
  {
    txSta = send_idle;
  }
}

// передача ответа на команду 
void MWake::txReplay(char n, char err)
{
  txNbt = n;                      // количество байт
  txDat[0] = err;                 // код ошибки
  txCmd = command;                // команда
  wakeStartWrite();               // инициализация передачи
  command = cmd_nop;              // команда обработана
}

void MWake::configReply(char n, char err, uint8_t comm)
{
  txNbt = n;                      // количество байт
  txDat[0] = err;                 // код ошибки
  txCmd = comm;                   // команда
  wakeStartWrite();               // инициализация передачи
  command = cmd_nop;              // команда обработана    
}

void MWake::configAsk(char n, uint8_t comm)
{
  txNbt = n;                      // количество байт
  txCmd = comm;                   // команда
  wakeStartWrite();               // инициализация передачи
  command = cmd_nop;              // команда обработана    
}

  // работа с буфером приема
uint8_t MWake::getCommand() { return command; }

uint8_t  MWake::get08(int i) { return rxDat[i]; }

uint16_t MWake::get16(int i)
{
  uint16_t par  = rxDat[i] << 8;
  return(  par |= rxDat[i+1]); 
}

float MWake::getF16(int i)
{
  uint16_t par  = (rxDat[i] << 8) & 0xff00;
  par |= rxDat[i+1];
  return (float)(par / 256); 
}

uint8_t MWake::getNbt() {return rxNbt;}



// работа с буфером передачи
void MWake::setU8(int id, uint8_t value)
{
  txDat[id] = value;   //uint8_t(( val       ) & 0xff);
}

// ======== Запись в буфер передатчика ========
// Ответить одним байтом
int MWake::replyU08(int id, uint8_t value)
{
  txDat[id] = value & 0xFF;
  return id+1;
}

// Ответить двухбайтовым по индексу
int MWake::replyU16(int id, uint16_t value)
{
  txDat[id]    = ( value >> 8) & 0xFF; // Hi
  txDat[id+1]  =   value & 0xFF;       // Lo
  return id+2;
}

// Ответить четырехбайтовым по индексу
int MWake::replyU32(int id, uint32_t value)
{
  txDat[id]    = ( value >> 24) & 0xFF; // Hi
  txDat[id]    = ( value >> 16) & 0xFF; // Lo
  txDat[id]    = ( value >>  8) & 0xFF; // Hi
  txDat[id+1]  =   value        & 0xFF; // Lo
  return id+4;
}




void MWake::testReply( int rxNbt )
{
  for( int i = 0; i < rxNbt ; i++ )
  txDat[i] = rxDat[i];
  txReplay( rxNbt, txDat[0] );
}
