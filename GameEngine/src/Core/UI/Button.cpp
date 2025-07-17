//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Button.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of Button class with executable function capabilities
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/Button.h"
#include <climits>

Button::~Button() = default;

// public ----------------------------------------------------------------------------------------------------
Button::Button()
{
   m_printableName = "default";
   m_lockPosition  = ScreenLockPosition::NONE;

   m_currentAnimationName = "default";
   m_animations.push_back(Animation());
   m_visable          = false;
   m_moveableByCamera = true;
   m_minPosition      = Position(0, 0);
   m_maxPosition      = Position(0, 0);
   m_ncurseWindow     = nullptr;
   m_isHighlighted    = false;
}

// public ----------------------------------------------------------------------------------------------------
Button::Button(const std::string printableName, const std::vector<Animation> animations, const bool visable,
               const bool moveableByCamera, std::function<void()> function)
{
   m_printableName        = printableName;
   m_animations           = animations;
   m_currentAnimationName = m_animations.at(0).getAnimationName();
   m_visable              = visable;
   m_moveableByCamera     = moveableByCamera;
   m_lockPosition         = ScreenLockPosition::NONE;
   m_minPosition          = Position(0, 0);
   m_maxPosition          = Position(0, 0);
   setFunction(function);
   setPositions();
   m_ncurseWindow  = nullptr;
   m_isHighlighted = false;

   // Store original background colors
   storeOriginalColors();
}

// public ----------------------------------------------------------------------------------------------------
void Button::setFunction(std::function<void()> func)
{
   m_function = std::move(func);
}

// public ----------------------------------------------------------------------------------------------------
void Button::executeFunction()
{
   if (m_function)
   {
      m_function();
   }
   else
   {
      return;
   }
}

