//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file GameObject.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Base class for all game objects with common attributes and functionality
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
/// Base class for all game objects with common attributes and functionality.
/// Any game object should inherit from this class.
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