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
         
         // For mouse events, always process through InputHandler AND state
         if (userInput == KEY_MOUSE)
         {
            globalInputHandler.processInput(userInput);
            // State will also process mouse events in its update()
         }
         else
         {
            // For non-mouse events, use exclusive logic
            if (!globalInputHandler.processInput(userInput))
            {
               // If InputHandler didn't handle the input, let the state handle it
               // This allows states to handle their own specific input logic
            }
         }
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