//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file main.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Main application entry point that initializes and runs the GameEngine
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "StateLogic/MainMenuState.h"

// public ----------------------------------------------------------------------------------------------------
int main()
{
   GameEngine engine(new MainMenuState());
   engine.run();
   return 0;
}