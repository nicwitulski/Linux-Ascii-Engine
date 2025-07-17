//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Camera.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of Camera class for viewport management and offset tracking
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/Camera.h"
#include "../../../include/Parameters.h"

// public ----------------------------------------------------------------------------------------------------
Camera::Camera()
{
   m_length           = 80;
   m_height           = 24;
   m_lengthOffset     = 0;
   m_heightOffset     = 0;
   m_lastLengthOffset = 0;
   m_lastHeightOffset = 0;
};

// public ----------------------------------------------------------------------------------------------------
Camera::Camera(const int length, const int height)
{
   m_length           = length;
   m_height           = height;
   m_lengthOffset     = 0;
   m_heightOffset     = 0;
   m_lastLengthOffset = 0;
   m_lastHeightOffset = 0;
};

// public ----------------------------------------------------------------------------------------------------
void Camera::displaceViewPort(const int dx, const int dy)
{
   if (dx != 0 || dy != 0)
   {
      displayNeedsCleared = true;
   }
   m_lengthOffset += dx;
   m_heightOffset += dy;
};

// public ----------------------------------------------------------------------------------------------------
int Camera::getLength() const
{
   return m_length;
};

// public ----------------------------------------------------------------------------------------------------
void Camera::setLength(const int length)
{
   m_length = length;
};

// public ----------------------------------------------------------------------------------------------------
int Camera::getHeight() const
{
   return m_height;
};

// public ----------------------------------------------------------------------------------------------------
void Camera::setHeight(const int height)
{
   m_height = height;
};

// public ----------------------------------------------------------------------------------------------------
int Camera::getHeightOffset() const
{
   return m_heightOffset;
};

// public ----------------------------------------------------------------------------------------------------
int Camera::getLengthOffset() const
{
   return m_lengthOffset;
};