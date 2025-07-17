//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Slider.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of Slider class for UI slider functionality
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/Slider.h"
#include "../../../include/Animation.h"
#include "../../../include/Frame.h"
#include "../../../include/Pixel.h"
#include "../../../include/Sprite.h"
#include <algorithm>

// public ----------------------------------------------------------------------------------------------------
Slider::Slider(int length, bool horizontal)
   : UIElement(), m_length(std::max(2, length)), m_position(0), m_horizontal(horizontal)
{
   m_printableName    = "defaultSlider";
   m_lockPosition     = ScreenLockPosition::NONE;
   m_stackDirection   = StackDirection::VERTICAL;
   m_visable          = true;
   m_moveableByCamera = false;
   m_ncurseWindow     = nullptr;
   updateSprite();
}

// public ----------------------------------------------------------------------------------------------------
void Slider::setLength(int length)
{
   m_length   = std::max(2, length);
   m_position = std::clamp(m_position, 0, m_length - 1);
   updateSprite();
}

// public ----------------------------------------------------------------------------------------------------
int Slider::getLength() const
{
   return m_length;
}

// public ----------------------------------------------------------------------------------------------------
void Slider::setPosition(int pos)
{
   m_position = std::clamp(pos, 0, m_length - 1);
   updateSprite();
}

// public ----------------------------------------------------------------------------------------------------
int Slider::getPosition() const
{
   return m_position;
}

// public ----------------------------------------------------------------------------------------------------
void Slider::moveLeft()
{
   setPosition(m_position - 1);
}

// public ----------------------------------------------------------------------------------------------------
void Slider::moveRight()
{
   setPosition(m_position + 1);
}

// public ----------------------------------------------------------------------------------------------------
float Slider::getValue() const
{
   if (m_length <= 1)
      return 0.0f;
   return float(m_position) / float(m_length - 1);
}

// public ----------------------------------------------------------------------------------------------------
void Slider::setAnimation(const Animation& animation)
{
   m_animations.clear();
   m_animations.push_back(animation);
   m_currentAnimationName = animation.getAnimationName();
}

// public ----------------------------------------------------------------------------------------------------
bool Slider::mouseInBounds(Position position)
{
   // If slider is associated with a window, adjust mouse coordinates to be relative to that window
   if (m_ncurseWindow && m_ncurseWindow != stdscr)
   {
      int      windowX = getbegx(m_ncurseWindow);
      int      windowY = getbegy(m_ncurseWindow);
      Position windowRelativePosition(position.getX() - windowX, position.getY() - windowY);
      return getCurrentAnimation().getCurrentFrameSprite().positionInBounds(windowRelativePosition);
   }
   else
   {
      // For stdscr or no window, use global coordinates
      return getCurrentAnimation().getCurrentFrameSprite().positionInBounds(position);
   }
}

// public ----------------------------------------------------------------------------------------------------
void Slider::setPositionFromMouse(Position mousePosition)
{
   // If slider is associated with a window, adjust mouse coordinates to be relative to that window
   Position adjustedMousePosition = mousePosition;
   if (m_ncurseWindow && m_ncurseWindow != stdscr)
   {
      int windowX           = getbegx(m_ncurseWindow);
      int windowY           = getbegy(m_ncurseWindow);
      adjustedMousePosition = Position(mousePosition.getX() - windowX, mousePosition.getY() - windowY);
   }

   const Sprite& sprite = getCurrentAnimation().getCurrentFrameSprite();
   Position      anchor = sprite.getAnchor();

   if (m_horizontal)
   {
      int relativeX   = adjustedMousePosition.getX() - anchor.getX();
      int newPosition = std::clamp(relativeX, 0, m_length - 1);
      setPosition(newPosition);
   }
   else
   {
      int relativeY   = adjustedMousePosition.getY() - anchor.getY();
      int newPosition = std::clamp(relativeY, 0, m_length - 1);
      setPosition(newPosition);
   }
}

// private ---------------------------------------------------------------------------------------------------
void Slider::updateSprite()
{
   Position prevAnchor =
         m_animations.empty() ? Position(0, 0) : m_animations.front().getCurrentFrameSprite().getAnchor();

   std::vector<Pixel> pixels;
   if (m_horizontal)
   {
      for (int i = 0; i < m_length; ++i)
      {
         char ch = (i == m_position) ? '|' : '-';
         pixels.push_back(Pixel(Position(i, 0), ch));
      }
   }
   else
   {
      for (int i = 0; i < m_length; ++i)
      {
         char ch = (i == m_position) ? '|' : '-';
         pixels.push_back(Pixel(Position(0, i), ch));
      }
   }
   Sprite sprite(pixels);
   sprite.moveAnchorToPosition(prevAnchor);

   Animation anim("default", {Frame(sprite, 1.0f)}, true);
   m_animations.clear();
   m_animations.push_back(anim);
   m_currentAnimationName = "default";
}
