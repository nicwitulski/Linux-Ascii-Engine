#ifndef PARAMETERS_H
#define PARAMETERS_H

#pragma once
#include "Camera.h"
#include "Entity.h"
#include "UIElement.h"
#include <memory>
#include <vector>

extern std::vector<std::shared_ptr<Printable>> allPrintables;
extern bool printablesNeedSorted;

extern std::shared_ptr<Camera> currentCamera;
extern std::shared_ptr<Entity> playerEntity;

extern int SCREEN_LENGTH;
extern int SCREEN_HEIGHT;

extern int userInput;
extern bool engineRunning;
extern bool displayNeedsCleared;

#endif
