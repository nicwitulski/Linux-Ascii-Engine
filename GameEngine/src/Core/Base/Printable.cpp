//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Printable.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Gives an object that is meant to be printed to an ncurses display attribues that allow it to be
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/Printable.h"
#include "../../../include/Animation.h"

// public ----------------------------------------------------------------------------------------------------
Printable::Printable()
{
   m_currentAnimationName = "default";
   m_animations.push_back(Animation());
   m_visable = false;
};

// public ----------------------------------------------------------------------------------------------------
void Printable::addAnimation(const Animation animation)
{
   m_animations.push_back(animation);
};

// public ----------------------------------------------------------------------------------------------------
bool Printable::setCurrentAnimation(const std::string name)
{
   for (const Animation& animation : m_animations)
   {
      if (animation.getAnimationName() == name)
      {
         m_currentAnimationName = animation.getAnimationName();
         return true;
      }
   }
   return false; // If not loaded, stays as current animation
};

// public ----------------------------------------------------------------------------------------------------
const std::string& Printable::getCurrentAnimationName() const
{
   return m_currentAnimationName;
};

// public ----------------------------------------------------------------------------------------------------
const std::vector<Animation>& Printable::getAnimations() const
{
   return m_animations;
};

// public ----------------------------------------------------------------------------------------------------
std::vector<Animation>& Printable::getAnimationsMutable()
{
   return m_animations;
};

// public ----------------------------------------------------------------------------------------------------
void Printable::displace(const int dx, const int dy)
{
   for (Animation& animation : m_animations)
   {
      if (m_currentAnimationName == animation.getAnimationName())
      {
         m_dirtySprites.push_back(animation.getCurrentFrameSprite());
         animation.displace(dx, dy);
         break;
      }
   }
};

// public ----------------------------------------------------------------------------------------------------
bool Printable::isVisable() const
{
   return m_visable;
};

// public ----------------------------------------------------------------------------------------------------
void Printable::setVisability(const bool visable)
{
   m_visable = visable;
};

// public ----------------------------------------------------------------------------------------------------
bool Printable::isMoveableByCamera() const
{
   return m_moveableByCamera;
};

// public ----------------------------------------------------------------------------------------------------
void Printable::setMoveableByCamera(const bool moveableByCamera)
{
   m_moveableByCamera = moveableByCamera;
};

// public ----------------------------------------------------------------------------------------------------
const Animation& Printable::getCurrentAnimation() const
{
   for (const Animation& animation : m_animations)
   {
      if (m_currentAnimationName == animation.getAnimationName())
      {
         return animation;
      }
   }
   return m_animations.at(0);
};

// public ----------------------------------------------------------------------------------------------------
Animation& Printable::getCurrentAnimationMutable()
{
   for (Animation& animation : m_animations)
   {
      if (m_currentAnimationName == animation.getAnimationName())
      {
         return animation;
      }
   }
   return m_animations.at(0);
};

// public ----------------------------------------------------------------------------------------------------
const std::vector<Sprite>& Printable::getDirtySprites() const
{
   return m_dirtySprites;
};

// public ----------------------------------------------------------------------------------------------------
void Printable::addDirtySprite(const Sprite dirtySprite)
{
   m_dirtySprites.push_back(dirtySprite);
};

// public ----------------------------------------------------------------------------------------------------
void Printable::moveToPosition(const Position position)
{
   for (Animation& animation : m_animations)
   {
      animation.getCurrentFrameSpriteMutable().moveAnchorToPosition(position);
   }
};

// public ----------------------------------------------------------------------------------------------------
void Printable::clearDirtySprites()
{
   m_dirtySprites.clear();
};

// public ----------------------------------------------------------------------------------------------------
void Printable::setAllAnimationSpriteLayers(const int layer)
{
   for (Animation& animation : m_animations)
   {
      animation.setAllSpriteLayers(layer);
   }
};