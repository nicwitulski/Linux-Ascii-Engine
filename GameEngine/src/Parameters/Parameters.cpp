//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Parameters.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of global parameters and variables
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../include/Parameters.h"
#include "../../include/InputHandler.h"

std::vector<std::shared_ptr<Printable>>     printablesToSave;
std::vector<std::shared_ptr<NcursesWindow>> ncursesWindows;
//std::vector<std::shared_ptr<Printable>>     allPrintables;
bool printablesNeedSorted = true;

std::shared_ptr<Camera> currentCamera;

std::shared_ptr<Entity> playerEntity;

int SCREEN_HEIGHT = 24;
int SCREEN_LENGTH = 80;

int  userInput           = 0;
bool engineRunning       = false;
bool displayNeedsCleared = false;

InputHandler globalInputHandler;