//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file QuitState.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Quits the application and game engine
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "QuitState.h"

// public ----------------------------------------------------------------------------------------------------
void QuitState::onEnter()
{
   for (const auto& printable : printablesToSave)
   {
      PrintableFactory::writePrintableToTextFiles(printable);
   }
   engineRunning = false;
}

// public ----------------------------------------------------------------------------------------------------
void QuitState::update() {}

// public ----------------------------------------------------------------------------------------------------
void QuitState::onExit() {}