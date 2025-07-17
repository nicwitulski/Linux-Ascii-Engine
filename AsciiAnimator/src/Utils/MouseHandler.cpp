//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file MouseHandler.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of MouseHandler class for mouse interactions
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MouseHandler.h"
#include "../../../GameEngine/include/Parameters.h"

// public ----------------------------------------------------------------------------------------------------
MouseHandler::MouseHandler()
{
   cameraDrag        = false;
   drawing           = false;
   lastMousePosition = Position(-1, -1);
}

// public ----------------------------------------------------------------------------------------------------
bool MouseHandler::isCameraDragging() const
{
   return cameraDrag;
}

// public ----------------------------------------------------------------------------------------------------
bool MouseHandler::isDrawing() const
{
   return drawing;
}

// public ----------------------------------------------------------------------------------------------------
void MouseHandler::startCameraDrag(const Position& position)
{
   cameraDrag        = true;
   lastMousePosition = position;
}

// public ----------------------------------------------------------------------------------------------------
void MouseHandler::stopCameraDrag()
{
   cameraDrag        = false;
   lastMousePosition = Position(-1, -1);
}

// public ----------------------------------------------------------------------------------------------------
void MouseHandler::startDrawing()
{
   drawing = true;
}

// public ----------------------------------------------------------------------------------------------------
void MouseHandler::stopDrawing()
{
   drawing = false;
}

// public ----------------------------------------------------------------------------------------------------
void MouseHandler::updateCameraDrag(const Position& currentPosition)
{
   if (cameraDrag && currentCamera)
   {
      int dx = lastMousePosition.getX() - currentPosition.getX();
      int dy = lastMousePosition.getY() - currentPosition.getY();

      currentCamera->displaceViewPort(-dx, -dy);
      lastMousePosition = currentPosition;
   }
}

// public ----------------------------------------------------------------------------------------------------
Position MouseHandler::getWorldPosition(const Position& screenPosition) const
{
   if (currentCamera)
   {
      return Position(screenPosition.getX() - currentCamera->getLengthOffset(),
                      screenPosition.getY() - currentCamera->getHeightOffset());
   }
   return screenPosition;
}