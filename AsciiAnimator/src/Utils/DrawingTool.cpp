//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file DrawingTool.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of DrawingTool class for drawing, erasing, and brush size management
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "DrawingTool.h"
#include "../../../GameEngine/include/Entity.h"
#include <algorithm>

// public ----------------------------------------------------------------------------------------------------
DrawingTool::DrawingTool()
{
   drawingCharacter       = 'x';
   currentBrushSize       = 1;
   currentTextColor       = RGB(1000, 1000, 1000);
   currentBackgroundColor = RGB(0, 0, 0);
}

// public ----------------------------------------------------------------------------------------------------
void DrawingTool::setDrawingCharacter(char character)
{
   drawingCharacter = character;
}

// public ----------------------------------------------------------------------------------------------------
char DrawingTool::getDrawingCharacter() const
{
   return drawingCharacter;
}

// public ----------------------------------------------------------------------------------------------------
void DrawingTool::setBrushSize(int size)
{
   currentBrushSize = std::max(1, std::min(6, size));
}

// public ----------------------------------------------------------------------------------------------------
int DrawingTool::getBrushSize() const
{
   return currentBrushSize;
}

// public ----------------------------------------------------------------------------------------------------
void DrawingTool::setTextColor(const RGB& color)
{
   currentTextColor = color;
}

// public ----------------------------------------------------------------------------------------------------
const RGB& DrawingTool::getTextColor() const
{
   return currentTextColor;
}

// public ----------------------------------------------------------------------------------------------------
void DrawingTool::setBackgroundColor(const RGB& color)
{
   currentBackgroundColor = color;
}

// public ----------------------------------------------------------------------------------------------------
const RGB& DrawingTool::getBackgroundColor() const
{
   return currentBackgroundColor;
}

// public ----------------------------------------------------------------------------------------------------
void DrawingTool::drawAtPosition(std::shared_ptr<Entity> entity, int centerX, int centerY)
{
   int offset = (currentBrushSize - 1) / 2;

   for (int y = centerY - offset; y < centerY - offset + currentBrushSize; y++)
   {
      for (int x = centerX - offset; x < centerX - offset + currentBrushSize; x++)
      {
         Pixel newPixel = Pixel(Position(x, y), drawingCharacter, currentTextColor, currentBackgroundColor, 0);
         entity->getCurrentAnimationMutable().addPixelToCurrentFrame(newPixel);
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void DrawingTool::eraseAtPosition(std::shared_ptr<Entity> entity, int centerX, int centerY)
{
   int offset = (currentBrushSize - 1) / 2;

   std::vector<Pixel>& pixels =
         entity->getCurrentAnimationMutable().getCurrentFrameSpriteMutable().getPixelsMutable();

   for (int y = centerY - offset; y < centerY - offset + currentBrushSize; y++)
   {
      for (int x = centerX - offset; x < centerX - offset + currentBrushSize; x++)
      {
         Position erasePos(x, y);
         pixels.erase(std::remove_if(pixels.begin(), pixels.end(),
                                     [erasePos](const Pixel& pixel) {
                                        return pixel.getPosition().getX() == erasePos.getX() &&
                                               pixel.getPosition().getY() == erasePos.getY();
                                     }),
                      pixels.end());
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
bool DrawingTool::isErasing() const
{
   return drawingCharacter == ' ';
}