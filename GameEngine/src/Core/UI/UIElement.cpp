//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file UIElement.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of UIElement class with dynamic positioning capabilities
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/UIElement.h"
#include "../../../include/Parameters.h"
#include <map>
#include <set>

// Initialize static member variables
std::vector<std::shared_ptr<UIElement>> UIElement::topMiddleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::rightMiddleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::bottomMiddleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::leftMiddleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::middleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::topLeftUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::topRightUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::bottomLeftUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::bottomRightUIElements;

// public ----------------------------------------------------------------------------------------------------
UIElement::UIElement()
{
   m_printableName = "default";
   m_minPosition   = Position(0, 0);
   m_maxPosition   = Position(0, 0);
   m_lockPosition  = ScreenLockPosition::NONE;

   m_currentAnimationName = "default";
   m_animations.push_back(Animation());
   m_visable          = false;
   m_moveableByCamera = true;
   m_ncurseWindow     = nullptr;
};

// public ----------------------------------------------------------------------------------------------------
UIElement::UIElement(const std::string printableName, const std::vector<Animation> animations,
                     const bool visable, const bool moveableByCamera)
{
   m_printableName        = printableName;
   m_animations           = animations;
   m_currentAnimationName = m_animations.at(0).getAnimationName();
   m_visable              = visable;
   m_moveableByCamera     = moveableByCamera;
   m_lockPosition         = ScreenLockPosition::NONE;
   m_ncurseWindow         = nullptr;
   setPositions();
};

// private ---------------------------------------------------------------------------------------------------
void UIElement::setPositions()
{
   m_minPosition = getCurrentAnimation().getCurrentFrameSprite().getAnchor();
   int maxX      = 0;
   int maxY      = 0;

   for (Animation animation : getAnimations())
   {
      if (animation.getAnimationName() == getCurrentAnimationName())
      {
         for (const Pixel& pixel : animation.getCurrentFrameSprite().getPixels())
         {
            if (pixel.getPosition().getX() > maxX)
               maxX = pixel.getPosition().getX();
            if (pixel.getPosition().getY() > maxY)
               maxY = pixel.getPosition().getY();
         }
      }
   }

   m_maxPosition = Position(maxX, maxY);
};

// public ----------------------------------------------------------------------------------------------------
void UIElement::setDynamicPosition(ScreenLockPosition position, StackDirection direction)
{
   m_lockPosition   = position;
   m_stackDirection = direction;
   switch (m_lockPosition)
   {
      case ScreenLockPosition::TOP_MIDDLE:
         topMiddleUIElements.push_back(shared_from_this());
         break;
      case ScreenLockPosition::RIGHT_MIDDLE:
         rightMiddleUIElements.push_back(shared_from_this());
         break;
      case ScreenLockPosition::BOTTOM_MIDDLE:
         bottomMiddleUIElements.push_back(shared_from_this());
         break;
      case ScreenLockPosition::LEFT_MIDDLE:
         leftMiddleUIElements.push_back(shared_from_this());
         break;
      case ScreenLockPosition::CENTER:
         middleUIElements.push_back(shared_from_this());
         break;
      case ScreenLockPosition::TOP_LEFT_CORNER:
         topLeftUIElements.push_back(shared_from_this());
         break;
      case ScreenLockPosition::TOP_RIGHT_CORNER:
         topRightUIElements.push_back(shared_from_this());
         break;
      case ScreenLockPosition::BOTTOM_LEFT_CORNER:
         bottomLeftUIElements.push_back(shared_from_this());
         break;
      case ScreenLockPosition::BOTTOM_RIGHT_CORNER:
         bottomRightUIElements.push_back(shared_from_this());
         break;

      default:
         break;
   }
   m_stackDirection = direction;
};

