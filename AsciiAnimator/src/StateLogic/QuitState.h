#ifndef QUITSTATE_H
#define QUITSTATE_H

#include "../AppParameters/AppParameters.h"
#include "AppState.h"

class QuitState : public GameState {
public:
  void onEnter() override;
  void update() override;
  void onExit() override;
};

#endif