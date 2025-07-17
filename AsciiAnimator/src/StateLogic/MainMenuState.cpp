//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file MainMenuState.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of MainMenuState for animation project management
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MainMenuState.h"
#include "QuitState.h"
#include <ncurses.h>
#include <memory>

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::onEnter()
{
   // Create window with auto-resize enabled from the start
   mainMenuWindow = std::make_shared<NcursesWindow>(120, 15, 1, false, 0, 0);
   ncursesWindows.push_back(mainMenuWindow);

   mainMenu = PrintableFactory::loadUIElement("mainMenuSprite", true, false, mainMenuWindow);
   mainMenu->setAllAnimationSpriteLayers(0);
   mainMenu->setDynamicPosition(ScreenLockPosition::TOP_MIDDLE);

   newAnimationButton = PrintableFactory::newButton("New Animation", &MainMenuState::newAnimationFunction,
                                                    this, mainMenuWindow);
   newAnimationButton->setAllAnimationSpriteLayers(1);
   newAnimationButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE);

   loadAnimationButton = PrintableFactory::newButton("Load Animation", &MainMenuState::loadAnimationFunction,
                                                     this, mainMenuWindow);
   loadAnimationButton->setAllAnimationSpriteLayers(1);
   loadAnimationButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE);

   quitButton = PrintableFactory::newButton("Quit", &MainMenuState::quitFunction, this, mainMenuWindow);
   quitButton->setAllAnimationSpriteLayers(1);
   quitButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE);

   currentCamera = std::make_shared<Camera>(SCREEN_LENGTH, SCREEN_HEIGHT);
   playerEntity  = nullptr;

   UIElement::updateAllLockedPositions();
}

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::update()
{
   // Mouse handling is managed automatically by the global InputHandler
   // Button interactions are processed automatically when clicked
}

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::onExit()
{
   clear();
   ncursesWindows.at(0)->clearPrintables();
   if (mainMenuWindow)
   {
      mainMenuWindow->clearPrintables();
      Display::removeWindow(mainMenuWindow);
      mainMenuWindow = nullptr;
   }
   globalInputHandler.clear();
}

// public ----------------------------------------------------------------------------------------------------
GameState* MainMenuState::getNextState()
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
void MainMenuState::newAnimationFunction()
{
   nextState = States::Drawing;
}

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::loadAnimationFunction()
{
   nextState = States::Drawing;
}

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::quitFunction()
{
   nextState = States::Quit;
}