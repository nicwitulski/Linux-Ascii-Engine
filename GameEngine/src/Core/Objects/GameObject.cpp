//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file GameObject.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of GameObject class for base game object functionality
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/GameObject.h"

// public ----------------------------------------------------------------------------------------------------
GameObject::GameObject()
{
   m_visable              = false;
   m_moveableByCamera     = true;
   m_currentAnimationName = "default";
   m_animations.push_back(Animation());
   m_printableName = "default";
   m_ncurseWindow  = nullptr;
};

// public ----------------------------------------------------------------------------------------------------
GameObject::GameObject(const std::string printableName, bool visable, bool moveableByCamera,
                       std::vector<Animation> animations, std::string currentAnimation)
{
   m_printableName        = printableName;
   m_visable              = visable;
   m_moveableByCamera     = moveableByCamera;
   m_animations           = animations;
   m_currentAnimationName = currentAnimation;
   m_ncurseWindow         = nullptr;
};