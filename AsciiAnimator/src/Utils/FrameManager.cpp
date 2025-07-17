//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file FrameManager.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of FrameManager class for frame navigation, creation, and duration editing
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "FrameManager.h"
#include "../../../GameEngine/include/Entity.h"
#include "../../../GameEngine/include/RGB.h"
#include "../../../GameEngine/include/Sprite.h"

// public ----------------------------------------------------------------------------------------------------
FrameManager::FrameManager() {}

// public ----------------------------------------------------------------------------------------------------
FrameManager::FrameManager(std::shared_ptr<Entity> visible, std::shared_ptr<Entity> drawn,
                           std::shared_ptr<Entity> greyed)
   : visibleEntity(visible), drawnEntity(drawn), greyedBackgroundEntity(greyed)
{
}

// public ----------------------------------------------------------------------------------------------------
void FrameManager::setEntities(std::shared_ptr<Entity> visible, std::shared_ptr<Entity> drawn,
                               std::shared_ptr<Entity> greyed)
{
   visibleEntity          = visible;
   drawnEntity            = drawn;
   greyedBackgroundEntity = greyed;
}

// public ----------------------------------------------------------------------------------------------------
void FrameManager::nextFrame()
{
   syncFrameToDrawnEntity();

   if (hasNextFrame())
   {
      const Frame& currentFrame = visibleEntity->getCurrentAnimation().getFrameAtIndex(getCurrentFrameIndex());
      visibleEntity->getCurrentAnimationMutable().manuallyIncrementFrame();
      drawnEntity->getCurrentAnimationMutable().manuallyIncrementFrame();
      setGreyedBackground(currentFrame);
   }
   else
   {
      createNewFrame();
      const Frame& currentFrame = visibleEntity->getCurrentAnimation().getFrameAtIndex(getCurrentFrameIndex());
      visibleEntity->getCurrentAnimationMutable().manuallyIncrementFrame();
      drawnEntity->getCurrentAnimationMutable().manuallyIncrementFrame();
      setGreyedBackground(currentFrame);
   }
}

// public ----------------------------------------------------------------------------------------------------
void FrameManager::previousFrame()
{
   syncFrameToDrawnEntity();

   if (hasPreviousFrame())
   {
      visibleEntity->getCurrentAnimationMutable().manuallyDecrementFrame();
      drawnEntity->getCurrentAnimationMutable().manuallyDecrementFrame();

      size_t currentIndex = getCurrentFrameIndex();
      if (currentIndex > 0)
      {
         const Frame& frameBefore = visibleEntity->getCurrentAnimation().getFrameAtIndex(currentIndex - 1);
         setGreyedBackground(frameBefore);
      }
      else
      {
         clearGreyedBackground();
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void FrameManager::syncFrameToDrawnEntity()
{
   size_t       currentIndex        = getCurrentFrameIndex();
   const Frame& currentVisibleFrame = visibleEntity->getCurrentAnimation().getFrameAtIndex(currentIndex);
   drawnEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(currentIndex) = currentVisibleFrame;
}

// public ----------------------------------------------------------------------------------------------------
void FrameManager::createNewFrame()
{
   Frame newFrame(Sprite(), 1.0f);
   visibleEntity->getCurrentAnimationMutable().addFrame(newFrame);
   drawnEntity->getCurrentAnimationMutable().addFrame(newFrame);
}

// public ----------------------------------------------------------------------------------------------------
void FrameManager::setGreyedBackground(const Frame& sourceFrame)
{
   Frame greyedFrame = createGreyedOutFrame(sourceFrame);
   greyedBackgroundEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(0) = greyedFrame;
   greyedBackgroundEntity->setVisability(true);
}

// public ----------------------------------------------------------------------------------------------------
void FrameManager::clearGreyedBackground()
{
   greyedBackgroundEntity->setVisability(false);
}

// public ----------------------------------------------------------------------------------------------------
void FrameManager::setFrameDuration(float duration)
{
   size_t currentIndex = getCurrentFrameIndex();

   Frame& currentVisibleFrame =
         visibleEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(currentIndex);
   Frame& currentDrawnFrame = drawnEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(currentIndex);

   Frame newVisibleFrame(currentVisibleFrame.getSprite(), duration);
   Frame newDrawnFrame(currentDrawnFrame.getSprite(), duration);

   visibleEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(currentIndex) = newVisibleFrame;
   drawnEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(currentIndex)   = newDrawnFrame;
}

// public ----------------------------------------------------------------------------------------------------
float FrameManager::getCurrentFrameDuration() const
{
   size_t currentIndex = getCurrentFrameIndex();
   return visibleEntity->getCurrentAnimation().getFrameAtIndex(currentIndex).getDuration();
}

// public ----------------------------------------------------------------------------------------------------
bool FrameManager::hasNextFrame() const
{
   return visibleEntity->getCurrentAnimation().hasNextFrame();
}

// public ----------------------------------------------------------------------------------------------------
bool FrameManager::hasPreviousFrame() const
{
   return visibleEntity->getCurrentAnimation().hasPreviousFrame();
}

// public ----------------------------------------------------------------------------------------------------
size_t FrameManager::getCurrentFrameIndex() const
{
   return visibleEntity->getCurrentAnimation().getCurrentFrameIndex();
}

// public ----------------------------------------------------------------------------------------------------
bool FrameManager::frameHasContent() const
{
   const Sprite&             currentSprite = visibleEntity->getCurrentAnimation().getCurrentFrameSprite();
   const std::vector<Pixel>& pixels        = currentSprite.getPixels();

   for (const Pixel& pixel : pixels)
   {
      if (pixel.getCharacter() != ' ' && pixel.getCharacter() != '\0')
      {
         return true;
      }
      if (pixel.getBackgroundColor().getR() > 0 || pixel.getBackgroundColor().getG() > 0 ||
          pixel.getBackgroundColor().getB() > 0)
      {
         return true;
      }
   }
   return false;
}

// private ---------------------------------------------------------------------------------------------------
Frame FrameManager::createGreyedOutFrame(const Frame& sourceFrame) const
{
   const Sprite&             sourceSprite = sourceFrame.getSprite();
   const std::vector<Pixel>& sourcePixels = sourceSprite.getPixels();

   std::vector<Pixel> greyedPixels;
   RGB                greyBackground(750, 750, 750);
   RGB                greyText(250, 250, 250);

   for (const Pixel& pixel : sourcePixels)
   {
      RGB newBackground = (pixel.getBackgroundColor().getR() > 0 || pixel.getBackgroundColor().getG() > 0 ||
                           pixel.getBackgroundColor().getB() > 0) ?
                                greyBackground :
                                pixel.getBackgroundColor();

      Pixel greyedPixel(pixel.getPosition(), pixel.getCharacter(), greyText, newBackground,
                        pixel.getAttributes());
      greyedPixels.push_back(greyedPixel);
   }

   Sprite greyedSprite(greyedPixels, sourceSprite.getLayer());
   return Frame(greyedSprite, sourceFrame.getDuration());
}