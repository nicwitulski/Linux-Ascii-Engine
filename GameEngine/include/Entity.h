//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Entity.h
/// @author Nicholas Witulski
/// @brief Game object that has a name
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ENTITY_H
#define ENTITY_H

#include "GameObject.h"
#include <iostream>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class Entity
///
/// Game object that has a name
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Entity : public GameObject
{
private:
public:
   Entity();
   Entity(const std::string entityName, const std::vector<Animation> animations, const bool visable,
          const bool moveableByCamera);
   Entity(std::string entityName, bool visable, bool moveableByCamera);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn positionInBoundsOfEntity
   ///
   /// @param position - position to check if its in the bounds of the entity
   /// @return boolean of if the given position is in the current animation
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   bool positionInBoundsOfEntity(const Position position);
};

#endif