#include "../../../include/Slider.h"
#include "../../../include/Animation.h"
#include "../../../include/Frame.h"
#include "../../../include/Pixel.h"
#include "../../../include/Sprite.h"
#include <algorithm>

Slider::Slider(int length, bool horizontal)
   : UIElement(), m_length(std::max(2, length)), m_position(0), m_horizontal(horizontal)
{
   m_printableName    = "defaultSlider";
   m_lockPosition     = ScreenLockPosition::NONE;
   m_stackDirection   = StackDirection::VERTICAL;
   m_visable          = true;
   m_moveableByCamera = false;
   updateSprite();
}

void Slider::setLength(int length)
{
   m_length   = std::max(2, length);
   m_position = std::clamp(m_position, 0, m_length - 1);
   updateSprite();
}

int Slider::getLength() const
{
   return m_length;
}

void Slider::setPosition(int pos)
{
   m_position = std::clamp(pos, 0, m_length - 1);
   updateSprite();
}

int Slider::getPosition() const
{
   return m_position;
}
 
void Slider::moveLeft()
{
   setPosition(m_position - 1);
}

void Slider::moveRight()
{
   setPosition(m_position + 1);
}

float Slider::getValue() const
{
   if (m_length <= 1)
      return 0.0f;
   return float(m_position) / float(m_length - 1);
}

void Slider::setAnimation(const Animation& animation)
{
   m_animations.clear();
   m_animations.push_back(animation);
   m_currentAnimationName = animation.getAnimationName();
   // User is now responsible for updating the animation/sprite
}

bool Slider::mouseInBounds(Position position)
{
   return getCurrentAnimation().getCurrentFrameSprite().positionInBounds(position);
}

void Slider::setPositionFromMouse(Position mousePosition)
{
   // Get the slider's current sprite to determine bounds
   const Sprite& sprite = getCurrentAnimation().getCurrentFrameSprite();
   Position anchor = sprite.getAnchor();
   
   if (m_horizontal)
   {
      // For horizontal slider, use X coordinate
      int relativeX = mousePosition.getX() - anchor.getX();
      int newPosition = std::clamp(relativeX, 0, m_length - 1);
      setPosition(newPosition);
   }
   else
   {
      // For vertical slider, use Y coordinate
      int relativeY = mousePosition.getY() - anchor.getY();
      int newPosition = std::clamp(relativeY, 0, m_length - 1);
      setPosition(newPosition);
   }
}

void Slider::updateSprite()
{
   // Save current anchor position before updating sprite
   Position prevAnchor =
         m_animations.empty() ? Position(0, 0) : m_animations.front().getCurrentFrameSprite().getAnchor();

   std::vector<Pixel> pixels;
   if (m_horizontal)
   {
      // Draw bar: [----|-----]
      for (int i = 0; i < m_length; ++i)
      {
         char ch = (i == m_position) ? '|' : '-';
         pixels.push_back(Pixel(Position(i, 0), ch));
      }
   }
   else
   {
      // Draw vertical bar: handle is '|', rest are '-'
      for (int i = 0; i < m_length; ++i)
      {
         char ch = (i == m_position) ? '|' : '-';
         pixels.push_back(Pixel(Position(0, i), ch));
      }
   }
   Sprite sprite(pixels);
   // Restore anchor position
   sprite.moveAnchorToPosition(prevAnchor);

   Animation anim("default", {Frame(sprite, 1.0f)}, true);
   m_animations.clear();
   m_animations.push_back(anim);
   m_currentAnimationName = "default";
}
