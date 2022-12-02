// /*
//  * Конечный автомат обработки данных емкостной клавиатуры
//  * Бесконечно повторяющийся цикл.
//  * Вариант  202105 - не совместим с Espressif 32 4.x
//  * pcb: eltr_v2.2
//  */

// #include "mtools.h"
// #include "board/mboard.h"
// #include "board/mpins.h"
// #include "board/mtouch.h"
// #include "board/mkeyboard.h"
// #include "state/mstate.h"
// #include <Arduino.h>


// MTouch::MTouch(MTools * tools) : Tools(tools), Board(tools->Board)
// {
//   State    = new MTouchStates::MButtonUp(Tools);
//   Keyboard = new MKeyboard();
// }

// void MTouch::doTouch()
// {
//   MState * newState = State->fsm();      
//   if (newState != State)                      //state changed!
//   {
//     delete State;

//   Board->buzzerOff();

//     State = newState;
//   } 
// }

// namespace MTouchStates
// {
//   short touchValue;

//   // Состояние "Опрос кнопки UP"
//   MButtonUp::MButtonUp(MTools * Tools) : MState(Tools) {}
//   MState * MButtonUp::fsm()
//   {
//     touch_pad_init(); 
//     touchValue = touchRead(MPins::touch_up_pin);
//     if( touchValue < treshold_up)
//     {
//       Keyboard->calcKeys( MKeyboard::adc_up );
//       #ifdef DEBUD_TOUCH_UP
//         Serial.print("UP=");  Serial.println(touchValue);
//         Board->ledsGreen();
//       #endif
//       return this;
//     }
//     else
//     {
//       Keyboard->calcKeys( MKeyboard::adc_no );
//       #ifdef DEBUD_TOUCH_UP
//         Board->ledsOff();
//       #endif
//       return new MButtonDn(Tools);
//     }
//   };

//   // Состояние "Опрос кнопки DN"
//   MButtonDn::MButtonDn(MTools * Tools) : MState(Tools) {}
//   MState * MButtonDn::fsm()
//   {
//     touch_pad_init();
//     touchValue = touchRead(MPins::touch_dn_pin);
//     if( touchValue < treshold_dn)
//     {
//       Keyboard->calcKeys( MKeyboard::adc_dn );
//       #ifdef DEBUD_TOUCH_DN
//         Serial.print("DN=");  Serial.println(touchValue);
//         Board->ledsGreen();
//       #endif
//       return this;
//     }
//     else
//     {
//       Keyboard->calcKeys( MKeyboard::adc_no );
//       #ifdef DEBUD_TOUCH_DN
//         Board->ledsOff();
//       #endif
//       return new MButtonP(Tools);
//     }
//   };

//   // Состояние "Опрос кнопки P"
//   MButtonP::MButtonP(MTools * Tools) : MState(Tools) {}
//   MState * MButtonP::fsm()
//   {
//     touch_pad_init();
//     touchValue = touchRead(MPins::touch_p_pin);
//     if( touchValue < treshold_p)
//     {
//       Keyboard->calcKeys( MKeyboard::adc_p );
//       #ifdef DEBUD_TOUCH_P
//         Serial.print("P=");  Serial.println(touchValue);
//         Board->ledsGreen();
//       #endif
//       return this;
//     }
//     else
//     {
//       Keyboard->calcKeys( MKeyboard::adc_no );
//       #ifdef DEBUD_TOUCH_P
//         Board->ledsOff();
//       #endif
//       return new MButtonB(Tools);
//     }
//   };

//   // Состояние "Опрос кнопки B"
//   MButtonB::MButtonB(MTools * Tools) : MState(Tools) {}
//   MState * MButtonB::fsm()
//   {
//     touch_pad_init();
//     touchValue = touchRead(MPins::touch_b_pin);
//     if( touchValue < treshold_b)
//     {
//       Keyboard->calcKeys( MKeyboard::adc_b );
//       #ifdef DEBUD_TOUCH_B
//         Serial.print("B=");  Serial.println(touchValue);
//         Board->ledsGreen();
//       #endif
//       return this;
//     }
//     else
//     {
//       Keyboard->calcKeys( MKeyboard::adc_no );
//       #ifdef DEBUD_TOUCH_B
//         Board->ledsOff();
//       #endif
//       return new MButtonC(Tools);
//     }
//   };

//   // Состояние "Опрос кнопки C"
//   MButtonC::MButtonC(MTools * Tools) : MState(Tools) {}
//   MState * MButtonC::fsm()
//   {
//     touch_pad_init();
//     touchValue = touchRead(MPins::touch_c_pin);
//     if( touchValue < treshold_c)
//     {
//       Keyboard->calcKeys( MKeyboard::adc_c );
//       #ifdef DEBUD_TOUCH_C
//         Serial.print("C=");  Serial.println(touchValue);
//         Board->ledsGreen();
//       #endif
//       return this;
//     }
//     else
//     {
//       Keyboard->calcKeys( MKeyboard::adc_no );
//       #ifdef DEBUD_TOUCH_C
//         Board->ledsOff();
// //  Board->buzzerOff();

//       #endif
//       return new MButtonUp(Tools);
//     }
//   };

// };
