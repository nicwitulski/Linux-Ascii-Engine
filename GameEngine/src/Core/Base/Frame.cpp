//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Frame.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Holds a sprite and a duration (in seconds). Used in an Animation
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/Frame.h"

// public ----------------------------------------------------------------------------------------------------
Frame::Frame()
{
   m_sprite            = Sprite();
   m_durationInSeconds = 1.0f;
};

// public ----------------------------------------------------------------------------------------------------
Frame::Frame(const Sprite sprite, const float duration)
{
   m_sprite            = sprite;
   m_durationInSeconds = duration;
};

// public ----------------------------------------------------------------------------------------------------
const float& Frame::getDuration() const
{
   return m_durationInSeconds;
};

// public ----------------------------------------------------------------------------------------------------
const Sprite& Frame::getSprite() const
{
   return m_sprite;
};

// public ----------------------------------------------------------------------------------------------------
Sprite& Frame::getMutableSprite()
{
   return m_sprite;
}

// public ----------------------------------------------------------------------------------------------------
void Frame::setSprite(Sprite sprite)
{
   m_sprite = sprite;
};

// public ----------------------------------------------------------------------------------------------------
void Frame::displace(const int dx, const int dy)
{
   m_sprite.displace(dx, dy);
};