//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Sprite.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of Sprite class for pixel collection and positioning
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/Sprite.h"

// public ----------------------------------------------------------------------------------------------------
Sprite::Sprite()
{
   m_pixels = std::vector<Pixel>();
   m_layer  = 0;
   m_anchor = Position(0, 0);
};

// public ----------------------------------------------------------------------------------------------------
Sprite::Sprite(const std::vector<Pixel> pixels)
{
   m_pixels = pixels;
   m_layer  = 0;
   refreshAnchor();
};

// public ----------------------------------------------------------------------------------------------------
Sprite::Sprite(const std::vector<Pixel> pixels, const int layer)
{
   m_pixels = pixels;
   m_layer  = layer;
   refreshAnchor();
};

// public ----------------------------------------------------------------------------------------------------
void Sprite::addPixel(const Pixel pixel)
{
   m_pixels.emplace_back(pixel);
   int newX   = m_anchor.getX();
   int newY   = m_anchor.getY();
   int pixelX = pixel.getPosition().getX();
   int pixelY = pixel.getPosition().getY();
   if (pixelX < newX)
   {
      newX = pixelX;
   }
   if (pixelY < newY)
   {
      newY = pixelY;
   }

   m_anchor = Position(newX, newY);
};

// public ----------------------------------------------------------------------------------------------------
const std::vector<Pixel>& Sprite::getPixels() const
{
   return m_pixels;
};

// public ----------------------------------------------------------------------------------------------------
std::vector<Pixel>& Sprite::getPixelsMutable()
{
   return m_pixels;
};

// public ----------------------------------------------------------------------------------------------------
Pixel Sprite::getPixelCopyAtPosition(Position position)
{
   for (Pixel pixel : m_pixels)
   {
      int pixelX = pixel.getPosition().getX();
      int pixelY = pixel.getPosition().getY();
      if (pixelX == position.getX() && pixelY == position.getY())
      {
         return pixel;
      }
   }
   return Pixel(position, '\0');
}

// public ----------------------------------------------------------------------------------------------------
Pixel& Sprite::getPixelMutableAtPosition(Position position)
{
   for (Pixel& pixel : m_pixels)
   {
      int pixelX = pixel.getPosition().getX();
      int pixelY = pixel.getPosition().getY();
      if (pixelX == position.getX() && pixelY == position.getY())
      {
         return pixel;
      }
   }
   return m_pixels.at(0); // Return first pixel if no pixel found at position
};

// public ----------------------------------------------------------------------------------------------------
void Sprite::displace(const int dx, const int dy)
{
   for (Pixel& pixel : m_pixels)
   {
      pixel.displace(dx, dy);
   }
   m_anchor = Position(m_anchor.getX() + dx, m_anchor.getY() + dy);
};

// public ----------------------------------------------------------------------------------------------------
void Sprite::moveAnchorToPosition(const Position position)
{
   int dx = position.getX() - m_anchor.getX();
   int dy = position.getY() - m_anchor.getY();
   displace(dx, dy);
};

// private ---------------------------------------------------------------------------------------------------
void Sprite::refreshAnchor()
{
   int newX = m_anchor.getX();
   int newY = m_anchor.getY();
   for (const Pixel pixel : m_pixels)
   {
      int pixelX = pixel.getPosition().getX();
      int pixelY = pixel.getPosition().getY();
      if (pixelX < newX)
      {
         newX = pixelX;
      }
      if (pixelY < newY)
      {
         newY = pixelY;
      }
   }
   m_anchor = Position(newX, newY);
};

// public ----------------------------------------------------------------------------------------------------
const Position& Sprite::getAnchor() const
{
   return m_anchor;
};

// public ----------------------------------------------------------------------------------------------------
void Sprite::setAnchor(const Position anchor)
{
   m_anchor = anchor;
};

// public ----------------------------------------------------------------------------------------------------
const int& Sprite::getLayer() const
{
   return m_layer;
}

// public ----------------------------------------------------------------------------------------------------
void Sprite::setLayer(const int layer)
{
   m_layer = layer;
}

// public ----------------------------------------------------------------------------------------------------
bool Sprite::positionInBounds(Position position) const
{
   for (const Pixel pixel : m_pixels)
   {
      if (position.getX() == pixel.getPosition().getX() && position.getY() == pixel.getPosition().getY())
      {
         return true;
      }
   }
   return false;
}

// public ----------------------------------------------------------------------------------------------------
void Sprite::setPixels(const std::vector<Pixel> pixels)
{
   m_pixels = pixels;
   refreshAnchor();
};