// public ----------------------------------------------------------------------------------------------------
void UIElement::updateWindowLockedPositions(WINDOW* targetWindow)
{
   // Helper lambdas
   auto elementWidth = [](const std::shared_ptr<UIElement>& el)
   { return el->m_maxPosition.getX() - el->m_minPosition.getX() + 1; };
   auto elementHeight = [](const std::shared_ptr<UIElement>& el)
   { return el->m_maxPosition.getY() - el->m_minPosition.getY() + 1; };

   // Helper function to find NcursesWindow from WINDOW*
   auto findNcursesWindow = [](WINDOW* window) -> std::shared_ptr<NcursesWindow>
   {
      for (auto& ncursesWindow : ncursesWindows)
      {
         if (ncursesWindow->getWindow() == window)
         {
            return ncursesWindow;
         }
      }
      return nullptr;
   };

   // Filter elements to only process those associated with the target window
   auto filterWindowElements = [targetWindow](const std::vector<std::shared_ptr<UIElement>>& elements)
   {
      std::vector<std::shared_ptr<UIElement>> filtered;
      for (auto& el : elements)
      {
         WINDOW* window = el->getNcurseWindow();
         if ((!window && targetWindow == stdscr) || window == targetWindow)
         {
            filtered.push_back(el);
         }
      }
      return filtered;
   };

   // Filter each position group to only include target window elements
   auto filteredTopMiddle    = filterWindowElements(topMiddleUIElements);
   auto filteredRightMiddle  = filterWindowElements(rightMiddleUIElements);
   auto filteredBottomMiddle = filterWindowElements(bottomMiddleUIElements);
   auto filteredLeftMiddle   = filterWindowElements(leftMiddleUIElements);
   auto filteredMiddle       = filterWindowElements(middleUIElements);
   auto filteredTopLeft      = filterWindowElements(topLeftUIElements);
   auto filteredTopRight     = filterWindowElements(topRightUIElements);
   auto filteredBottomLeft   = filterWindowElements(bottomLeftUIElements);
   auto filteredBottomRight  = filterWindowElements(bottomRightUIElements);

   // Group filtered elements by their window
   std::map<WINDOW*, std::vector<std::shared_ptr<UIElement>>> windowGroups[9];

   // Helper function to add elements to window groups
   auto addToWindowGroup = [&](const std::vector<std::shared_ptr<UIElement>>& elements, int groupIndex)
   {
      for (auto& el : elements)
      {
         WINDOW* window = el->getNcurseWindow();
         if (!window)
            window = stdscr;
         windowGroups[groupIndex][window].push_back(el);
      }
   };

   // Group all filtered elements by window and position
   addToWindowGroup(filteredTopMiddle, 0);
   addToWindowGroup(filteredRightMiddle, 1);
   addToWindowGroup(filteredBottomMiddle, 2);
   addToWindowGroup(filteredLeftMiddle, 3);
   addToWindowGroup(filteredMiddle, 4);
   addToWindowGroup(filteredTopLeft, 5);
   addToWindowGroup(filteredTopRight, 6);
   addToWindowGroup(filteredBottomLeft, 7);
   addToWindowGroup(filteredBottomRight, 8);

   // Process each window
   for (int groupIndex = 0; groupIndex < 9; ++groupIndex)
   {
      for (auto& [window, elements] : windowGroups[groupIndex])
      {
         if (elements.empty())
            continue;

         // Get window dimensions and adjust for borders if enabled
         int windowHeight, windowLength;
         getmaxyx(window, windowHeight, windowLength);

         // Check if this window has borders and adjust usable area
         auto ncursesWindow = findNcursesWindow(window);
         int  borderPadding = 0;
         if (ncursesWindow && ncursesWindow->isBorderEnabled())
         {
            borderPadding = 1; // 1 character padding on each side for borders
            windowHeight -= 2; // Top and bottom border
            windowLength -= 2; // Left and right border
         }

         // Process elements based on their position type
         switch (groupIndex)
         {
            case 0: // TOP MIDDLE
            {
               int totalWidth = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalWidth += elementWidth(el);
               }
               int dx = (windowLength - totalWidth) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(dx, borderPadding));
                  el->setPositions();
                  dx += elementWidth(el);
               }
               break;
            }
            case 1: // RIGHT MIDDLE
            {
               int totalHeight = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalHeight += elementHeight(el);
               }
               int dy = (windowHeight - totalHeight) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  int x = windowLength - elementWidth(el) + borderPadding;
                  el->moveToPosition(Position(x, dy));
                  el->setPositions();
                  dy += elementHeight(el);
               }
               break;
            }
            case 2: // BOTTOM MIDDLE
            {
               int totalWidth = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalWidth += elementWidth(el);
               }
               int dx = (windowLength - totalWidth) / 2 + borderPadding;
               int y  = windowHeight - 1 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(dx, y - elementHeight(el) + 1));
                  el->setPositions();
                  dx += elementWidth(el);
               }
               break;
            }
            case 3: // LEFT MIDDLE
            {
               int totalHeight = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalHeight += elementHeight(el);
               }
               int dy = (windowHeight - totalHeight) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(borderPadding, dy));
                  el->setPositions();
                  dy += elementHeight(el);
               }
               break;
            }
            case 4: // CENTER
            {
               int totalWidth = 0;
               int maxHeight  = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalWidth += elementWidth(el);
                  if (elementHeight(el) > maxHeight)
                     maxHeight = elementHeight(el);
               }
               int dx = (windowLength - totalWidth) / 2 + borderPadding;
               int y  = (windowHeight - maxHeight) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(dx, y));
                  el->setPositions();
                  dx += elementWidth(el);
               }
               break;
            }
            case 5: // TOP LEFT CORNER
            {
               int dx = borderPadding;
               int dy = borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(dx, dy));
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     dx += elementWidth(el);
                  }
                  else
                  {
                     dy += elementHeight(el);
                  }
               }
               break;
            }
            case 6: // TOP RIGHT CORNER
            {
               int dx = windowLength + borderPadding;
               int dy = borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     dx -= elementWidth(el);
                     el->moveToPosition(Position(dx, dy));
                  }
                  else
                  {
                     el->moveToPosition(Position(dx - elementWidth(el), dy));
                     dy += elementHeight(el);
                  }

                  el->setPositions();
               }
               break;
            }
            case 7: // BOTTOM LEFT CORNER
            {
               int dx = borderPadding;
               int dy = windowHeight + borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     el->moveToPosition(Position(dx, dy - elementHeight(el)));
                     dx += elementWidth(el);
                  }
                  else
                  {
                     dy -= elementHeight(el);
                     el->moveToPosition(Position(dx, dy));
                  }

                  el->setPositions();
               }
               break;
            }
            case 8: // BOTTOM RIGHT CORNER
            {
               int dx = windowLength + borderPadding;
               int dy = windowHeight + borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     dx -= elementWidth(el);
                     el->moveToPosition(Position(dx, dy - elementHeight(el)));
                  }
                  else
                  {
                     dy -= elementHeight(el);
                     el->moveToPosition(Position(dx - elementWidth(el), dy));
                  }

                  el->setPositions();
               }
               break;
            }
         }
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void UIElement::updateAllLockedPositions()
{
   // Update all windows by iterating through ncursesWindows
   std::set<WINDOW*> processedWindows;

   // Always process stdscr first
   updateWindowLockedPositions(stdscr);
   processedWindows.insert(stdscr);

   // Process all other windows
   for (auto& ncursesWindow : ncursesWindows)
   {
      WINDOW* window = ncursesWindow->getWindow();
      if (processedWindows.find(window) == processedWindows.end())
      {
         updateWindowLockedPositions(window);
         processedWindows.insert(window);
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void UIElement::updateStdscrLockedPositions()
{
   // Helper lambdas
   auto elementWidth = [](const std::shared_ptr<UIElement>& el)
   { return el->m_maxPosition.getX() - el->m_minPosition.getX() + 1; };
   auto elementHeight = [](const std::shared_ptr<UIElement>& el)
   { return el->m_maxPosition.getY() - el->m_minPosition.getY() + 1; };

   // Helper function to find NcursesWindow from WINDOW*
   auto findNcursesWindow = [](WINDOW* window) -> std::shared_ptr<NcursesWindow>
   {
      for (auto& ncursesWindow : ncursesWindows)
      {
         if (ncursesWindow->getWindow() == window)
         {
            return ncursesWindow;
         }
      }
      return nullptr;
   };

   // Filter elements to only process those associated with stdscr
   auto filterStdscrElements = [](const std::vector<std::shared_ptr<UIElement>>& elements)
   {
      std::vector<std::shared_ptr<UIElement>> filtered;
      for (auto& el : elements)
      {
         WINDOW* window = el->getNcurseWindow();
         if (!window || window == stdscr)
         {
            filtered.push_back(el);
         }
      }
      return filtered;
   };

   // Filter each position group to only include stdscr elements
   auto filteredTopMiddle    = filterStdscrElements(topMiddleUIElements);
   auto filteredRightMiddle  = filterStdscrElements(rightMiddleUIElements);
   auto filteredBottomMiddle = filterStdscrElements(bottomMiddleUIElements);
   auto filteredLeftMiddle   = filterStdscrElements(leftMiddleUIElements);
   auto filteredMiddle       = filterStdscrElements(middleUIElements);
   auto filteredTopLeft      = filterStdscrElements(topLeftUIElements);
   auto filteredTopRight     = filterStdscrElements(topRightUIElements);
   auto filteredBottomLeft   = filterStdscrElements(bottomLeftUIElements);
   auto filteredBottomRight  = filterStdscrElements(bottomRightUIElements);

   // Group filtered elements by their window (should only be stdscr now)
   std::map<WINDOW*, std::vector<std::shared_ptr<UIElement>>> windowGroups[9];

   // Helper function to add elements to window groups
   auto addToWindowGroup = [&](const std::vector<std::shared_ptr<UIElement>>& elements, int groupIndex)
   {
      for (auto& el : elements)
      {
         WINDOW* window = el->getNcurseWindow();
         if (!window)
            window = stdscr;
         windowGroups[groupIndex][window].push_back(el);
      }
   };

   // Group all filtered elements by window and position
   addToWindowGroup(filteredTopMiddle, 0);
   addToWindowGroup(filteredRightMiddle, 1);
   addToWindowGroup(filteredBottomMiddle, 2);
   addToWindowGroup(filteredLeftMiddle, 3);
   addToWindowGroup(filteredMiddle, 4);
   addToWindowGroup(filteredTopLeft, 5);
   addToWindowGroup(filteredTopRight, 6);
   addToWindowGroup(filteredBottomLeft, 7);
   addToWindowGroup(filteredBottomRight, 8);

   // Process each window (should only be stdscr)
   for (int groupIndex = 0; groupIndex < 9; ++groupIndex)
   {
      for (auto& [window, elements] : windowGroups[groupIndex])
      {
         if (elements.empty())
            continue;

         // Get window dimensions and adjust for borders if enabled
         int windowHeight, windowLength;
         getmaxyx(window, windowHeight, windowLength);

         // Check if this window has borders and adjust usable area
         auto ncursesWindow = findNcursesWindow(window);
         int  borderPadding = 0;
         if (ncursesWindow && ncursesWindow->isBorderEnabled())
         {
            borderPadding = 1;
            windowHeight -= 2;
            windowLength -= 2;
         }

         // Process elements based on their position type (same logic as updateAllLockedPositions)
         switch (groupIndex)
         {
            case 0: // TOP MIDDLE
            {
               int totalWidth = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalWidth += elementWidth(el);
               }
               int dx = (windowLength - totalWidth) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(dx, borderPadding));
                  el->setPositions();
                  dx += elementWidth(el);
               }
               break;
            }
            case 1: // RIGHT MIDDLE
            {
               int totalHeight = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalHeight += elementHeight(el);
               }
               int dy = (windowHeight - totalHeight) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  int x = windowLength - elementWidth(el) + borderPadding;
                  el->moveToPosition(Position(x, dy));
                  el->setPositions();
                  dy += elementHeight(el);
               }
               break;
            }
            case 2: // BOTTOM MIDDLE
            {
               int totalWidth = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalWidth += elementWidth(el);
               }
               int dx = (windowLength - totalWidth) / 2 + borderPadding;
               int y  = windowHeight - 1 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(dx, y - elementHeight(el) + 1));
                  el->setPositions();
                  dx += elementWidth(el);
               }
               break;
            }
            case 3: // LEFT MIDDLE
            {
               int totalHeight = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalHeight += elementHeight(el);
               }
               int dy = (windowHeight - totalHeight) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(borderPadding, dy));
                  el->setPositions();
                  dy += elementHeight(el);
               }
               break;
            }
            case 4: // CENTER
            {
               int totalWidth = 0;
               int maxHeight  = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalWidth += elementWidth(el);
                  if (elementHeight(el) > maxHeight)
                     maxHeight = elementHeight(el);
               }
               int dx = (windowLength - totalWidth) / 2 + borderPadding;
               int y  = (windowHeight - maxHeight) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(dx, y));
                  el->setPositions();
                  dx += elementWidth(el);
               }
               break;
            }
            case 5: // TOP LEFT CORNER
            {
               int dx = borderPadding;
               int dy = borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(dx, dy));
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     dx += elementWidth(el);
                  }
                  else
                  {
                     dy += elementHeight(el);
                  }
               }
               break;
            }
            case 6: // TOP RIGHT CORNER
            {
               int dx = windowLength + borderPadding;
               int dy = borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     dx -= elementWidth(el);
                     el->moveToPosition(Position(dx, dy));
                  }
                  else
                  {
                     el->moveToPosition(Position(dx - elementWidth(el), dy));
                     dy += elementHeight(el);
                  }

                  el->setPositions();
               }
               break;
            }
            case 7: // BOTTOM LEFT CORNER
            {
               int dx = borderPadding;
               int dy = windowHeight + borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     el->moveToPosition(Position(dx, dy - elementHeight(el)));
                     dx += elementWidth(el);
                  }
                  else
                  {
                     dy -= elementHeight(el);
                     el->moveToPosition(Position(dx, dy));
                  }

                  el->setPositions();
               }
               break;
            }
            case 8: // BOTTOM RIGHT CORNER
            {
               int dx = windowLength + borderPadding;
               int dy = windowHeight + borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     dx -= elementWidth(el);
                     el->moveToPosition(Position(dx, dy - elementHeight(el)));
                  }
                  else
                  {
                     dy -= elementHeight(el);
                     el->moveToPosition(Position(dx - elementWidth(el), dy));
                  }

                  el->setPositions();
               }
               break;
            }
         }
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void UIElement::displace(int dx, int dy)
{
   m_minPosition = Position(m_minPosition.getX() + dx, m_minPosition.getY() + dy);
   m_maxPosition = Position(m_maxPosition.getX() + dx, m_maxPosition.getY() + dy);
   for (Animation& animation : m_animations)
   {
      if (m_currentAnimationName == animation.getAnimationName())
      {
         m_dirtySprites.push_back(animation.getCurrentFrameSprite());
         animation.displace(dx, dy);
         break;
      }
   }
};