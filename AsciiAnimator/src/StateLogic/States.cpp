#include "States.h"
#include <ncurses.h>

void StartAppState::onEnter() {
  mainMenu = loadPrintable("MainMenu", true, 0, false, true);
  newAnimation = loadButton("newAnimation", true, 1, false, true,
                            [this]() { this->newAnimationFunction(); });
  loadAnimation = loadButton("loadAnimation", true, 1, false, true,
                             [this]() { this->loadAnimationFunction(); });
  quit = loadButton("quit", true, 1, false, true,
                    [this]() { this->quitFunction(); });

  SCREEN_LENGTH = 120;
  currentCamera = std::make_unique<Camera>(SCREEN_LENGTH, SCREEN_HEIGHT);
  playerEntity = nullptr;

  newAnimation.displace(58, 11);
  loadAnimation.displace(58, 13);
  quit.displace(58, 15);
};

void StartAppState::update() {
  if (userInput == KEY_MOUSE) {
    if (getmouse(&event) == OK) {
      if (event.bstate & BUTTON1_PRESSED) {
        int mouseX = event.x;
        int mouseY = event.y;

        if (newAnimation.mouseInBounds(mouseX, mouseY)) {
          newAnimation.executeFunction();
        } else if (loadAnimation.mouseInBounds(mouseX, mouseY)) {
          loadAnimation.executeFunction();
        } else if (quit.mouseInBounds(mouseX, mouseY)) {
          quit.executeFunction();
        }
      }
    }
  }
}

void StartAppState::onExit() {
  clear();
  allPrintables.clear();
}

GameState *StartAppState::getNextState() {
  switch (nextState) {
  case States::None:
    return nullptr;
    break;
  case States::MainMenu:
    return nullptr;
    break;
  case States::Drawing:
    return new MainAppState();
    break;
  case States::Quit:
    return new QuitState();
    break;
  default:
    return nullptr;
    break;
  }
}

void StartAppState::newAnimationFunction() { nextState = States::Drawing; }
void StartAppState::loadAnimationFunction() { nextState = States::Drawing; }
void StartAppState::quitFunction() { nextState = States::Quit; }

void MainAppState::onEnter() {
  counterEntity = loadPrintable("counter", true, 2, true, false);
  stickEntity = loadPrintable("player", true, 1, true, false);
  cameraOutline = loadPrintable("camera", true, 100, false, true);

  currentCamera = std::make_unique<Camera>(SCREEN_LENGTH, SCREEN_HEIGHT);
  playerEntity = stickEntity;

  //   Initial Displace
  int midX = (SCREEN_LENGTH / 2);
  int midY = (SCREEN_HEIGHT / 2);
  counterEntity->displace(midX, midY);
};

void MainAppState::update() {
  // Mouse Handling
  // --------------
  if (userInput == KEY_MOUSE) {
    if (getmouse(&event) == OK) {

      // Camera Drag
      if (event.bstate & BUTTON2_PRESSED) {
        cameraDrag = true;
        lastMouseX = event.x;
        lastMouseY = event.y;
      }
      if (event.bstate & BUTTON1_PRESSED) {
        drawing = true;
      }

      if (event.bstate & BUTTON2_RELEASED) {
        cameraDrag = false;
        lastMouseX = -1;
        lastMouseY = -1;
      }
      if (event.bstate & BUTTON1_RELEASED) {
        drawing = false;
      }

      if (event.bstate & REPORT_MOUSE_POSITION) {
        if (cameraDrag) {
          // Calculate how much the mouse moved
          int dx = lastMouseX - event.x;
          int dy = lastMouseY - event.y;

          // Pan the camera accordingly
          currentCamera->displaceViewPort(-dx, -dy);

          // Update the last known position
          lastMouseX = event.x;
          lastMouseY = event.y;
        } else if (drawing) {
        }
      }
    }
  }

  // Player Movement Test
  if (userInput == 'w') {
    playerEntity->displace(0, -2);
  } else if (userInput == 'a') {
    playerEntity->displace(-2, 0);
  } else if (userInput == 's') {
    playerEntity->displace(0, 2);
  } else if (userInput == 'd') {
    playerEntity->displace(2, 0);
  }
}

void MainAppState::onExit() {}

GameState *MainAppState::getNextState() { return nullptr; }

void QuitState::onEnter() { engineRunning = false; }

void QuitState::update() {}

void QuitState::onExit() {}