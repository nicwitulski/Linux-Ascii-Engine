//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file QuitState.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of QuitState for application termination
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
void QuitState::update() 
{
   // No update logic needed for quit state
}

// public ----------------------------------------------------------------------------------------------------
void QuitState::onExit() 
{
   // No cleanup needed for quit state
}