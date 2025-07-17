//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Entity.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of Entity class for game object management
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/Entity.h"

// public ----------------------------------------------------------------------------------------------------
Entity::Entity()
{
   m_printableName        = "none";
   m_currentAnimationName = "default";
   m_animations.push_back(Animation());
   m_visable          = false;
   m_moveableByCamera = true;
   m_ncurseWindow     = nullptr;
};

// public ----------------------------------------------------------------------------------------------------
Entity::Entity(const std::string entityName, const std::vector<Animation> animations, const bool visable,
               const bool moveableByCamera)
{
   m_printableName = entityName;
   m_animations    = animations;

   if (!animations.empty())
   {
      m_currentAnimationName = animations.at(0).getAnimationName();
   }
   else
   {
      std::cerr << "Warning: Entity \"" << entityName << "\" has no animations!" << std::endl;
      m_currentAnimationName = "none";
   }

   m_currentAnimationName = animations.at(0).getAnimationName(); // Default is first loaded animation,
                                                                 // change manually
   m_visable          = visable;
   m_moveableByCamera = moveableByCamera;
   m_ncurseWindow     = nullptr;
};

// public ----------------------------------------------------------------------------------------------------
bool Entity::positionInBoundsOfEntity(const Position position)
{
   return getCurrentAnimation().getCurrentFrameSprite().positionInBounds(position);
};