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
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::onEnter()
{
   // Create window with auto-resize enabled from the start
   mainMenuWindow = std::make_shared<NcursesWindow>(120, 15, 1, false, 0, 0);
   ncursesWindows.push_back(mainMenuWindow);

   // Add the main menu window to the input context
   globalInputHandler.addContext(mainMenuWindow);

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

   // Initialize animation browser
   showAnimationBrowser = false;
   animationBrowserMenu = nullptr;

   UIElement::updateAllLockedPositions();
}

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::update()
{
   // Handle animation browser input if active
   if (showAnimationBrowser && animationBrowserMenu)
   {
      // Check for keyboard input specifically for the menu
      // Note: userInput is set by the GameEngine's input handling
      if (userInput != 0)
      {
         if (animationBrowserMenu->handleInput(userInput))
         {
            userInput = 0; // Clear input after handling
         }
      }
   }
   // Clean up animation browser if it should be closed
   else if (!showAnimationBrowser && animationBrowserMenu)
   {
      onAnimationBrowserCancelled();
   }

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

   // Clean up animation browser menu
   if (animationBrowserMenu)
   {
      // Remove from main menu window
      mainMenuWindow->removePrintable(animationBrowserMenu);
      animationBrowserMenu->setActive(false);
      animationBrowserMenu = nullptr;
   }
   showAnimationBrowser = false;

   // Clear all contexts and UI elements
   globalInputHandler.clearContext();
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
   createAnimationBrowserMenu();
}

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::createAnimationBrowserMenu()
{
   std::vector<std::string> animationDirs = scanAnimationDirectories();

   if (animationDirs.empty())
   {
      return;
   }

   // Create menu with the found directories
   animationBrowserMenu = std::make_shared<Menu>(animationDirs, 10, 50);

   // Set callbacks
   animationBrowserMenu->setOnSelectCallback([this](const std::string& animationName)
                                             { onAnimationSelected(animationName); });

   animationBrowserMenu->setOnCancelCallback([this]() { onAnimationBrowserCancelled(); });

   // Add to the main menu window to ensure proper layering
   animationBrowserMenu->setNcurseWindow(mainMenuWindow->getWindow());
   mainMenuWindow->addPrintable(animationBrowserMenu);
   mainMenuWindow->setPrintablesNeedSorted(true);

   // Disable main menu buttons while browser is open by removing window context
   globalInputHandler.removeContext(mainMenuWindow);

   animationBrowserMenu->setActive(true);
   showAnimationBrowser = true;

   // Set menu sprite layers to be much higher than buttons
   animationBrowserMenu->setAllAnimationSpriteLayers(2);

   // Set dynamic position and update after everything is set up
   animationBrowserMenu->setDynamicPosition(ScreenLockPosition::CENTER);
   UIElement::updateWindowLockedPositions(mainMenuWindow->getWindow());

   // Force display refresh to ensure proper layering
   displayNeedsCleared = true;
}

// public ----------------------------------------------------------------------------------------------------
std::vector<std::string> MainMenuState::scanAnimationDirectories()
{
   namespace fs = std::filesystem;
   std::vector<std::string> directories;

   std::string animationsPath = "./src/Animations/";

   try
   {
      if (fs::exists(animationsPath) && fs::is_directory(animationsPath))
      {
         for (const auto& entry : fs::directory_iterator(animationsPath))
         {
            if (entry.is_directory())
            {
               std::string dirName = entry.path().filename().string();
               directories.push_back(dirName);
            }
         }
      }
      else
      {
         std::cerr << "Animation directory not found: " << animationsPath << std::endl;
      }
   }
   catch (const fs::filesystem_error& e)
   {
      std::cerr << "Error scanning animation directories: " << e.what() << std::endl;
   }

   // Sort directories alphabetically
   std::sort(directories.begin(), directories.end());

   return directories;
}

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::onAnimationSelected(const std::string& animationName)
{
   // Mark the browser for closure instead of immediate cleanup to avoid segfault
   showAnimationBrowser = false;

   // Explicitly remove from positioning vectors to prevent accumulation
   if (animationBrowserMenu)
   {
      UIElement::removeFromPositioningVectors(animationBrowserMenu);
   }

   // Force display refresh
   displayNeedsCleared = true;

   // TODO: In the future, this would load the selected animation into the editor
}

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::onAnimationBrowserCancelled()
{
   if (animationBrowserMenu)
   {
      // Remove from main menu window
      mainMenuWindow->removePrintable(animationBrowserMenu);
      animationBrowserMenu->setActive(false);

      // Explicitly remove from positioning vectors to prevent accumulation
      UIElement::removeFromPositioningVectors(animationBrowserMenu);

      animationBrowserMenu = nullptr;
   }

   // Re-enable main menu buttons by adding window context back
   globalInputHandler.addContext(mainMenuWindow);

   showAnimationBrowser = false;

   // Force display refresh to ensure the menu disappears
   displayNeedsCleared = true;
}

// public ----------------------------------------------------------------------------------------------------
void MainMenuState::quitFunction()
{
   nextState = States::Quit;
}