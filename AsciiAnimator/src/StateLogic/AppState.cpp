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
   currentCamera = std::make_shared<Camera>(SCREEN_LENGTH, SCREEN_HEIGHT);

   currentFrame = Frame(Sprite(), 10);
   std::vector<Frame> allFrames;
   allFrames.push_back(currentFrame);
   drawingAnimation = Animation("newAnimation", allFrames, true);
   drawingAnimation.setPlaying(false);
   std::vector<Animation> allAnimations;
   allAnimations.push_back(drawingAnimation);

   userEntity = std::make_unique<Entity>(Entity("newEntity", allAnimations, true, true));
   allPrintables.push_back(userEntity);
   printablesToSave.push_back(userEntity);
   playerEntity = userEntity;

   selectNewCharacter = false;
   drawingCharacter   = 'x';

   testSlider = std::make_shared<Slider>(Slider());
   allPrintables.push_back(testSlider);
   testSlider->getCurrentAnimationMutable().setAllSpriteLayers(200);
   testSlider->setDynamicPosition(ScreenLockPosition::BOTTOM_LEFT_CORNER);

   backgroundColorButton = PrintableFactory::newButton("Background Color", &AppState::backgroundColorButtonFunc, this);
   textColorButton = PrintableFactory::newButton("Text Color", &AppState::textColorButtonFunc, this);
   currentCharacterButton = PrintableFactory::newButton("Current Character", &AppState::currentCharacterButtonFunc, this);
   eraserSelectButton = PrintableFactory::newButton("Eraser", &AppState::eraserSelectButtonFunc, this);
   frameLengthButton = PrintableFactory::newButton("Frame Length", &AppState::frameLengthButtonFunc, this);
   nextFrameButton = PrintableFactory::newButton("Next Frame", &AppState::nextFrameButtonFunc, this);
   playAnimationButton = PrintableFactory::newButton("Play Animation", &AppState::playAnimationButtonFunc, this);
   previousFrameButton = PrintableFactory::newButton("Previous Frame", &AppState::previousFrameButtonFunc, this);
   quitButton = PrintableFactory::newButton("Quit", &AppState::quitButtonFunc, this);

   currentCharacterButton->setDynamicPosition(ScreenLockPosition::TOP_RIGHT_CORNER);
   eraserSelectButton->setDynamicPosition(ScreenLockPosition::TOP_RIGHT_CORNER);
   frameLengthButton->setDynamicPosition(ScreenLockPosition::TOP_RIGHT_CORNER);

   textColorButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE);
   backgroundColorButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE);
   previousFrameButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE, StackDirection::HORIZONTAL);
   nextFrameButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE, StackDirection::HORIZONTAL);
   playAnimationButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE, StackDirection::HORIZONTAL);

   quitButton->setDynamicPosition(ScreenLockPosition::TOP_LEFT_CORNER);
   UIElement::updateAllLockedPositions();

   // Register UI elements with the global InputHandler
   globalInputHandler.addSlider(testSlider);
};

// public ----------------------------------------------------------------------------------------------------
void AppState::update()
{
   // Handle character selection mode
   if (selectNewCharacter)
   {
      drawingCharacter = userInput;
      selectNewCharacter = false;
   }
   // Handle mouse input for drawing and camera
   else if (userInput == KEY_MOUSE)
   {
      // Get the mouse event that was already processed by InputHandler
      MEVENT* mouseEvent = globalInputHandler.getLastMouseEvent();
      if (mouseEvent != nullptr)
      {
         // Camera Drag
         if (mouseEvent->bstate & BUTTON2_PRESSED)
         {
            cameraDrag = true;
            lastMousePosition.setX(mouseEvent->x);
            lastMousePosition.setY(mouseEvent->y);
         }
         if (mouseEvent->bstate & BUTTON1_PRESSED)
         {
            drawing = true;

            // Only draw if not over any UI element
            Position mousePos(mouseEvent->x, mouseEvent->y);
            if (!globalInputHandler.isMouseOverUI(mousePos)) {
                int   worldX   = mouseEvent->x - currentCamera->getLengthOffset();
                int   worldY   = mouseEvent->y - currentCamera->getHeightOffset();
               Pixel newPixel = Pixel(Position(worldX, worldY), drawingCharacter);
               userEntity->getCurrentAnimationMutable().addPixelToCurrentFrame(newPixel);
            }
         }

         if (mouseEvent->bstate & BUTTON2_RELEASED)
         {
            cameraDrag = false;
            lastMousePosition.setX(-1);
            lastMousePosition.setY(-1);
         }
         if (mouseEvent->bstate & BUTTON1_RELEASED)
         {
            drawing = false;
         }

         if (mouseEvent->bstate & (BUTTON1_RELEASED | BUTTON1_CLICKED))
         {
            drawing = false;
         }
         if (mouseEvent->bstate & (BUTTON2_RELEASED | BUTTON2_CLICKED))
         {
            cameraDrag = false;
         }

         if (mouseEvent->bstate & REPORT_MOUSE_POSITION)
         {
            if (cameraDrag)
            {
               // Calculate how much the mouse moved
               int dx = lastMousePosition.getX() - mouseEvent->x;
               int dy = lastMousePosition.getY() - mouseEvent->y;

               // Pan the camera accordingly
               currentCamera->displaceViewPort(-dx, -dy);

               // Update the last known position
               lastMousePosition.setX(mouseEvent->x);
               lastMousePosition.setY(mouseEvent->y);
            }
            else if (drawing)
            {
               // Only draw if not over any UI element
               Position mousePos(mouseEvent->x, mouseEvent->y);
               if (!globalInputHandler.isMouseOverUI(mousePos)) {
                   int   worldX   = mouseEvent->x - currentCamera->getLengthOffset();
                   int   worldY   = mouseEvent->y - currentCamera->getHeightOffset();
               Pixel newPixel = Pixel(Position(worldX, worldY), drawingCharacter);
               userEntity->getCurrentAnimationMutable().addPixelToCurrentFrame(newPixel);
            }
         }
      }
   }
   }
}

// public ----------------------------------------------------------------------------------------------------
void AppState::onExit()
{
   clear();
   allPrintables.clear();
   globalInputHandler.clear();
}

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

// public ----------------------------------------------------------------------------------------------------
void AppState::backgroundColorButtonFunc() {}

// public ----------------------------------------------------------------------------------------------------
void AppState::textColorButtonFunc() {}