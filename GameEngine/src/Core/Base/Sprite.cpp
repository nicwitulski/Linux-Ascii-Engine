//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Sprite.cpp
/// @author Nicholas Witulski
/// @brief Holds a vector of Pixels to form a sprite
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