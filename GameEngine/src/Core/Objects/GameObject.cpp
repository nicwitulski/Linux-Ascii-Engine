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

#include "../../../include/GameObject.h"

// public ----------------------------------------------------------------------------------------------------
GameObject::GameObject()
{
   m_visable              = false;
   m_moveableByCamera     = true;
   m_currentAnimationName = "default";
   m_animations.push_back(Animation());
};

// public ----------------------------------------------------------------------------------------------------
GameObject::GameObject(bool visable, bool moveableByCamera, std::vector<Animation> animations,
                       std::string currentAnimation)
{
   m_visable              = visable;
   m_moveableByCamera     = moveableByCamera;
   m_animations           = animations;
   m_currentAnimationName = currentAnimation;
};