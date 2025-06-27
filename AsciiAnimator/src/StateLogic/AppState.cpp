//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file AppState.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Main state of the animator
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AppState.h"
#include "QuitState.h"
#include <ncurses.h>

// public ----------------------------------------------------------------------------------------------------
void AppState::onEnter()
{
   currentCamera = std::make_unique<Camera>(SCREEN_LENGTH, SCREEN_HEIGHT);

   currentFrame = Frame(Sprite(), 10);
   std::vector<Frame> allFrames;
   allFrames.push_back(currentFrame);
   drawingAnimation = Animation("newAnimation", allFrames, true);
   drawingAnimation.setPlaying(false);
   std::vector<Animation> allAnimations;
   allAnimations.push_back(drawingAnimation);

   userEntity = std::make_unique<Entity>(Entity("newEntity", allAnimations, true, true));
   allPrintables.push_back(userEntity);
   playerEntity = userEntity;

   selectNewCharacter = false;
   drawingCharacter   = 'x';

   currentCharacterButton = PrintableFactory::loadButton("currentCharacterButton", true, false,
                                                         [this]() { this->currentCharacterButtonFunc(); });
   currentCharacterButton->setAllAnimationSpriteLayers(100);
   eraserSelectButton = PrintableFactory::loadButton("eraserSelectButton", true, false,
                                                     [this]() { this->eraserSelectButtonFunc(); });
   eraserSelectButton->setAllAnimationSpriteLayers(100);
   frameLengthButton = PrintableFactory::loadButton("frameLengthButton", true, false,
                                                    [this]() { this->frameLengthButtonFunc(); });
   frameLengthButton->setAllAnimationSpriteLayers(100);
   nextFrameButton = PrintableFactory::loadButton("nextFrameButton", true, false,
                                                  [this]() { this->nextFrameButtonFunc(); });
   nextFrameButton->setAllAnimationSpriteLayers(100);
   playAnimationButton = PrintableFactory::loadButton("playAnimationButton", true, false,
                                                      [this]() { this->playAnimationButtonFunc(); });
   playAnimationButton->setAllAnimationSpriteLayers(100);
   previousFrameButton = PrintableFactory::loadButton("previousFrameButton", true, false,
                                                      [this]() { this->previousFrameButtonFunc(); });
   previousFrameButton->setAllAnimationSpriteLayers(100);
   quitButton = PrintableFactory::loadButton("quitButton", true, false, [this]() { this->quitButtonFunc(); });
   quitButton->setAllAnimationSpriteLayers(100);

   currentCharacterButton->setDynamicPosition(ScreenLockPosition::TOP_RIGHT_CORNER);
   eraserSelectButton->setDynamicPosition(ScreenLockPosition::TOP_RIGHT_CORNER);
   frameLengthButton->setDynamicPosition(ScreenLockPosition::TOP_RIGHT_CORNER);

   previousFrameButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE, StackDirection::HORIZONTAL);
   nextFrameButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE, StackDirection::HORIZONTAL);
   playAnimationButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE, StackDirection::HORIZONTAL);

   quitButton->setDynamicPosition(ScreenLockPosition::TOP_LEFT_CORNER);
   UIElement::updateAllLockedPositions();
};

// public ----------------------------------------------------------------------------------------------------
void AppState::update()
{
   // Mouse Handling
   // --------------
   if (userInput == KEY_MOUSE)
   {
      if (getmouse(&event) == OK)
      {
         // Camera Drag
         if (event.bstate & BUTTON2_PRESSED)
         {
            cameraDrag = true;
            lastMousePosition.setX(event.x);
            lastMousePosition.setY(event.y);
         }
         if (event.bstate & BUTTON1_PRESSED)
         {
            drawing                = true;
            Position mousePosition = Position(event.x, event.y);

            if (currentCharacterButton->mouseInBounds(mousePosition))
            {
               currentCharacterButton->executeFunction();
            }
            else if (eraserSelectButton->mouseInBounds(mousePosition))
            {
               eraserSelectButton->executeFunction();
            }
            else if (frameLengthButton->mouseInBounds(mousePosition))
            {
               frameLengthButton->executeFunction();
            }
            else if (nextFrameButton->mouseInBounds(mousePosition))
            {
               nextFrameButton->executeFunction();
            }
            else if (previousFrameButton->mouseInBounds(mousePosition))
            {
               previousFrameButton->executeFunction();
            }
            else if (playAnimationButton->mouseInBounds(mousePosition))
            {
               playAnimationButton->executeFunction();
            }
            else if (quitButton->mouseInBounds(mousePosition))
            {
               quitButton->executeFunction();
            }
         }

         if (event.bstate & BUTTON2_RELEASED)
         {
            cameraDrag = false;
            lastMousePosition.setX(-1);
            lastMousePosition.setY(-1);
         }
         if (event.bstate & BUTTON1_RELEASED)
         {
            drawing = false;
         }

         if (event.bstate & (BUTTON1_RELEASED | BUTTON1_CLICKED))
         {
            drawing = false;
         }
         if (event.bstate & (BUTTON2_RELEASED | BUTTON2_CLICKED))
         {
            cameraDrag = false;
         }

         if (event.bstate & REPORT_MOUSE_POSITION)
         {
            if (cameraDrag)
            {
               // Calculate how much the mouse moved
               int dx = lastMousePosition.getX() - event.x;
               int dy = lastMousePosition.getY() - event.y;

               // Pan the camera accordingly
               currentCamera->displaceViewPort(-dx, -dy);

               // Update the last known position
               lastMousePosition.setX(event.x);
               lastMousePosition.setY(event.y);
            }
            else if (drawing)
            {
               int   worldX   = event.x - currentCamera->getLengthOffset();
               int   worldY   = event.y - currentCamera->getHeightOffset();
               Pixel newPixel = Pixel(Position(worldX, worldY), drawingCharacter);
               userEntity->getCurrentAnimationMutable().addPixelToCurrentFrame(newPixel);
            }
         }
      }
   }
   else if (selectNewCharacter)
   {
      drawingCharacter = userInput;
   }
}

// public ----------------------------------------------------------------------------------------------------
void AppState::onExit() {}

// public ----------------------------------------------------------------------------------------------------
GameState* AppState::getNextState()
{
   switch (nextState)
   {
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

// public ----------------------------------------------------------------------------------------------------
void AppState::currentCharacterButtonFunc()
{
   selectNewCharacter = true;
}

// public ----------------------------------------------------------------------------------------------------
void AppState::eraserSelectButtonFunc()
{
   drawingCharacter = ' ';
}

// public ----------------------------------------------------------------------------------------------------
void AppState::frameLengthButtonFunc() {}

// public ----------------------------------------------------------------------------------------------------
void AppState::nextFrameButtonFunc()
{
   userEntity->getCurrentAnimationMutable().manuallyIncrementFrame();
}

// public ----------------------------------------------------------------------------------------------------
void AppState::playAnimationButtonFunc()
{
   if (!userEntity->getCurrentAnimation().isPlaying())
   {
      userEntity->getCurrentAnimationMutable().setPlaying(true);
   }
   else
   {
      userEntity->getCurrentAnimationMutable().setPlaying(false);
   }
}

// public ----------------------------------------------------------------------------------------------------
void AppState::previousFrameButtonFunc()
{
   userEntity->getCurrentAnimationMutable().manuallyDecrementFrame();
}

// public ----------------------------------------------------------------------------------------------------
void AppState::quitButtonFunc()
{
   nextState = States::Quit;
}