// #ifndef _MTOUCH_H_
// #define _MTOUCH_H_

// /*
//  * Емкостная клавиатура, опрос
//  * Вариант  202105
//  * pcb: eltr_v2.2
//  */

// #include "state/mstate.h"

// class MTools;
// class MBoard;
// class MState;
// class MKeyboard;

// class MTouch
// {
//   public:
//     MTouch(MTools * tools);
//     ~MTouch();

//     void doTouch();
//     //void delegateWork();

//   private:
//     MTools    * Tools    = nullptr;
//     MBoard    * Board    = nullptr;
//     MState    * State    = nullptr;
//     MKeyboard * Keyboard = nullptr;   
 
// };

// namespace MTouchStates
// {
//     // Пороговые значения - уточняются при первом включении разработанной платы
//   static constexpr short treshold_up = 200;
//   static constexpr short treshold_dn = 200;
//   static constexpr short treshold_p  = 200;
//   static constexpr short treshold_b  = 200;
//   static constexpr short treshold_c  = 200;

//   class MButtonUp : public MState
//   {
//     public:   
//       MButtonUp(MTools * Tools);
//       MState * fsm() override;
//   };
  
//   class MButtonDn : public MState
//   {
//     public:   
//       MButtonDn(MTools * Tools);
//       MState * fsm() override;
//   };
    
//   class MButtonP : public MState
//   {
//     public:   
//       MButtonP(MTools * Tools);
//       MState * fsm() override;
//   };

//   class MButtonB : public MState
//   {
//     public:   
//       MButtonB(MTools * Tools);
//       MState * fsm() override;
//   };

//   class MButtonC : public MState
//   {
//     public:   
//       MButtonC(MTools * Tools);
//       MState * fsm() override;
//   };

// };

// #endif  //!_MTOUCH_H_
