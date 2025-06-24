#ifndef APPSTATE_H
#define APPSTATE_H

#include "../../../GameEngine/include/GameEngine.h"
#include "../AppParameters/AppParameters.h"

class AppState : public GameState {
private:
  MEVENT event;
  bool cameraDrag = false;
  bool drawing = true;
  int lastMouseX = -1;
  int lastMouseY = -1;

  std::shared_ptr<Entity> userEntity;
  Animation drawingAnimation;
  Frame currentFrame;

  std::shared_ptr<Button> currentCharacterButton;
  std::shared_ptr<Button> eraserSelectButton;
  std::shared_ptr<Button> frameLengthButton;
  std::shared_ptr<Button> nextFrameButton;
  std::shared_ptr<Button> playAnimationButton;
  std::shared_ptr<Button> previousFrameButton;
  std::shared_ptr<Button> quitButton;

  void currentCharacterButtonFunc();
  void eraserSelectButtonFunc();
  void frameLengthButtonFunc();
  void newFrameButtonFunc();
  void playAnimationButtonFunc();
  void previousFrameButtonFunc();
  void quitButtonFunc();

  States nextState = States::None;

public:
  void onEnter() override;
  void update() override;
  void onExit() override;

  GameState *getNextState() override;
};

#endif