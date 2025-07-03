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
#include <ncurses.h>

InputHandler::InputHandler()
{
   event = {0};
   sliderDragging = false;
   draggedSlider = nullptr;
   mouseEventProcessed = false;
}

void InputHandler::addButton(std::shared_ptr<Button> button)
{
   buttons.push_back(button);
}

void InputHandler::addSlider(std::shared_ptr<Slider> slider)
{
   sliders.push_back(slider);
}

bool InputHandler::processInput(int userInput)
{
   mouseEventProcessed = false; // Reset flag at start of processing
   
   // Handle mouse input
   if (userInput == KEY_MOUSE)
   {
      if (getmouse(&event) == OK)
      {
         mouseEventProcessed = true; // Mark that we processed a mouse event
         Position mousePosition = Position(event.x, event.y);

         // Handle mouse button press - use separate if statements, not else if
         if (event.bstate & BUTTON1_PRESSED)
         {
            if (handleMousePress(mousePosition))
            {
               return true;
            }
         }

         // Handle mouse button release - use separate if statements, not else if
         if (event.bstate & BUTTON1_RELEASED)
         {
            handleMouseRelease();
         }

         // Handle mouse click events (additional safety checks)
         if (event.bstate & (BUTTON1_RELEASED | BUTTON1_CLICKED))
         {
            handleMouseRelease();
         }

         // Handle mouse movement/dragging - use separate if statements, not else if
         if (event.bstate & REPORT_MOUSE_POSITION)
         {
            handleMouseDrag(mousePosition);
         }
      }
   }

   return false; // Input was not handled by UI elements
}

bool InputHandler::handleMousePress(Position mousePosition)
{
   // Check if mouse is over any slider
   for (auto& slider : sliders)
   {
      if (slider->mouseInBounds(mousePosition))
      {
         sliderDragging = true;
         draggedSlider = slider;
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

void InputHandler::handleMouseRelease()
{
   sliderDragging = false;
   draggedSlider = nullptr;
}

void InputHandler::handleMouseDrag(Position mousePosition)
{
   if (sliderDragging && draggedSlider)
   {
      // Update the specific slider that's being dragged
      draggedSlider->setPositionFromMouse(mousePosition);
   }
}

void InputHandler::clear()
{
   buttons.clear();
   sliders.clear();
   sliderDragging = false;
   draggedSlider = nullptr;
}

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

MEVENT* InputHandler::getLastMouseEvent()
{
   return mouseEventProcessed ? &event : nullptr;
}

bool InputHandler::wasMouseEventProcessed()
{
   return mouseEventProcessed;
} 