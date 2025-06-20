#ifndef STARTAPPSTATE_H
#define STARTAPPSTATE_H

#include "../../../GameEngine/include/GameEngine.h"

enum class States { None, MainMenu, Drawing, Quit };

class MainAppState : public GameState {
private:
  std::shared_ptr<Entity> counterEntity;
  std::shared_ptr<Entity> stickEntity;
  std::shared_ptr<Entity> cameraOutline;

  MEVENT event;
  bool cameraDrag = false;
  bool drawing = true;
  int lastMouseX = -1;
  int lastMouseY = -1;

public:
  void onEnter() override;
  void update() override;
  void onExit() override;

  GameState *getNextState() override;
};

class StartAppState : public GameState {
private:
  std::shared_ptr<Entity> mainMenu;
  Button newAnimation;
  Button loadAnimation;
  Button quit;

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

class QuitState : public GameState {
public:
  void onEnter() override;
  void update() override;
  void onExit() override;
};

#endif