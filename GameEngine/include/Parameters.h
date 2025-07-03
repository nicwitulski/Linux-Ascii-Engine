//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Parameters.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief File for all global variables
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PARAMETERS_H
#define PARAMETERS_H

#pragma once
#include "Camera.h"
#include "Entity.h"
#include "UIElement.h"
#include <memory>
#include <vector>

// Forward declaration
class InputHandler;

extern std::vector<std::shared_ptr<Printable>> printablesToSave;
extern std::vector<std::shared_ptr<Printable>> allPrintables;
extern bool                                    printablesNeedSorted;

extern std::shared_ptr<Camera> currentCamera;
extern std::shared_ptr<Entity> playerEntity;

extern int SCREEN_LENGTH;
extern int SCREEN_HEIGHT;

extern int  userInput;
extern bool engineRunning;
extern bool displayNeedsCleared;

extern InputHandler globalInputHandler;

#endif
