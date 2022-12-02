#ifndef _MSTATE_H_
#define _MSTATE_H_

class MTools;
class MBoard;
class MDisplay;
class MKeyboard;

class MState
{
  public:
    MState(MTools * Tools);
    virtual ~MState(){}
    virtual MState * fsm() = 0;

  protected:
    MTools    * Tools    = nullptr;
    MBoard    * Board    = nullptr;
    MDisplay  * Display  = nullptr;
    MKeyboard * Keyboard = nullptr;
};

#endif // !_MSTATE_H_
