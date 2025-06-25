#include "AppState.h"
#include "QuitState.h"
#include <ncurses.h>

void AppState::onEnter() {
  currentCamera = std::make_unique<Camera>(SCREEN_LENGTH, SCREEN_HEIGHT);

  currentFrame = Frame(Sprite(), 10);
  std::vector<Frame> allFrames;
  allFrames.push_back(currentFrame);
  drawingAnimation = Animation("newAnimation", allFrames, true);
  drawingAnimation.setPlaying(false);
  std::vector<Animation> allAnimations;
  allAnimations.push_back(drawingAnimation);
  userEntity = std::make_unique<Entity>(
      Entity("newEntity", allAnimations, true, 0, true));
  allPrintables.push_back(userEntity);
  playerEntity = userEntity;

  selectNewCharacter = false;
  drawingCharacter = 'x';

  currentCharacterButton =
      loadButton("currentCharacterButton", true, 100, false, true,
                 [this]() { this->currentCharacterButtonFunc(); });
  eraserSelectButton = loadButton("eraserSelectButton", true, 100, false, true,
                                  [this]() { this->eraserSelectButtonFunc(); });
  frameLengthButton = loadButton("frameLengthButton", true, 100, false, true,
                                 [this]() { this->frameLengthButtonFunc(); });
  nextFrameButton = loadButton("nextFrameButton", true, 100, false, true,
                               [this]() { this->nextFrameButtonFunc(); });
  playAnimationButton =
      loadButton("playAnimationButton", true, 100, false, true,
                 [this]() { this->playAnimationButtonFunc(); });
  previousFrameButton =
      loadButton("previousFrameButton", true, 100, false, true,
                 [this]() { this->previousFrameButtonFunc(); });
  quitButton = loadButton("quitButton", true, 100, false, true,
                          [this]() { this->quitButtonFunc(); });

  currentCharacterButton->setDynamicPosition(
      ScreenLockPosition::TOP_RIGHT_CORNER);
  eraserSelectButton->setDynamicPosition(ScreenLockPosition::TOP_RIGHT_CORNER);
  frameLengthButton->setDynamicPosition(ScreenLockPosition::TOP_RIGHT_CORNER);

  previousFrameButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE,
                                          StackDirection::HORIZONTAL);
  nextFrameButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE,
                                      StackDirection::HORIZONTAL);
  playAnimationButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE,
                                          StackDirection::HORIZONTAL);

  quitButton->setDynamicPosition(ScreenLockPosition::TOP_LEFT_CORNER);
  UIElement::updateAllLockedPositions();
};

void AppState::update() {
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

        int mouseX = event.x;
        int mouseY = event.y;

        if (currentCharacterButton->mouseInBounds(mouseX, mouseY)) {
          currentCharacterButton->executeFunction();
        } else if (eraserSelectButton->mouseInBounds(mouseX, mouseY)) {
          eraserSelectButton->executeFunction();
        } else if (frameLengthButton->mouseInBounds(mouseX, mouseY)) {
          frameLengthButton->executeFunction();
        } else if (nextFrameButton->mouseInBounds(mouseX, mouseY)) {
          nextFrameButton->executeFunction();
        } else if (previousFrameButton->mouseInBounds(mouseX, mouseY)) {
          previousFrameButton->executeFunction();
        } else if (playAnimationButton->mouseInBounds(mouseX, mouseY)) {
          playAnimationButton->executeFunction();
        } else if (quitButton->mouseInBounds(mouseX, mouseY)) {
          quitButton->executeFunction();
        }
      }

      if (event.bstate & BUTTON2_RELEASED) {
        cameraDrag = false;
        lastMouseX = -1;
        lastMouseY = -1;
      }
      if (event.bstate & BUTTON1_RELEASED) {
        drawing = false;
      }

      if (event.bstate & (BUTTON1_RELEASED | BUTTON1_CLICKED)) {
        drawing = false;
      }
      if (event.bstate & (BUTTON2_RELEASED | BUTTON2_CLICKED)) {
        cameraDrag = false;
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
          int worldX = event.x - currentCamera->getLengthOffset();
          int worldY = event.y - currentCamera->getHeightOffset();
          Pixel newPixel = Pixel(Position(worldX, worldY), drawingCharacter);
          userEntity->getCurrentAnimation().addPixelToCurrentFrame(newPixel);
        }
      }
    }
  } else if (selectNewCharacter) {
    drawingCharacter = userInput;
  }
}

void AppState::onExit() {}

GameState *AppState::getNextState() {
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

void AppState::currentCharacterButtonFunc() { selectNewCharacter = true; }
void AppState::eraserSelectButtonFunc() { drawingCharacter = ' '; }
void AppState::frameLengthButtonFunc() {}
void AppState::nextFrameButtonFunc() {
  userEntity->getCurrentAnimation().manuallyIncrementFrame();
}
void AppState::playAnimationButtonFunc() {
  if (!userEntity->getCurrentAnimation().isPlaying()) {
    userEntity->getCurrentAnimation().setPlaying(true);
  } else {
    userEntity->getCurrentAnimation().setPlaying(false);
  }
}
void AppState::previousFrameButtonFunc() {
  userEntity->getCurrentAnimation().manuallyDecrementFrame();
}
void AppState::quitButtonFunc() { nextState = States::Quit; }