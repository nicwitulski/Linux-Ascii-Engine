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
   newAnimationButton = PrintableFactory::loadButton("newAnimationButton", true, false,
                                                     [this]() { this->newAnimationFunction(); });
   newAnimationButton->setAllAnimationSpriteLayers(1);
   loadAnimationButton = PrintableFactory::loadButton("loadAnimationButton", true, false,
                                                      [this]() { this->loadAnimationFunction(); });
   loadAnimationButton->setAllAnimationSpriteLayers(1);
   quitButton = PrintableFactory::loadButton("quitButton", true, false, [this]() { this->quitFunction(); });
   quitButton->setAllAnimationSpriteLayers(1);

   SCREEN_LENGTH = 120;
   currentCamera = std::make_unique<Camera>(SCREEN_LENGTH, SCREEN_HEIGHT);
   playerEntity  = nullptr;

   newAnimationButton->displace(58, 11);
   loadAnimationButton->displace(58, 13);
   quitButton->displace(58, 15);
};

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::update()
{
   if (userInput == KEY_MOUSE)
   {
      if (getmouse(&event) == OK)
      {
         if (event.bstate & BUTTON1_PRESSED)
         {
            Position mousePosition = Position(event.x, event.y);

            if (newAnimationButton->mouseInBounds(mousePosition))
            {
               newAnimationButton->executeFunction();
            }
            else if (loadAnimationButton->mouseInBounds(mousePosition))
            {
               loadAnimationButton->executeFunction();
            }
            else if (quitButton->mouseInBounds(mousePosition))
            {
               quitButton->executeFunction();
            }
         }
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::onExit()
{
   clear();
   allPrintables.clear();
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