//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file InputHandler.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of input processing for UI elements in the GameEngine
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/InputHandler.h"
#include "../../../include/NcursesWindow.h"
#include "../../../include/Parameters.h"
#include <ncurses.h>
#include <cstring>

// public ----------------------------------------------------------------------------------------------------
InputHandler::InputHandler()
{
   memset(&event, 0, sizeof(event));
   sliderDragging      = false;
   draggedSlider       = nullptr;
   windowDragging      = false;
   draggedWindow       = nullptr;
   dragOffset          = Position(0, 0);
   mouseEventProcessed = false;
}

// public ----------------------------------------------------------------------------------------------------
void InputHandler::addButton(std::shared_ptr<Button> button)
{
   buttons.push_back(button);
}

// public ----------------------------------------------------------------------------------------------------
void InputHandler::addSlider(std::shared_ptr<Slider> slider)
{
   sliders.push_back(slider);
}

// public ----------------------------------------------------------------------------------------------------
void InputHandler::removeButton(std::shared_ptr<Button> button)
{
   buttons.erase(std::remove(buttons.begin(), buttons.end(), button), buttons.end());
}

// public ----------------------------------------------------------------------------------------------------
void InputHandler::removeSlider(std::shared_ptr<Slider> slider)
{
   sliders.erase(std::remove(sliders.begin(), sliders.end(), slider), sliders.end());
}

// public ----------------------------------------------------------------------------------------------------
bool InputHandler::processInput(int userInput)
{
   mouseEventProcessed = false; // Reset flag at start of processing

   // Handle mouse input
   if (userInput == KEY_MOUSE)
   {
      if (getmouse(&event) == OK)
      {
         mouseEventProcessed    = true; // Mark that we processed a mouse event
         Position mousePosition = Position(event.x, event.y);

         // Handle mouse button press - use separate if statements, not else if
         if (event.bstate & BUTTON1_PRESSED)
         {
            // Check for window dragging first (takes precedence over UI elements)
            if (handleWindowPress(mousePosition))
            {
               return true;
            }

            if (handleMousePress(mousePosition))
            {
               return true;
            }
         }

         // Handle mouse button release - use separate if statements, not else if
         if (event.bstate & BUTTON1_RELEASED)
         {
            handleWindowRelease();
            handleMouseRelease();
         }

         // Handle mouse click events (additional safety checks)
         if (event.bstate & (BUTTON1_RELEASED | BUTTON1_CLICKED))
         {
            handleWindowRelease();
            handleMouseRelease();
         }

         // Handle mouse movement/dragging - use separate if statements, not else if
         if (event.bstate & REPORT_MOUSE_POSITION)
         {
            handleWindowDrag(mousePosition);
            handleMouseDrag(mousePosition);
         }
      }
   }

   return false; // Input was not handled by UI elements
}

// private ---------------------------------------------------------------------------------------------------
bool InputHandler::handleMousePress(Position mousePosition)
{
   // Check if mouse is over any slider
   for (auto& slider : sliders)
   {
      if (slider->mouseInBounds(mousePosition))
      {
         sliderDragging = true;
         draggedSlider  = slider;
         slider->setPositionFromMouse(mousePosition);
         return true;
      }
   }

   // Check if mouse is over any button
   for (auto& button : buttons)
   {
      if (button->mouseInBounds(mousePosition))
      {
         button->executeFunction();
         return true;
      }
   }

   return false; // Mouse was not over any UI element
}

// private ---------------------------------------------------------------------------------------------------
void InputHandler::handleMouseRelease()
{
   sliderDragging = false;
   draggedSlider  = nullptr;
}

// private ---------------------------------------------------------------------------------------------------
void InputHandler::handleMouseDrag(Position mousePosition)
{
   if (sliderDragging && draggedSlider)
   {
      // Update the specific slider that's being dragged
      draggedSlider->setPositionFromMouse(mousePosition);
   }
}

// public ----------------------------------------------------------------------------------------------------
void InputHandler::clear()
{
   buttons.clear();
   sliders.clear();
   sliderDragging = false;
   draggedSlider  = nullptr;
   windowDragging = false;
   draggedWindow  = nullptr;
}

// public ----------------------------------------------------------------------------------------------------
bool InputHandler::isMouseOverUI(Position mousePos)
{
   for (auto& button : buttons)
   {
      if (button->mouseInBounds(mousePos))
         return true;
   }
   for (auto& slider : sliders)
   {
      if (slider->mouseInBounds(mousePos))
         return true;
   }
   return false;
}

// public ----------------------------------------------------------------------------------------------------
MEVENT* InputHandler::getLastMouseEvent()
{
   return mouseEventProcessed ? &event : nullptr;
}

// public ----------------------------------------------------------------------------------------------------
bool InputHandler::wasMouseEventProcessed()
{
   return mouseEventProcessed;
}

// private ---------------------------------------------------------------------------------------------------
bool InputHandler::handleWindowPress(Position mousePosition)
{
   // Check if mouse is over any draggable window border
   for (auto& window : ncursesWindows)
   {
      if (window->isDraggable() && window->isMouseOnBorder(mousePosition))
      {
         windowDragging = true;
         draggedWindow  = window;

         // Calculate offset from mouse position to window position
         int windowX, windowY;
         getbegyx(window->getWindow(), windowY, windowX);
         dragOffset = Position(mousePosition.getX() - windowX, mousePosition.getY() - windowY);

         return true;
      }
   }
   return false;
}

// private ---------------------------------------------------------------------------------------------------
void InputHandler::handleWindowRelease()
{
   windowDragging = false;
   draggedWindow  = nullptr;
}

// private ---------------------------------------------------------------------------------------------------
void InputHandler::handleWindowDrag(Position mousePosition)
{
   if (windowDragging && draggedWindow)
   {
      // Calculate new window position based on mouse position and offset
      int newX = mousePosition.getX() - dragOffset.getX();
      int newY = mousePosition.getY() - dragOffset.getY();

      // Get window dimensions
      int windowHeight, windowWidth;
      getmaxyx(draggedWindow->getWindow(), windowHeight, windowWidth);

      // Ensure window stays within screen bounds
      newX = std::max(0, std::min(newX, SCREEN_LENGTH - windowWidth));
      newY = std::max(0, std::min(newY, SCREEN_HEIGHT - windowHeight));

      draggedWindow->setBasePosition(newX, newY);
   }
}