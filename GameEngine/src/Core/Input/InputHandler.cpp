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
   sliderDragging            = false;
   draggedSlider             = nullptr;
   windowDragging            = false;
   draggedWindow             = nullptr;
   dragOffset                = Position(0, 0);
   mouseEventProcessed       = false;
   contextsExplicitlyManaged = false;
   pressedButton             = nullptr;
   currentHoveredButton      = nullptr;
   currentClickedButton      = nullptr;
   currentSelectedButton     = nullptr;
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
void InputHandler::addContext(std::shared_ptr<NcursesWindow> window)
{
   contextsExplicitlyManaged = true;
   // Only add if not already in the list
   if (std::find(inFocusedWindows.begin(), inFocusedWindows.end(), window) == inFocusedWindows.end())
   {
      inFocusedWindows.push_back(window);
   }
}

// public ----------------------------------------------------------------------------------------------------
void InputHandler::removeContext(std::shared_ptr<NcursesWindow> window)
{
   contextsExplicitlyManaged = true;
   inFocusedWindows.erase(std::remove(inFocusedWindows.begin(), inFocusedWindows.end(), window),
                          inFocusedWindows.end());
}

// public ----------------------------------------------------------------------------------------------------
void InputHandler::clearContext()
{
   contextsExplicitlyManaged = true;
   inFocusedWindows.clear();
}

// private ---------------------------------------------------------------------------------------------------
bool InputHandler::isWindowInFocus(WINDOW* window) const
{
   // If contexts have never been explicitly managed, allow all interactions (backward compatibility)
   if (!contextsExplicitlyManaged)
   {
      return true;
   }

   // Check if the window is in the focused windows list
   for (const auto& focusedWindow : inFocusedWindows)
   {
      if (focusedWindow->getWindow() == window)
      {
         return true;
      }
   }

   return false;
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
            // Update button highlighting for mouse press
            updateButtonHighlighting(mousePosition, true);

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
            // Update button highlighting for mouse release
            updateButtonHighlighting(mousePosition, false);

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
            // Update button highlighting for mouse movement
            updateButtonHighlighting(mousePosition, false);

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
         // Check if the slider's window is in focus
         if (isWindowInFocus(slider->getNcurseWindow()))
         {
            sliderDragging = true;
            draggedSlider  = slider;
            slider->setPositionFromMouse(mousePosition);
            return true;
         }
      }
   }

   // Check if mouse is over any button
   for (auto& button : buttons)
   {
      if (button->mouseInBounds(mousePosition))
      {
         // Check if the button's window is in focus and the button has a function
         if (isWindowInFocus(button->getNcurseWindow()) && button->hasFunction())
         {
            pressedButton = button; // Record which button was pressed
            return true;
         }
      }
   }

   return false; // Mouse was not over any UI element
}

// private ---------------------------------------------------------------------------------------------------
void InputHandler::handleMouseRelease()
{
   sliderDragging = false;
   draggedSlider  = nullptr;

   // Execute button function if there was a pressed button and mouse is still over it
   if (pressedButton)
   {
      Position mousePosition = Position(event.x, event.y);
      if (pressedButton->mouseInBounds(mousePosition))
      {
         pressedButton->executeFunction();
      }
      pressedButton = nullptr;
   }
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
   inFocusedWindows.clear();
   contextsExplicitlyManaged = false;
   pressedButton             = nullptr;

   // Clear highlighting state
   if (currentHoveredButton && currentHoveredButton->isAutoHighlightEnabled())
   {
      currentHoveredButton->unhighlight();
   }
   if (currentClickedButton && currentClickedButton->isAutoHighlightEnabled())
   {
      currentClickedButton->unhighlight();
   }
   if (currentSelectedButton && currentSelectedButton->isAutoHighlightEnabled())
   {
      currentSelectedButton->unhighlight();
   }
   currentHoveredButton  = nullptr;
   currentClickedButton  = nullptr;
   currentSelectedButton = nullptr;
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

// public ----------------------------------------------------------------------------------------------------
void InputHandler::setSelectedButton(std::shared_ptr<Button> button)
{
   // Unhighlight previous selection
   if (currentSelectedButton && currentSelectedButton->isAutoHighlightEnabled())
   {
      currentSelectedButton->unhighlight();
   }

   currentSelectedButton = button;

   // Highlight new selection
   if (currentSelectedButton && currentSelectedButton->isAutoHighlightEnabled())
   {
      currentSelectedButton->highlight(currentSelectedButton->getSelectedColor());
   }
}

// public ----------------------------------------------------------------------------------------------------
std::shared_ptr<Button> InputHandler::getSelectedButton() const
{
   return currentSelectedButton;
}

// private ---------------------------------------------------------------------------------------------------
void InputHandler::updateButtonHighlighting(Position mousePosition, bool isMousePressed)
{
   std::shared_ptr<Button> buttonAtPosition = getButtonAtPosition(mousePosition);

   // Handle hover highlighting
   if (buttonAtPosition != currentHoveredButton)
   {
      // Unhighlight previously hovered button (if not clicked and not selected)
      if (currentHoveredButton && currentHoveredButton->isAutoHighlightEnabled() &&
          currentHoveredButton != currentClickedButton && currentHoveredButton != currentSelectedButton)
      {
         currentHoveredButton->unhighlight();
      }

      // Highlight new button (if not selected)
      currentHoveredButton = buttonAtPosition;
      if (currentHoveredButton && currentHoveredButton->isAutoHighlightEnabled() &&
          currentHoveredButton != currentClickedButton && currentHoveredButton != currentSelectedButton)
      {
         currentHoveredButton->highlight(currentHoveredButton->getHoverColor());
      }
   }

   // Handle click highlighting
   if (isMousePressed && buttonAtPosition)
   {
      // Unhighlight previously clicked button (if not selected)
      if (currentClickedButton && currentClickedButton->isAutoHighlightEnabled() &&
          currentClickedButton != buttonAtPosition && currentClickedButton != currentSelectedButton)
      {
         currentClickedButton->unhighlight();
      }

      // Highlight clicked button (if not selected)
      currentClickedButton = buttonAtPosition;
      if (currentClickedButton && currentClickedButton->isAutoHighlightEnabled() &&
          currentClickedButton != currentSelectedButton)
      {
         currentClickedButton->highlight(currentClickedButton->getClickColor());
      }
   }
   else if (!isMousePressed && currentClickedButton)
   {
      // Mouse released - unhighlight clicked button
      if (currentClickedButton->isAutoHighlightEnabled() && currentClickedButton != currentSelectedButton)
      {
         currentClickedButton->unhighlight();

         // Re-highlight if mouse is still over the button
         if (currentHoveredButton == currentClickedButton)
         {
            currentHoveredButton->highlight(currentHoveredButton->getHoverColor());
         }
      }
      currentClickedButton = nullptr;
   }
}

// private ---------------------------------------------------------------------------------------------------
std::shared_ptr<Button> InputHandler::getButtonAtPosition(Position position)
{
   for (auto& button : buttons)
   {
      if (button && button->isAutoHighlightEnabled() && button->isVisable() && button->mouseInBounds(position))
      {
         // Check if the button's window is in focus and the button has a function
         if (isWindowInFocus(button->getNcurseWindow()) && button->hasFunction())
         {
            return button;
         }
      }
   }
   return nullptr;
}