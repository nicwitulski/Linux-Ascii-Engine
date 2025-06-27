//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Animation.h
/// @author Nicholas Witulski (nicwitulski@gmail.con)
/// @brief Defines an Animation which holds a vector of frames and advances through them
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/Animation.h"

// public ----------------------------------------------------------------------------------------------------
Animation::Animation()
{
   m_animationName = "none";
   m_frames.push_back(Frame());
   m_repeats = true;
   m_playing = true;
};

// public ----------------------------------------------------------------------------------------------------
Animation::Animation(const std::string animationName, const std::vector<Frame> frames, const bool repeats)
{
   m_animationName = animationName;
   m_frames        = frames;
   m_repeats       = repeats;
   m_playing       = true;
};

// public ----------------------------------------------------------------------------------------------------
const std::vector<Frame>& Animation::getFrames() const
{
   return m_frames;
};

// public ----------------------------------------------------------------------------------------------------
void Animation::update(const float deltaTime)
{
   if (m_frames.empty() || !m_playing)
      return;

   frameTimer += deltaTime;

   previousFrameIndex = currentFrameIndex;

   while (frameTimer >= m_frames[currentFrameIndex].getDuration())
   {
      frameTimer -= m_frames[currentFrameIndex].getDuration();
      currentFrameIndex++;

      if (currentFrameIndex >= m_frames.size())
      {
         if (m_repeats)
            currentFrameIndex = 0;
         else
            currentFrameIndex = m_frames.size() - 1; // Stop at last frame
      }
   }
};

// public ----------------------------------------------------------------------------------------------------
void Animation::manuallyIncrementFrame()
{
   previousFrameIndex = currentFrameIndex;
   currentFrameIndex++;
   if (currentFrameIndex >= m_frames.size())
   {
      if (m_repeats)
         currentFrameIndex = 0;
      else
         currentFrameIndex = m_frames.size() - 1; // Stop at last frame
   }
};

// public ----------------------------------------------------------------------------------------------------
void Animation::manuallyDecrementFrame()
{
   previousFrameIndex = currentFrameIndex;
   currentFrameIndex--;
   if (currentFrameIndex >= m_frames.size())
   {
      if (m_repeats)
         currentFrameIndex = 0;
      else
         currentFrameIndex = m_frames.size() - 1; // Stop at last frame
   }
};

// public ----------------------------------------------------------------------------------------------------
const Sprite& Animation::getCurrentFrameSprite() const
{
   return m_frames[currentFrameIndex].getSprite();
};

// public ----------------------------------------------------------------------------------------------------
Sprite& Animation::getCurrentFrameSpriteMutable()
{
   return m_frames[currentFrameIndex].getMutableSprite();
};

// public ----------------------------------------------------------------------------------------------------
const Sprite& Animation::getPreviousFrameSprite() const
{
   return m_frames[previousFrameIndex].getSprite();
};

// public ----------------------------------------------------------------------------------------------------
const std::string& Animation::getAnimationName() const
{
   return m_animationName;
};

// public ----------------------------------------------------------------------------------------------------
void Animation::setAnimationName(const std::string animationName)
{
   m_animationName = animationName;
};

// public ----------------------------------------------------------------------------------------------------
void Animation::displace(const int dx, const int dy)
{
   for (Frame& frame : m_frames)
   {
      frame.displace(dx, dy);
   }
};

// public ----------------------------------------------------------------------------------------------------
void Animation::setPlaying(const bool playing)
{
   m_playing = playing;
};

// public ----------------------------------------------------------------------------------------------------
const bool& Animation::isPlaying() const
{
   return m_playing;
};

// public ----------------------------------------------------------------------------------------------------
void Animation::addPixelToCurrentFrame(const Pixel pixel)
{
   m_frames[currentFrameIndex].getMutableSprite().addPixel(pixel);
};

// public ----------------------------------------------------------------------------------------------------
void Animation::setAllSpriteLayers(const int layer)
{
   for (Frame& frame : m_frames)
   {
      frame.getMutableSprite().setLayer(layer);
   }
};