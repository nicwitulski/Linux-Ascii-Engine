#include "MainMenuState.h"
#include "QuitState.h"
#include <memory>
#include <ncurses.h>

void StartAppState::onEnter() {
  mainMenu = loadUIElement("mainMenuSprite", true, 0, false, true);
  newAnimation = loadButton("newAnimationButton", true, 1, false, true,
                            [this]() { this->newAnimationFunction(); });
  loadAnimation = loadButton("loadAnimationButton", true, 1, false, true,
                             [this]() { this->loadAnimationFunction(); });
  quit = loadButton("quitButton", true, 1, false, true,
                    [this]() { this->quitFunction(); });

  SCREEN_LENGTH = 120;
  currentCamera = std::make_unique<Camera>(SCREEN_LENGTH, SCREEN_HEIGHT);
  playerEntity = nullptr;

  newAnimation->displace(58, 11);
  loadAnimation->displace(58, 13);
  quit->displace(58, 15);
};

void StartAppState::update() {
  if (userInput == KEY_MOUSE) {
    if (getmouse(&event) == OK) {
      if (event.bstate & BUTTON1_PRESSED) {
        int mouseX = event.x;
        int mouseY = event.y;

        if (newAnimation->mouseInBounds(mouseX, mouseY)) {
          newAnimation->executeFunction();
        } else if (loadAnimation->mouseInBounds(mouseX, mouseY)) {
          loadAnimation->executeFunction();
        } else if (quit->mouseInBounds(mouseX, mouseY)) {
          quit->executeFunction();
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
    return new AppState();
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