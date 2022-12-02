/*
  Сonnection manager
*/

#ifndef _MCONNMNG_H_
#define _MCONNMNG_H_

class MTools;
class MBoard;
class MDisplay;
class MState;

class MConnect 
{
  public:
    MConnect(MTools * tools);

    void run();
    void delegateWork();

  private:
    MTools * Tools;
    MBoard * Board;
    MDisplay * Display;
    MState * State = 0;
};

#endif  //!_MCONNMNG_H_
