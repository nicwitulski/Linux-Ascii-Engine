//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file MainMenuState.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Main menu. Loads animation project or create new project
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
   mainMenu = PrintableFactory::loadUIElement("mainMenuSprite", true, false);
   mainMenu->setAllAnimationSpriteLayers(0);
   mainMenu->setDynamicPosition(ScreenLockPosition::TOP_MIDDLE);

   newAnimationButton = PrintableFactory::newButton("New Animation", &MainMenuState::newAnimationFunction, this);
   newAnimationButton->setAllAnimationSpriteLayers(1);
   newAnimationButton->setDynamicPosition(ScreenLockPosition::CENTER);

   loadAnimationButton = PrintableFactory::newButton("Load Animation", &MainMenuState::loadAnimationFunction, this);
   loadAnimationButton->setAllAnimationSpriteLayers(1);
   loadAnimationButton->setDynamicPosition(ScreenLockPosition::CENTER);

   quitButton = PrintableFactory::newButton("Quit", &MainMenuState::quitFunction, this);
   quitButton->setAllAnimationSpriteLayers(1);
   quitButton->setDynamicPosition(ScreenLockPosition::CENTER);

   currentCamera = std::make_shared<Camera>(SCREEN_LENGTH, SCREEN_HEIGHT);
   playerEntity  = nullptr;
};

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::update()
{
   // No mouse handling needed here - it's handled automatically by the global InputHandler
   // The buttons will execute their functions automatically when clicked
}

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::onExit()
{
   clear();
   allPrintables.clear();
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