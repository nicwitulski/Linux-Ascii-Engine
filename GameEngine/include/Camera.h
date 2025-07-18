//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Camera.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Defines a camera which is a rectangle in the ncurses display. Has an offset which gets used to
/// print camera-movable objects at their world coordinates + the camera offset to give the illusion of
/// moving in the world.
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CAMERA_H
#define CAMERA_H

#include "Position.h"
#include <cstdint>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class Camera
///
/// Defines a camera which is a rectangle in the ncurses display. Has an offset which gets used to
/// print camera-movable objects at their world coordinates + the camera offset to give the illusion of
/// moving in the world.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Camera
{
private:
   int m_length;
   int m_height;
   int m_lengthOffset;
   int m_heightOffset;
   int m_lastLengthOffset;
   int m_lastHeightOffset;

public:
   Camera();
   Camera(const int length, const int height);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn displaceViewPort
   ///
   /// @param dx - X axis difference
   /// @param dy - Y axis difference
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void displaceViewPort(const int dx, const int dy);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn getLength
   ///
   /// @return The length of the camera's viewport
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   int getLength() const;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn setLength
   ///
   /// @param length - Sets the viewport length
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void setLength(const int length);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn getHeight
   ///
   /// @return The height of the camera's viewport
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   int getHeight() const;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn setHeight
   ///
   /// @param height - Sets the viewport height
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void setHeight(const int height);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn getLengthOffset
   ///
   /// @return The camera's viewport length offset
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   int getLengthOffset() const;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn getHeightOffset
   ///
   /// @return The camera's viewport height offset
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   int getHeightOffset() const;
};

#endif