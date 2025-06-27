//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Button.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Defines a button with an executable function
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/Button.h"

Button::~Button() = default;

// public ----------------------------------------------------------------------------------------------------
Button::Button()
{
   m_lockPosition = ScreenLockPosition::NONE;

   m_currentAnimationName = "default";
   m_animations.push_back(Animation());
   m_visable                 = false;
   m_moveableByCamera        = true;
   Sprite m_spriteBeforeMove = Sprite();
   m_minPosition             = Position(0, 0);
   m_maxPosition             = Position(0, 0);
};

// public ----------------------------------------------------------------------------------------------------
Button::Button(const std::vector<Animation> animations, const bool visable, const bool moveableByCamera,
               std::function<void()> function)
{
   m_animations              = animations;
   m_currentAnimationName    = m_animations.at(0).getAnimationName();
   m_visable                 = visable;
   m_moveableByCamera        = moveableByCamera;
   m_lockPosition            = ScreenLockPosition::NONE;
   Sprite m_spriteBeforeMove = Sprite();
   m_minPosition             = Position(0, 0);
   m_maxPosition             = Position(0, 0);
   setFunction(function);
   setPositions();
};

// public ----------------------------------------------------------------------------------------------------
void Button::setFunction(std::function<void()> func)
{
   m_function = std::move(func);
};

// public ----------------------------------------------------------------------------------------------------
void Button::executeFunction()
{
   if (m_function)
   {
      m_function();
   }
   else
   {
      {
         return;
      }
   }
};

// public ----------------------------------------------------------------------------------------------------
bool Button::mouseInBounds(Position position)
{
   return getCurrentAnimation().getCurrentFrameSprite().positionInBounds(position);
};