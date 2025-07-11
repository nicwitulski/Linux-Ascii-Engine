//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file GameObject.h
/// @author Nicholas Witulski
/// @brief Way to denote attributes (not the ncurses kind haha) for all gameObjects.
/// Any object should inherit this class.
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Printable.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class GameObject
///
/// Way to denote attributes (not the ncurses kind haha) for all gameObjects.
/// Any object should inherit this class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GameObject : public Printable
{
protected:
public:
   GameObject();
   GameObject(const std::string printableName, const bool visable, const bool moveableByCamera,
              const std::vector<Animation> animations, const std::string currentAnimation);
};

#endif