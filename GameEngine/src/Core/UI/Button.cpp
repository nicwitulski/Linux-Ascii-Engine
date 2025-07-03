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
#include <climits>

Button::~Button() = default;

// public ----------------------------------------------------------------------------------------------------
Button::Button()
{
   m_printableName = "default";
   m_lockPosition  = ScreenLockPosition::NONE;

   m_currentAnimationName = "default";
   m_animations.push_back(Animation());
   m_visable                 = false;
   m_moveableByCamera        = true;
   Sprite m_spriteBeforeMove = Sprite();
   m_minPosition             = Position(0, 0);
   m_maxPosition             = Position(0, 0);
};

// public ----------------------------------------------------------------------------------------------------
Button::Button(const std::string printableName, const std::vector<Animation> animations, const bool visable,
               const bool moveableByCamera, std::function<void()> function)
{
   m_printableName           = printableName;
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

// public ----------------------------------------------------------------------------------------------------
void Button::setText(std::string text)
{
   // Get the current animation and sprite
   Animation& animation = getCurrentAnimationMutable();
   Sprite& sprite = animation.getCurrentFrameSpriteMutable();
   
   // Detect border style from current sprite
   wchar_t topLeftCorner = L'+', topRightCorner = L'+', bottomLeftCorner = L'+', bottomRightCorner = L'+';
   wchar_t topEdge = L'-', bottomEdge = L'-', leftEdge = L'|', rightEdge = L'|';
   
   const std::vector<Pixel>& currentPixels = sprite.getPixels();
   int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;
   
   if (!currentPixels.empty()) {
       // Find the bounds of the current sprite
       for (const Pixel& pixel : currentPixels) {
           int x = pixel.getPosition().getX();
           int y = pixel.getPosition().getY();
           if (x < minX) minX = x;
           if (x > maxX) maxX = x;
           if (y < minY) minY = y;
           if (y > maxY) maxY = y;
       }
       
       // Extract border characters from current sprite
       for (const Pixel& pixel : currentPixels) {
           int x = pixel.getPosition().getX();
           int y = pixel.getPosition().getY();
           wchar_t c = pixel.getCharacter();
           
           if (y == minY) { // Top row
               if (x == minX) topLeftCorner = c;
               else if (x == maxX) topRightCorner = c;
               else topEdge = c;
           } else if (y == maxY) { // Bottom row
               if (x == minX) bottomLeftCorner = c;
               else if (x == maxX) bottomRightCorner = c;
               else bottomEdge = c;
           } else { // Middle rows
               if (x == minX) leftEdge = c;
               else if (x == maxX) rightEdge = c;
           }
       }
   }
   
   // Split text into lines based on newline characters
   std::vector<std::string> lines;
   std::string currentLine;
   for (char c : text) {
       if (c == '\n') {
           lines.push_back(currentLine);
           currentLine.clear();
       } else {
           currentLine += c;
       }
   }
   lines.push_back(currentLine); // Add the last line
   
   // Find the longest line to determine border width
   size_t maxLineLength = 0;
   for (const std::string& line : lines) {
       if (line.length() > maxLineLength) {
           maxLineLength = line.length();
       }
   }
   
   // Ensure minimum border length and prevent overflow
   size_t borderLength = std::max(maxLineLength + 4, size_t(6)); // Minimum 6 characters wide
   
   // Create the new button sprite
   std::vector<Pixel> newPixels;
   
   // Top border
   std::wstring topBorder = std::wstring(1, topLeftCorner) + std::wstring(borderLength - 2, topEdge) + std::wstring(1, topRightCorner);
   for (size_t x = 0; x < topBorder.length(); x++) {
       newPixels.push_back(Pixel(Position(static_cast<int>(x), 0), topBorder[x]));
   }
   
   // Middle lines with text
   for (size_t y = 0; y < lines.size(); y++) {
       std::string line = lines[y];
       
       // Calculate padding safely
       size_t textLength = line.length();
       size_t availableSpace = borderLength - 2; // Account for left and right borders
       size_t padding = (availableSpace > textLength) ? (availableSpace - textLength) : 0;
       
       // Create the line with proper padding
       std::wstring paddedLine = std::wstring(1, leftEdge);
       
       // Add text with proper conversion
       for (char c : line) {
           paddedLine += static_cast<wchar_t>(c);
       }
       
       // Add padding spaces
       paddedLine += std::wstring(padding, L' ');
       paddedLine += std::wstring(1, rightEdge);
       
       for (size_t x = 0; x < paddedLine.length(); x++) {
           newPixels.push_back(Pixel(Position(static_cast<int>(x), static_cast<int>(y + 1)), paddedLine[x]));
       }
   }
   
   // Bottom border
   std::wstring bottomBorder = std::wstring(1, bottomLeftCorner) + std::wstring(borderLength - 2, bottomEdge) + std::wstring(1, bottomRightCorner);
   for (size_t x = 0; x < bottomBorder.length(); x++) {
       newPixels.push_back(Pixel(Position(static_cast<int>(x), static_cast<int>(lines.size() + 1)), bottomBorder[x]));
   }
   
   // Update the sprite with new pixels
   sprite.setPixels(newPixels);
   sprite.setAnchor(Position(0, 0));
   
   // Update the button's position bounds
   setPositions();
};