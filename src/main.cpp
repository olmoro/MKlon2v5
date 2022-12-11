/*
  project:      MESP32v7 -> Mklon -> MKlon2
  pcb:          mesp_v7.1         -> mklon2v5
  pcb:          mcdm.v7.3         -> mklon2v5
  display:      1.8 дюймовый TFT ЖК-дисплей 128*160 полноцветный экран IPS. Driver IC: ST7735
  driver:       ATSAMD21 M0 MINI
  date:         2022 ноябрь
  VS:           1.73.1            -> 1.74.0
  //Espressif 32: 3.5.0 (с 4.0 не совместимо)
  Espressif 32: 5.2.0 - проверить
*/

#include "board/mboard.h"
#include "board/msupervisor.h"
#include "driver/mcommands.h"
#include "display/mdisplay.h"
#include "mtools.h"
#include "mdispatcher.h"
#include "mconnmng.h"
//#include "board/mmeasure.h"
#include "measure/mmeasure.h"
//#include "dallas/temp.h"
#include "connect/connectfsm.h"
#include <Arduino.h>
  #include "project_config.h"

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

static MBoard      * Board      = 0;
static MDisplay    * Display    = 0;
static MTools      * Tools      = 0;
static MCommands   * Commands   = 0;
static MMeasure    * Measure    = 0;
static MDispatcher * Dispatcher = 0;
static MConnect    * Connect    = 0;

void connectTask ( void * );
void displayTask ( void * );
void coolTask    ( void * );
void mainTask    ( void * );
void measureTask ( void * );
void driverTask  ( void * );
//void tempTask    ( void * );


// setup() выполняется до запуска RTOS, а потому без обязательных требований
//  по максимальному времени монопольного захвата ядра (13мс).
void setup()
{
  //initTemp();

  Display    = new MDisplay();
  Board      = new MBoard();
  Tools      = new MTools(Board, Display);
  Commands   = new MCommands(Tools);
  Measure    = new MMeasure(Tools);
  Dispatcher = new MDispatcher(Tools);
  Connect    = new MConnect(Tools);

  
  // Выделение ресурсов для каждой задачи: память, приоритет, ядро.
  // Все задачи исполняются ядром 1, ядро 0 выделено для радиочастотных задач - BT и WiFi.
  xTaskCreatePinnedToCore ( connectTask, "Connect", 10000, NULL, 1, NULL, 1 );
  xTaskCreatePinnedToCore ( mainTask,    "Main",    10000, NULL, 2, NULL, 1 );
  xTaskCreatePinnedToCore ( displayTask, "Display",  5000, NULL, 2, NULL, 1 );
  xTaskCreatePinnedToCore ( coolTask,    "Cool",     1000, NULL, 2, NULL, 1 );
  xTaskCreatePinnedToCore ( measureTask, "Measure",  5000, NULL, 2, NULL, 1 );
  xTaskCreatePinnedToCore ( driverTask,  "Driver",   5000, NULL, 3, NULL, 1 );
  //xTaskCreatePinnedToCore ( tempTask,    "Dallas",   5000, NULL, 1, NULL, 1 );

}

void loop() 
{
  if( Serial2.available() )    // В буфере приема есть принятые байты, не факт, что пакет полный
  {                            // Пока не принят весь пакет, время ожидания ограничено 5мс
    vTaskEnterCritical(&timerMux);
      Tools->setErr( Commands->dataProcessing() );
    vTaskExitCritical(&timerMux);
  }
}

// Задача подключения к WiFi сети (полностью заимствована как есть)
void connectTask( void * )
{
  while(true) {
  //unsigned long start = millis();   // Старт таймера 
  //Serial.print("*");
    Connect->run(); 
    // Период вызова задачи задается в TICK'ах, TICK по умолчанию равен 1мс.
    vTaskDelay( 10 / portTICK_PERIOD_MS );
  // Для удовлетворения любопытства о длительности выполнения задачи - раскомментировать как и таймер.
  //Serial.print("Autoconnect: Core "); Serial.print(xPortGetCoreID()); Serial.print(" Time = "); Serial.print(millis() - start); Serial.println(" mS");
  // Core 1, 2...3 mS
  }
  vTaskDelete( NULL );
}

// Задача выдачи данных на дисплей (Закомментированы автоматически передаваемые параметры)
void displayTask( void * )
{
  while(true)
  {
    Display->runDisplay(Board->getCelsius(), Tools->getAP());
    vTaskDelay( 250 / portTICK_PERIOD_MS );
  }
  vTaskDelete( NULL );
}

// Задача управления системой теплоотвода. Предполагается расширить функциональность, добавив 
// слежение за правильностью подключения нагрузки, масштабирование тока и т.д. 
void coolTask( void * )
{
  while (true)
  {
    //unsigned long start = millis();
    Board->Supervisor->runCool();
    //Serial.print("Cool: Core "); Serial.print(xPortGetCoreID()); Serial.print(" Time = "); Serial.print(millis() - start); Serial.println(" mS");
    // Core 1, 0 mS
    vTaskDelay( 200 / portTICK_PERIOD_MS );
  }
  vTaskDelete( NULL );
}

// 1. Задача обслуживает выбор режима работы.
// 2. Управляет конечным автоматом выбранного режима вплоть да выхода из режима.
// И то и другое построены как конечные автоматы (FSM).
void mainTask ( void * )
{ 
  while (true)
  {
    // Выдерживается период запуска для вычисления амперчасов. Если прочие задачи исполняются в порядке 
    // очереди, то эта точно по таймеру - через 0,1с.
    portTickType xLastWakeTime = xTaskGetTickCount();   // To count the amp hours
    //unsigned long start = millis();
    Dispatcher->run(); 
    //Serial.print("Main: Core "); Serial.print(xPortGetCoreID()); Serial.print(" Time = "); Serial.print(millis() - start); Serial.println(" mS");
    // Core 1, 100...102 mS
    vTaskDelayUntil( &xLastWakeTime, 100 / portTICK_PERIOD_MS );    // период 0,1с
  }
  vTaskDelete( NULL );
}

// Задача управления измерениями напряжения источника питания и датчика температуры, 
void measureTask( void * )
{
  while (true)
  {
    //unsigned long start = micros();
    Measure->run();
    //Serial.print(" Time, uS: "); Serial.println(micros() - start);
    // Core 1, 32 260 mkS
    //vTaskDelay( 10 / portTICK_PERIOD_MS );
    vTaskDelay( 100 / portTICK_PERIOD_MS ); //на время экспериментов задача вызывается в 10 раз реже
  }
  vTaskDelete(NULL);
}

void driverTask( void * )
{
  while (true)
  {
    //unsigned long start = micros();
    vTaskEnterCritical(&timerMux);
      Commands->doCommand();
    vTaskExitCritical(&timerMux);
    //Serial.print(" Time, uS: "); Serial.println(micros() - start);
    // 310µS max
    vTaskDelay( 75 / portTICK_PERIOD_MS );
    //vTaskDelay( 100 / portTICK_PERIOD_MS );
  }
  vTaskDelete(NULL);
}

// void tempTask( void * parameter )
// {
//   portTickType xLastWakeTime;                                     // ******
//   xLastWakeTime = xTaskGetTickCount();                            // ******
//   while(1)
//   {
//     workTemp();
//     vTaskDelayUntil( &xLastWakeTime, 1000 / portTICK_PERIOD_MS ); 
//   }
//   vTaskDelete( NULL );
// }
