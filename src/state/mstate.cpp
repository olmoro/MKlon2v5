#include "state/mstate.h"
#include "mtools.h"

MState::MState(MTools * Tools) :
  Tools(Tools),
  Board(Tools->Board),
  Display(Tools->Display),
  Keyboard(Tools->Keyboard) {}
