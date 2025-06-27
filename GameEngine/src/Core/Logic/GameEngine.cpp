//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file GameEngine.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Highest level file, holds all needed includes. Create an instance of this object and call .run()
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/GameEngine.h"
#include <ncurses.h>
#include <unistd.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <thread>

using Clock = std::chrono::steady_clock;

// public ----------------------------------------------------------------------------------------------------
void GameEngine::run()
{
   userInput     = 0;
   auto lastTime = Clock::now();

   Display::initCurse();
   engineRunning = true;

   while (engineRunning)
   {
      // --- Time Tracking ---
      auto                         currentTime = Clock::now();
      std::chrono::duration<float> delta       = currentTime - lastTime;
      float                        deltaTime   = delta.count();
      lastTime                                 = currentTime;

      // --- Input Handling ---
      int ch;
      while ((ch = getch()) != ERR)
      {
         userInput = ch;
         if (userInput == '`')
            engineRunning = false;
      }
      // State Update
      currentState->update();
      GameState* next = currentState->getNextState();
      if (next)
      {
         currentState->onExit();
         delete currentState;
         currentState = next;
         currentState->onEnter();
      }

      // --- Refresh display using actual deltaTime ---
      Display::refreshDisplay(deltaTime);

      std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Prevent CPU maxing out
   }

   exit();
}

// private ---------------------------------------------------------------------------------------------------
void GameEngine::exit()
{
   Display::closeCurseWindow();
}