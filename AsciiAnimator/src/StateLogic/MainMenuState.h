#ifndef MAINMENUSTATE_H
#define MAINMENUSTATE_H

#include "../AppParameters/AppParameters.h"
#include "AppState.h"

class StartAppState : public GameState {
private:
  std::shared_ptr<UIElement> mainMenu;
  std::shared_ptr<Button> newAnimation;
  std::shared_ptr<Button> loadAnimation;
  std::shared_ptr<Button> quit;

  MEVENT event;

  void newAnimationFunction();
  void loadAnimationFunction();
  void quitFunction();

  States nextState = States::None;

public:
  void onEnter() override;
  void update() override;
  void onExit() override;

  GameState *getNextState() override;
};

#endif