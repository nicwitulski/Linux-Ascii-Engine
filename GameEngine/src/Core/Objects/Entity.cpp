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

#include "../../../include/Entity.h"

// public ----------------------------------------------------------------------------------------------------
Entity::Entity()
{
   m_entityName           = "none";
   m_currentAnimationName = "default";
   m_animations.push_back(Animation());
   m_visable                 = false;
   m_moveableByCamera        = true;
   Sprite m_spriteBeforeMove = Sprite();
};

// public ----------------------------------------------------------------------------------------------------
Entity::Entity(const std::string entityName, const std::vector<Animation> animations, const bool visable,
               const bool moveableByCamera)
{
   m_entityName = entityName;
   m_animations = animations;

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
   m_visable                 = visable;
   m_moveableByCamera        = moveableByCamera;
   Sprite m_spriteBeforeMove = Sprite();
};

// public ----------------------------------------------------------------------------------------------------
const std::string& Entity::getEntityName() const
{
   return m_entityName;
};

// public ----------------------------------------------------------------------------------------------------
void Entity::setEntityName(const std::string entityName)
{
   m_entityName = entityName;
};

// public ----------------------------------------------------------------------------------------------------
bool Entity::positionInBoundsOfEntity(const Position position)
{
   return getCurrentAnimation().getCurrentFrameSprite().positionInBounds(position);
};