// public ----------------------------------------------------------------------------------------------------
bool Button::mouseInBounds(Position position)
{
   // If button is associated with a window, adjust mouse coordinates to be relative to that window
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
void Button::setText(std::string text)
{
   Animation& animation = getCurrentAnimationMutable();
   Sprite&    sprite    = animation.getCurrentFrameSpriteMutable();

   wchar_t topLeftCorner = L'+', topRightCorner = L'+', bottomLeftCorner = L'+', bottomRightCorner = L'+';
   wchar_t topEdge = L'-', bottomEdge = L'-', leftEdge = L'|', rightEdge = L'|';

   const std::vector<Pixel>& currentPixels = sprite.getPixels();
   int                       minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;

   if (!currentPixels.empty())
   {
      for (const Pixel& pixel : currentPixels)
      {
         int x = pixel.getPosition().getX();
         int y = pixel.getPosition().getY();
         if (x < minX)
            minX = x;
         if (x > maxX)
            maxX = x;
         if (y < minY)
            minY = y;
         if (y > maxY)
            maxY = y;
      }

      for (const Pixel& pixel : currentPixels)
      {
         int     x = pixel.getPosition().getX();
         int     y = pixel.getPosition().getY();
         wchar_t c = pixel.getCharacter();

         if (y == minY)
         {
            if (x == minX)
               topLeftCorner = c;
            else if (x == maxX)
               topRightCorner = c;
            else
               topEdge = c;
         }
         else if (y == maxY)
         {
            if (x == minX)
               bottomLeftCorner = c;
            else if (x == maxX)
               bottomRightCorner = c;
            else
               bottomEdge = c;
         }
         else
         {
            if (x == minX)
               leftEdge = c;
            else if (x == maxX)
               rightEdge = c;
         }
      }
   }

   std::vector<std::string> lines;
   std::string              currentLine;
   for (char c : text)
   {
      if (c == '\n')
      {
         lines.push_back(currentLine);
         currentLine.clear();
      }
      else
      {
         currentLine += c;
      }
   }
   lines.push_back(currentLine);

   size_t maxLineLength = 0;
   for (const std::string& line : lines)
   {
      if (line.length() > maxLineLength)
      {
         maxLineLength = line.length();
      }
   }

   size_t borderLength = std::max(maxLineLength + 4, size_t(6));

   std::vector<Pixel> newPixels;

   std::wstring topBorder = std::wstring(1, topLeftCorner) + std::wstring(borderLength - 2, topEdge) +
                            std::wstring(1, topRightCorner);
   for (size_t x = 0; x < topBorder.length(); x++)
   {
      newPixels.push_back(Pixel(Position(static_cast<int>(x), 0), topBorder[x]));
   }

   for (size_t y = 0; y < lines.size(); y++)
   {
      std::string line = lines[y];

      size_t textLength     = line.length();
      size_t availableSpace = borderLength - 2;
      size_t padding        = (availableSpace > textLength) ? (availableSpace - textLength) : 0;

      std::wstring paddedLine = std::wstring(1, leftEdge);

      for (char c : line)
      {
         paddedLine += static_cast<wchar_t>(c);
      }

      paddedLine += std::wstring(padding, L' ');
      paddedLine += std::wstring(1, rightEdge);

      for (size_t x = 0; x < paddedLine.length(); x++)
      {
         newPixels.push_back(Pixel(Position(static_cast<int>(x), static_cast<int>(y + 1)), paddedLine[x]));
      }
   }

   std::wstring bottomBorder = std::wstring(1, bottomLeftCorner) +
                               std::wstring(borderLength - 2, bottomEdge) + std::wstring(1, bottomRightCorner);
   for (size_t x = 0; x < bottomBorder.length(); x++)
   {
      newPixels.push_back(
            Pixel(Position(static_cast<int>(x), static_cast<int>(lines.size() + 1)), bottomBorder[x]));
   }

   sprite.setPixels(newPixels);
   sprite.setAnchor(Position(0, 0));

   // Store original colors after text change
   storeOriginalColors();

   setPositions();
}

// public ----------------------------------------------------------------------------------------------------
void Button::highlight(RGB rgbValue)
{
   if (m_isHighlighted)
   {
      unhighlight();
   }

   Animation&          animation = getCurrentAnimationMutable();
   Sprite&             sprite    = animation.getCurrentFrameSpriteMutable();
   std::vector<Pixel>& pixels    = sprite.getPixelsMutable();

   // Store original colors and positions if not already stored
   if (m_originalBackgroundColors.empty())
   {
      storeOriginalColors();
   }

   // Change all pixels' background colors to the specified RGB value
   for (Pixel& pixel : pixels)
   {
      pixel.setBackgroundColor(rgbValue);
   }

   m_isHighlighted = true;
}

// public ----------------------------------------------------------------------------------------------------
void Button::unhighlight()
{
   if (!m_isHighlighted || m_originalBackgroundColors.empty())
   {
      return;
   }

   Animation&          animation = getCurrentAnimationMutable();
   Sprite&             sprite    = animation.getCurrentFrameSpriteMutable();
   std::vector<Pixel>& pixels    = sprite.getPixelsMutable();

   // Restore original background colors
   for (size_t i = 0; i < pixels.size() && i < m_originalBackgroundColors.size(); i++)
   {
      pixels[i].setBackgroundColor(m_originalBackgroundColors[i]);
   }

   m_isHighlighted = false;
}

// private ---------------------------------------------------------------------------------------------------
void Button::storeOriginalColors()
{
   const Animation&          animation = getCurrentAnimation();
   const Sprite&             sprite    = animation.getCurrentFrameSprite();
   const std::vector<Pixel>& pixels    = sprite.getPixels();

   m_originalBackgroundColors.clear();
   m_originalPositions.clear();

   for (const Pixel& pixel : pixels)
   {
      m_originalBackgroundColors.push_back(pixel.getBackgroundColor());
      m_originalPositions.push_back(pixel.getPosition());
   }
}