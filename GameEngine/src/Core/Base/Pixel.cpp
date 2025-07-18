//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Pixel.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Stores all information needed to print a character to an ncurses window
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/Pixel.h"

// public ----------------------------------------------------------------------------------------------------
Pixel::Pixel()
{
   m_position        = Position();
   m_character       = ' ';
   m_backgroundColor = RGB(0, 0, 0);
   m_textColor       = RGB(1000, 1000, 1000);
   m_attributes      = A_NORMAL;
};

// public ----------------------------------------------------------------------------------------------------
Pixel::Pixel(const Position position, const wchar_t character)
{
   m_position        = position;
   m_character       = character;
   m_backgroundColor = RGB(0, 0, 0);
   m_textColor       = RGB(1000, 1000, 1000);
   m_attributes      = A_NORMAL;
};

// public ----------------------------------------------------------------------------------------------------
Pixel::Pixel(const Position position, const wchar_t character, const RGB textColor, const RGB backgroundColor)
{
   m_position        = position;
   m_character       = character;
   m_textColor       = textColor;
   m_backgroundColor = backgroundColor;
   m_attributes      = A_NORMAL;
};

// public ----------------------------------------------------------------------------------------------------
Pixel::Pixel(const Position position, const wchar_t character, const RGB textColor, const RGB backgroundColor,
             const attr_t attributes)
{
   m_position        = position;
   m_character       = character;
   m_textColor       = textColor;
   m_backgroundColor = backgroundColor;
   m_attributes      = attributes;
};

// public ----------------------------------------------------------------------------------------------------
void Pixel::setCharacter(const wchar_t character)
{
   m_character = character;
};

// public ----------------------------------------------------------------------------------------------------
void Pixel::setPosition(const Position position)
{
   m_position = position;
};

// public ----------------------------------------------------------------------------------------------------
const Position& Pixel::getPosition() const
{
   return m_position;
};

// public ----------------------------------------------------------------------------------------------------
const wchar_t& Pixel::getCharacter() const
{
   return m_character;
};

// public ----------------------------------------------------------------------------------------------------
void Pixel::displace(const int dx, const int dy)
{
   int newX   = m_position.getX() + dx;
   int newY   = m_position.getY() + dy;
   m_position = Position(newX, newY);
};

// public ----------------------------------------------------------------------------------------------------
void Pixel::setTextColor(const RGB textColor)
{
   m_textColor = textColor;
};

// public ----------------------------------------------------------------------------------------------------
void Pixel::setBackgroundColor(const RGB backgroundColor)
{
   m_backgroundColor = backgroundColor;
};

// public ----------------------------------------------------------------------------------------------------
const RGB& Pixel::getTextColor() const
{
   return m_textColor;
};

// public ----------------------------------------------------------------------------------------------------
const RGB& Pixel::getBackgroundColor() const
{
   return m_backgroundColor;
};

// public ----------------------------------------------------------------------------------------------------
const attr_t& Pixel::getAttributes() const
{
   return m_attributes;
};

// public ----------------------------------------------------------------------------------------------------
void Pixel::setAttributes(const attr_t attributes)
{
   m_attributes = attributes;
};